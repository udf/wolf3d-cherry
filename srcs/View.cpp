#include "View.hpp"

// TODO: maybe use reference
uint32_t *texel(uint32_t *pixels, size_t width, size_t x, size_t y) {
    return &pixels[y * width + x];
}

View::View() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        throw Exception("Failed to initialize SDL video subsystem")
            .set_hint(SDL_GetError());
    }

    window = SDL_CreateWindow(
        "wolf3d",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(width),
        static_cast<int>(height),
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        throw Exception("Failed to create SDL window")
            .set_hint(SDL_GetError());
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        throw Exception("Failed to create renderer")
            .set_hint(SDL_GetError());
    }

    if (TTF_Init() != 0) {
        throw Exception("Failed to initialise SDL TTF")
            .set_hint(TTF_GetError());
    }
    font = TTF_OpenFont("assets/fonts/CyberpunkWaifus.ttf", 16);
    if (!font) {
        throw Exception("Failed to load font")
            .set_hint(TTF_GetError());
    }
    TTF_SetFontHinting(font, TTF_HINTING_NONE);

    buffer = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(width),
        static_cast<int>(height)
    );
    uint32_t buffer_format_enum;
    SDL_QueryTexture(buffer, &buffer_format_enum, NULL, NULL, NULL);
    buffer_format = SDL_AllocFormat(buffer_format_enum);
}

View::~View() {
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_FreeFormat(buffer_format);
    SDL_DestroyTexture(buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void View::draw_text(const char *text, int x, int y) {
    auto font_surface = TTF_RenderText_Solid(
        font,
        text,
        {255, 255, 255, 0}
    );
    SDL_Rect src_rect = {x, y, font_surface->w, font_surface->h};
    auto font_tex = SDL_CreateTextureFromSurface(renderer, font_surface);
    SDL_FreeSurface(font_surface);
    SDL_RenderCopy(renderer, font_tex, NULL, &src_rect);
    SDL_DestroyTexture(font_tex);
}

void View::draw_overlay(const Model &m) {
    // Compute a vector to center everything around the player
    Model::Coord center = Model::Coord(
        static_cast<float>(width) / 2.f,
        static_cast<float>(height) / 2.f
    );

    const Model::Coord::type scale = 50.f;
    Model::Coord transform = center - m.player.pos * scale;

    const auto draw_scaled_line = [&](auto x1, auto y1, auto x2, auto y2) {
        Model::Coord p1 = {(float)x1, (float)y1};
        Model::Coord p2 = {(float)x2, (float)y2};
        p1 *= scale;
        p1 += transform;
        p2 *= scale;
        p2 += transform;
        SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
    };

    // draw map grid
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 0);
    for (size_t x = 0; x <= m.map_w; x++) {
        draw_scaled_line(x, 0, x, m.map_h);
    }
    for (size_t y = 0; y <= m.map_h; y++) {
        draw_scaled_line(0, y, m.map_w, y);
    }

    // draw walls
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    for (size_t x = 0; x < m.map_w; x++) {
        for (size_t y = 0; y < m.map_h; y++) {
            auto cell = m.get_cell((ssize_t)x, (ssize_t)y);
            if (cell->wall_top)
                draw_scaled_line(x, y, x + 1, y);
            if (cell->wall_bottom)
                draw_scaled_line(x, y + 1, x + 1, y + 1);
            if (cell->wall_left)
                draw_scaled_line(x, y, x, y + 1);
            if (cell->wall_right)
                draw_scaled_line(x + 1, y, x + 1, y + 1);
        }
    }

    // draw player
    SDL_FRect rect = {center.x - 1.f, center.y - 1.f, 2.f, 2.f};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_RenderFillRectF(renderer, &rect);
    SDL_RenderDrawLineF(
        renderer,
        center.x,
        center.y,
        center.x + m.player.rot_vec.x * 10,
        center.y + m.player.rot_vec.y * 10
    );

    // Draw first wall hit
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
    auto hit = m.cast_ray(m.player.rot_vec);
    rect.x = hit.pos.x * scale + transform.x;
    rect.y = hit.pos.y * scale + transform.y;
    SDL_RenderFillRectF(renderer, &rect);
    SDL_RenderDrawLineF(
        renderer,
        center.x,
        center.y,
        rect.x,
        rect.y
    );
}

void View::draw(const Model &m) {
    uint32_t *pixels;
    int pitch;

    SDL_LockTexture(
        buffer,
        NULL,
        reinterpret_cast<void **>(&pixels),
        &pitch
    );

    #pragma omp parallel for
    for (uint32_t x = 0; x < width; x++) {
        float camX = fmapf((float)(x + 1), 1, (float)width, 1.f, -1.f);
        const Model::Coord ray_dir = m.player.rot_vec + m.cam_rot_vec * camX;
        auto hit = m.cast_ray(ray_dir);
        float line_height = (float)height / hit.dist;
        ssize_t y_start = (ssize_t)((float)height / 2.f - line_height / 2.f);
        ssize_t y_end = (ssize_t)((float)height / 2.f + line_height / 2.f);
        ssize_t y;
        for (y = 0; y < y_start; y++) {
            float dist = ((float)height / 2.0f) / (((float)height / 2.0f) - (float)y);
            Model::Coord place = m.player.pos + (ray_dir * dist);
            float tx = (frac(place.x));
            float ty = (frac(place.y));
            auto cell = m.get_cell((ssize_t)place.x, (ssize_t)place.y);
            if (!cell)
                continue;
            if (cell->ceil)
                *texel(pixels, width, x, y) = cell->ceil->get_uint((int)(tx * (float)(cell->ceil->w)), (int)(ty * (float)(cell->ceil->h)));
            if (cell->floor)
                *texel(pixels, width, x, height - y - 2) = cell->floor->get_uint((int)(tx * (float)(cell->floor->w)), (int)(ty * (float)(cell->floor->h)));
        }
        if (hit.tex)
            for (; y < y_end && y < (ssize_t)height; y++) {
                float tx = hit.is_ns ? (ray_dir.y < 0 ? frac(hit.pos.x) : 1 - frac(hit.pos.x)) :  (ray_dir.x > 0 ? frac(hit.pos.y) : 1 - frac(hit.pos.y));
                float ty = (float)(y - y_start) / (float)(y_end - y_start);
		auto p= hit.tex->get((int)(tx * (float)hit.tex->w), (int)(ty * (float)hit.tex->h));

		if (ty > 0.3f)
                	*texel(pixels, width, x, y) = p.get_int();
		else
	                *texel(pixels, width, x, y) = (p * ((ty / 0.6f) + 0.5f)).get_int();
		if ((y_end + (y_end - y)) < (ssize_t)height)
		{
			Pixel res = *reinterpret_cast<Pixel*>(texel(pixels, width, x, (2 * y_end - y)));
			res += p * 0.03f;
			*texel(pixels, width, x, (2 * y_end - y)) = res.get_int();
		}

            }
    }

    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);

    // draw_overlay(m);

    uint32_t frame_time = SDL_GetTicks() - m.frame_start_ms;

    std::stringstream ss;
    ss << "fps: " << m.fps;
    draw_text(ss.str().c_str(), 5, 5);
    ss.str("");
    ss << "frame time: " << frame_time << " ms";
    draw_text(ss.str().c_str(), 5, 25);

    SDL_RenderPresent(renderer);
}
