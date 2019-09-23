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
        SDL_PIXELFORMAT_RGB888,
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
            *p = SDL_MapRGB(buffer_format, 0, 0, 0);
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

void View::cast_ray(
    const Model &m,
    float camX,
    float scale,
    Model::Coord transform
) {
    // Calculate ray direction
    const float rayDirX = m.player.rot_vec.x + m.cam_rot_vec.x * camX;
    const float rayDirY = m.player.rot_vec.y + m.cam_rot_vec.y * camX;

    // Which cell of the map we're in
    ssize_t mapX = (ssize_t)m.player.pos.x;
    ssize_t mapY = (ssize_t)m.player.pos.y;

    // Length of ray from current position to next side
    float sideDistX, sideDistY;

    // Length of ray from one side to next
    const float deltaDistX = std::abs(1.0f / rayDirX);
    const float deltaDistY = std::abs(1.0f / rayDirY);

    // What direction to step in
    ssize_t stepX, stepY;

    // bool hit = false;
    bool is_ns;

    // Calculate step and initial sideDist
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (m.player.pos.x - (float)mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = ((float)mapX + 1.0f - m.player.pos.x) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (m.player.pos.y - (float)mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = ((float)mapY + 1.0f - m.player.pos.y) * deltaDistY;
    }

    SDL_FRect rect = {0, 0, 2.f, 2.f};

    if (m.debug) {
        std::cout << "player data" << std::endl;
        std::cout << "pos(" << m.player.pos.x << ", " << m.player.pos.y << ")" << std::endl;
        std::cout << "ra " << m.player.rot << std::endl;
        std::cout << "dir(" << m.player.rot_vec.x << ", " << m.player.rot_vec.y << ")" << std::endl;
        std::cout << "cast data" << std::endl;
        std::cout << "ray(" << rayDirX << ", " << rayDirY << ")" << std::endl;
        std::cout << "delta(" << deltaDistX << ", " << deltaDistY << ")" << std::endl;
        std::cout << "step(" << stepX << ", " << stepY << ")" << std::endl;
        std::cout << "map(" << mapX << ", " << mapY << ")" << std::endl;
        std::cout << "side(" << sideDistX << ", " << sideDistY << ")" << std::endl;
        std::cout << "loop start" << std::endl;
    }

    // Do the thing
    // while(!hit) {
    for (size_t i = 0; i < 2; i++) {
        if (sideDistX < sideDistY) {
            mapX += stepX;
            is_ns = false;
        } else {
            mapY += stepY;
            is_ns = true;
        }

        if (m.debug) {
            std::cout << "is_ns " << is_ns << std::endl;
            std::cout << "map(" << mapX << ", " << mapY << ")" << std::endl;
            std::cout << "side(" << sideDistX << ", " << sideDistY << ")" << std::endl;
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 0);
        rect.x = (float)mapX * scale + transform.x;
        rect.y = (float)mapY * scale + transform.y;
        SDL_RenderFillRectF(renderer, &rect);

        if (is_ns) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
            rect.x = (m.player.pos.x + sideDistY * rayDirX) * scale + transform.x;
            rect.y = (m.player.pos.y + sideDistY * rayDirY) * scale + transform.y;
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
            rect.x = (m.player.pos.x + sideDistX * rayDirX) * scale + transform.x;
            rect.y = (m.player.pos.y + sideDistX * rayDirY) * scale + transform.y;
        }
        SDL_RenderFillRectF(renderer, &rect);

        if (is_ns) {
            sideDistY += deltaDistY;
        } else {
            sideDistX += deltaDistX;
        }
    }
    if (m.debug)
        std::cout << std::endl;
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

void View::draw(const Model &model) {
    (void)model;
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

    // Center the player's position on screen and draw a texture centered on it
    // Model::Coord pos = model.player_pos;
    // pos += Model::Coord(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);
    // pos -= static_cast<float>(model.player_tex->w) / 2.f;
    // Point<size_t> offset(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));

    // for (size_t x = 0; x < model.player_tex->w; x++) {
    //     for (size_t y = 0; y < model.player_tex->h; y++) {
    //         auto p = model.player_tex->get(x, y);
    //         *texel(pixels, width, x + offset.x, y + offset.y) = SDL_MapRGB(buffer_format, p->r, p->g, p->b);
    //     }
    // }

    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);

    // Compute a vector to center everything around the player
    Model::Coord center = Model::Coord(
        static_cast<float>(width) / 2.f,
        static_cast<float>(height) / 2.f
    );

    const Model::Coord::type scale = 50.f;
    Model::Coord transform = center - model.player.pos * scale;

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
    for (size_t x = 0; x <= model.map_w; x++) {
        draw_scaled_line(x, 0, x, model.map_h);
    }
    for (size_t y = 0; y <= model.map_h; y++) {
        draw_scaled_line(0, y, model.map_w, y);
    }

    // draw walls
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    for (size_t x = 0; x < model.map_w; x++) {
        for (size_t y = 0; y < model.map_h; y++) {
            auto cell = model.get_cell((ssize_t)x, (ssize_t)y);
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
        center.x + model.player.rot_vec.x * 100,
        center.y + model.player.rot_vec.y * 100
    );

    cast_ray(model, 0, scale, transform);

    uint32_t frame_time = SDL_GetTicks() - model.frame_start_ms;

    std::stringstream ss;
    ss << "fps: " << model.fps;
    draw_text(ss.str().c_str(), 5, 5);
    ss.str("");
    ss << "frame time: " << frame_time << " ms";
    draw_text(ss.str().c_str(), 5, 25);

    // TODO: move this into a function
    SDL_RenderPresent(renderer);
}