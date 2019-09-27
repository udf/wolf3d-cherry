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
        static_cast<int>(1280),
        static_cast<int>(1024),
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
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
    font = TTF_OpenFont("assets/fonts/CyberpunkWaifus.ttf", 32);
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


void View::draw_text(const char *text, int x, int y, SDL_Color col) {
    auto font_surface = TTF_RenderText_Solid(
        font,
        text,
        col
    );
    SDL_Rect src_rect = {x, y, font_surface->w, font_surface->h};
    auto font_tex = SDL_CreateTextureFromSurface(renderer, font_surface);
    SDL_FreeSurface(font_surface);
    SDL_RenderCopy(renderer, font_tex, NULL, &src_rect);
    SDL_DestroyTexture(font_tex);
}

void View::draw_text2(const char *text, int x, int y, SDL_Color col) {
    draw_text(text, x + 1, y + 1, {0, 0, 0, 0});
    draw_text(text, x, y, col);
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

    const auto draw_wall_line = [&](
        const Texture *tex,
        auto x1,
        auto y1,
        auto x2,
        auto y2
    ) {
        if (!tex->is_solid) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 0);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        }
        draw_scaled_line(x1, y1, x2, y2);
    };

    // draw walls
    for (size_t x = 0; x < m.map_w; x++) {
        for (size_t y = 0; y < m.map_h; y++) {
            auto cell = m.get_cell((ssize_t)x, (ssize_t)y);
            if (cell->wall_top)
                draw_wall_line(cell->wall_top, x, y, x + 1, y);
            if (cell->wall_bottom)
                draw_wall_line(cell->wall_bottom, x, y + 1, x + 1, y + 1);
            if (cell->wall_left)
                draw_wall_line(cell->wall_left, x, y, x, y + 1);
            if (cell->wall_right)
                draw_wall_line(cell->wall_right, x + 1, y, x + 1, y + 1);
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
    auto hits = m.cast_ray(m.player.rot_vec);
    Model::Coord pos = center;
    for (auto &hit : hits) {
        if (!hit.tex)
            break;
        rect.x = hit.pos.x * scale + transform.x;
        rect.y = hit.pos.y * scale + transform.y;
        SDL_RenderFillRectF(renderer, &rect);
        SDL_RenderDrawLineF(
            renderer,
            pos.x,
            pos.y,
            rect.x,
            rect.y
        );
        pos.x = rect.x;
        pos.y = rect.y;
    }
}

void View::draw_sprites(const Model &m, uint32_t *pixels) {
    const float inv_det = 1.0f / (m.cam_rot_vec.x * m.player.rot_vec.y - m.player.rot_vec.x * m.cam_rot_vec.y);

    for (auto &sprite : m.sprites) {
        Model::Coord pos = sprite.pos - m.player.pos;
        Model::Coord transform;
        transform.x = -inv_det * (m.player.rot_vec.y * pos.x - m.player.rot_vec.x * pos.y);
        transform.y = inv_det * (-m.cam_rot_vec.y * pos.x + m.cam_rot_vec.x * pos.y);

        if (transform.y <= 0)
            continue;

        float sprite_h = std::abs(height / transform.y);
        ssize_t y_start = (ssize_t)std::max(0.f, -sprite_h / 2.f + height / 2.f);
        ssize_t y_end = (ssize_t)std::min((float)height, sprite_h / 2.f + height / 2.f);

        float sprite_w = std::abs(height / transform.y);
        float screen_x = (width / 2.f) * (1 + transform.x / transform.y);
        ssize_t x_start = (ssize_t)std::max(0.f, -sprite_w / 2.f + screen_x);
        ssize_t x_end = (ssize_t)std::min((float)width, sprite_w / 2.f + screen_x);

        for (ssize_t x = x_start; x < x_end; x++) {
            const ssize_t tx = (ssize_t)(((float)x - (-sprite_w / 2 + screen_x)) * (float)sprite.tex->w / sprite_w);
            for (ssize_t y = y_start; y < y_end; y++) {
                const float d = (float)y - height / 2.f + sprite_h / 2.f;
                const ssize_t ty = (ssize_t)((d * (float)sprite.tex->h) / sprite_h);
                auto p = sprite.tex->get(tx, ty);
                if (!p.a || transform.y >= z_buf[x][y])
                    continue;
                *texel(pixels, width, x, y) = p.get_int();
                z_buf[x][y] = transform.y;
            }
        }
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

    #pragma omp parallel for
    for (uint32_t x = 0; x < width; x++) {
        for (uint32_t y = 0; y < height; y++) {
            z_buf[x][y] = std::numeric_limits<float>::infinity();
        }
        float camX = fmapf((float)(x + 1), 1, (float)width, 1.f, -1.f);
        const Model::Coord ray_dir = m.player.rot_vec + m.cam_rot_vec * camX;
        auto hits = m.cast_ray(ray_dir);
        for (int i = (int)hits.size() - 1; i >= 0; i--) {
            auto &hit = hits[i];
            if (!hit.tex)
                continue;
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
                if (cell->floor) {
                    Pixel p = cell->floor->get((int)(tx * (float)(cell->floor->w)), (int)(ty * (float)(cell->floor->h)));
                    *texel(pixels, width, x, height - y - 2) = p.get_int();
                }
            }
            
            if (hit.tex) {
                for (; y < y_end && y < (ssize_t)height; y++) {
                    float tx = hit.is_ns ? (ray_dir.y < 0 ? frac(hit.pos.x) : 1 - frac(hit.pos.x)) :  (ray_dir.x > 0 ? frac(hit.pos.y) : 1 - frac(hit.pos.y));
                    float ty = (float)(y - y_start) / (float)(y_end - y_start);
                    auto p = hit.tex->get((int)(tx * (float)hit.tex->w), (int)(ty * (float)hit.tex->h));

                    if (p.a) {
                        if (ty <= 0.3f) {
                            p = (p * ((ty / 0.6f) + 0.5f));
                        }
                        *texel(pixels, width, x, y) = p.get_int();
                        z_buf[x][y] = hit.dist;
                    }
                    if ((y_end + (y_end - y)) < (ssize_t)height) {
                        Pixel res = *reinterpret_cast<Pixel*>(texel(pixels, width, x, (2 * y_end - y)));
                        res += p * 0.02f;
                        *texel(pixels, width, x, (2 * y_end - y)) = res.get_int();
                    }
                }
            }
        }
    }

    draw_sprites(m, pixels);

    SDL_UnlockTexture(buffer);
    const SDL_Rect dst = {0, 0, 1280, 840};
    SDL_RenderCopy(renderer, buffer, NULL, &dst);

    // draw_overlay(m);

    char buffer[6];
    uint32_t game_elapsed_s = (SDL_GetTicks() - m.game_start_ms) / 1000;
    int rem_s = std::max(0, 90 - (int)game_elapsed_s);
    snprintf(buffer, 6, "%02d:%02d", rem_s / 60, rem_s % 60);
    SDL_Color col = {255, 255, 255, 0};
    if (rem_s == 0)
        col = {255, 70, 70, 0};
    draw_text2(buffer, 1280 - 76 - 20, 15, col);

    // uint32_t frame_time = SDL_GetTicks() - m.frame_start_ms;
    // std::stringstream ss;
    // ss << "fps: " << m.fps;
    // draw_text(ss.str().c_str(), 5, 5);
    // ss.str("");
    // ss << "frame time: " << frame_time << " ms";
    // draw_text(ss.str().c_str(), 5, 25);
    if (m.hint)
        draw_text2(m.hint, 5, 5);

    SDL_RenderPresent(renderer);
}
