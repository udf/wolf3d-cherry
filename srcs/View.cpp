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

    background_pixels = std::make_unique<uint32_t[]>(width * height);
    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            auto p = texel(background_pixels.get(), width, x, y);
            *p = SDL_MapRGB(buffer_format, 150, 150, 150);
        }
    }
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

auto View::cast_ray(const Model &m, const Model::Coord ray_dir) -> RayHit {
    // Which cell of the map we're in
    Point<ssize_t> map = m.player.pos.cast_to<ssize_t>();

    // Length of ray from current position to next side
    Model::Coord side_dist;

    // Length of ray from one side to next
    const Model::Coord delta_dist = {
        std::abs(1.f / ray_dir.x), std::abs(1.f / ray_dir.y)
    };

    // What direction to step in
    const Point<ssize_t> step = {
        ray_dir.x < 0 ? -1 : 1,
        ray_dir.y < 0 ? -1 : 1
    };

    RayHit hit;
    bool is_ns = false;

    // Which wall to check for the near and far cell
    const Texture *Cell::* ew_near_texture;
    const Texture *Cell::* ew_far_texture;
    const Texture *Cell::* ns_near_texture;
    const Texture *Cell::* ns_far_texture;

    // Calculate initial sideDist
    if (ray_dir.x < 0) {
        side_dist.x = (m.player.pos.x - (float)map.x) * delta_dist.x;
        ew_near_texture = &Cell::wall_left;
        ew_far_texture = &Cell::wall_right;
    } else {
        side_dist.x = ((float)map.x + 1.0f - m.player.pos.x) * delta_dist.x;
        ew_near_texture = &Cell::wall_right;
        ew_far_texture = &Cell::wall_left;
    }
    if (ray_dir.y < 0) {
        side_dist.y = (m.player.pos.y - (float)map.y) * delta_dist.y;
        ns_near_texture = &Cell::wall_top;
        ns_far_texture = &Cell::wall_bottom;
    } else {
        side_dist.y = ((float)map.y + 1.0f - m.player.pos.y) * delta_dist.y;
        ns_near_texture = &Cell::wall_bottom;
        ns_far_texture = &Cell::wall_top;
    }

    // Do the thing
    size_t iters = 0;
    while(true) {
        if (map.x < 0 || map.y < 0 || (size_t)map.x >= m.map_w || (size_t)map.y >= m.map_h)
            break; // down and cry
        if (side_dist.x < side_dist.y) {
            map.x += step.x;
            is_ns = false;
        } else {
            map.y += step.y;
            is_ns = true;
        }

        auto check_cell = [&m, &hit](
            ssize_t x, ssize_t y, const Texture *Cell::* tex_member
        ) {
            if (hit.tex)
                return;
            auto cell = m.get_cell(x, y);
            if (cell)
                hit.tex = cell->*tex_member;
        };

        ssize_t mapNearX = map.x;
        ssize_t mapNearY = map.y;
        if (is_ns) {
            mapNearY -= step.y;
            check_cell(mapNearX, mapNearY, ns_near_texture);
            check_cell(map.x, map.y, ns_far_texture);
            hit.pos = m.player.pos + ray_dir * side_dist.y;
        } else {
            mapNearX -= step.x;
            check_cell(mapNearX, mapNearY, ew_near_texture);
            check_cell(map.x, map.y, ew_far_texture);
            hit.pos = m.player.pos + ray_dir * side_dist.x;
        }

        if (hit.tex)
            break;

        if (is_ns) {
            side_dist.y += delta_dist.y;
        } else {
            side_dist.x += delta_dist.x;
        }
        iters++;
    }
    if (hit.tex) {
        hit.dist = (
            is_ns
            ? ((float)map.y - m.player.pos.y + (1.f - (float)step.y) / 2.f) / ray_dir.y
            : ((float)map.x - m.player.pos.x + (1.f - (float)step.x) / 2.f) / ray_dir.x
        );
    }
    return hit;
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
    auto hit = cast_ray(m, m.player.rot_vec);
    if (hit.tex) {
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

    // Copy background pixels to texture
    std::memcpy(
        static_cast<void *>(pixels),
        static_cast<void *>(background_pixels.get()),
        sizeof(uint32_t) * width * height
    );

    if (m.debug) {
        std::cout << "player data" << std::endl;
        std::cout << "pos(" << m.player.pos.x << ", " << m.player.pos.y << ")" << std::endl;
        std::cout << "ra " << m.player.rot << std::endl;
        std::cout << "dir(" << m.player.rot_vec.x << ", " << m.player.rot_vec.y << ")" << std::endl;
    }
    for (uint32_t x = 0; x < width; x++) {
        float camX = fmapf((float)(x + 1), 1, (float)width, 1.f, -1.f);
        const Model::Coord ray_dir = m.player.rot_vec + m.cam_rot_vec * camX;
        auto hit = cast_ray(m, ray_dir);
        if (!hit.tex)
            continue;
        float line_height = (float)height / hit.dist;
        size_t y_start = (size_t)std::clamp(
            (float)height / 2.f - line_height / 2.f,
            0.f,
            (float)(height - 1)
        );
        size_t y_end = (size_t)std::clamp(
            (float)height / 2.f + line_height / 2.f,
            0.f,
            (float)(height - 1)
        );

        for (size_t y = y_start; y < y_end; y++) {
            *texel(pixels, width, x, y) = hit.tex->get_uint(0, 0);
        }
    }

    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);

    draw_overlay(m);

    uint32_t frame_time = SDL_GetTicks() - m.frame_start_ms;

    std::stringstream ss;
    ss << "fps: " << m.fps;
    draw_text(ss.str().c_str(), 5, 5);
    ss.str("");
    ss << "frame time: " << frame_time << " ms";
    draw_text(ss.str().c_str(), 5, 25);

    SDL_RenderPresent(renderer);
}
