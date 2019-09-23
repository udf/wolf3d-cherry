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
            *p = SDL_MapRGB(buffer_format, 50, 50, 50);
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

auto View::cast_ray(const Model &m, float camX) -> RayHit {
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

    RayHit hit;
    bool is_ns = false;

    // Which wall to check for the near and far cell
    const Texture *Cell::* ew_near_texture;
    const Texture *Cell::* ew_far_texture;
    const Texture *Cell::* ns_near_texture;
    const Texture *Cell::* ns_far_texture;

    // Calculate step and initial sideDist
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (m.player.pos.x - (float)mapX) * deltaDistX;
        ew_near_texture = &Cell::wall_left;
        ew_far_texture = &Cell::wall_right;
        if (m.debug) {
            std::cout << "left right" << std::endl;
        }
    } else {
        stepX = 1;
        sideDistX = ((float)mapX + 1.0f - m.player.pos.x) * deltaDistX;
        ew_near_texture = &Cell::wall_right;
        ew_far_texture = &Cell::wall_left;
        if (m.debug) {
            std::cout << "right left" << std::endl;
        }
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (m.player.pos.y - (float)mapY) * deltaDistY;
        ns_near_texture = &Cell::wall_top;
        ns_far_texture = &Cell::wall_bottom;
        if (m.debug) {
            std::cout << "top bottom" << std::endl;
        }
    } else {
        stepY = 1;
        sideDistY = ((float)mapY + 1.0f - m.player.pos.y) * deltaDistY;
        ns_near_texture = &Cell::wall_bottom;
        ns_far_texture = &Cell::wall_top;
        if (m.debug) {
            std::cout << "bottom top" << std::endl;
        }
    }

    if (m.debug) {
        std::cout << "cast data" << std::endl;
        std::cout << "ray(" << rayDirX << ", " << rayDirY << ")" << std::endl;
        std::cout << "delta(" << deltaDistX << ", " << deltaDistY << ")" << std::endl;
        std::cout << "step(" << stepX << ", " << stepY << ")" << std::endl;
        std::cout << "map(" << mapX << ", " << mapY << ")" << std::endl;
        std::cout << "side(" << sideDistX << ", " << sideDistY << ")" << std::endl;
        std::cout << "loop start" << std::endl;
    }

    // Do the thing
    size_t iters = 0;
    while(true) {
        if (mapX < 0 || mapY < 0 || (size_t)mapX >= m.map_w || (size_t)mapY >= m.map_h)
            break; // down and cry
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

        ssize_t mapNearX = mapX;
        ssize_t mapNearY = mapY;
        if (is_ns) {
            mapNearY -= stepY;
            auto cell = m.get_cell(mapNearX, mapNearY);
            if (cell)
                hit.tex = cell->*ns_near_texture;
            cell = m.get_cell(mapX, mapY);
            if (cell)
                hit.tex = cell->*ns_far_texture;
        } else {
            mapNearX -= stepX;
            auto cell = m.get_cell(mapNearX, mapNearY);
            if (cell)
                hit.tex = cell->*ew_near_texture;
            cell = m.get_cell(mapX, mapY);
            if (cell)
                hit.tex = cell->*ew_far_texture;
        }

        if (hit.tex)
            break;

        if (is_ns) {
            sideDistY += deltaDistY;
        } else {
            sideDistX += deltaDistX;
        }
        iters++;
    }
    if (m.debug) {
        std::cout << "iters: " << iters << std::endl;
        std::cout << std::endl;
    }
    if (hit.tex) {
        hit.dist = (
            is_ns
            ? ((float)mapY - m.player.pos.y + (1.f - (float)stepY) / 2.f) / rayDirY
            : ((float)mapX - m.player.pos.x + (1.f - (float)stepX) / 2.f) / rayDirX
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

    if (model.debug) {
        std::cout << "player data" << std::endl;
        std::cout << "pos(" << model.player.pos.x << ", " << model.player.pos.y << ")" << std::endl;
        std::cout << "ra " << model.player.rot << std::endl;
        std::cout << "dir(" << model.player.rot_vec.x << ", " << model.player.rot_vec.y << ")" << std::endl;
    }
    for (uint32_t x = 0; x < width; x++) {
        float camX = fmapf((float)(x + 1), 1, (float)width, 1.f, -1.f);
        if (model.debug) {
            std::cout << camX << std::endl;
        }
        auto hit = cast_ray(model, camX);
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
        center.x + model.player.rot_vec.x * 10,
        center.y + model.player.rot_vec.y * 10
    );

    uint32_t frame_time = SDL_GetTicks() - model.frame_start_ms;

    std::stringstream ss;
    ss << "fps: " << model.fps;
    draw_text(ss.str().c_str(), 5, 5);
    ss.str("");
    ss << "frame time: " << frame_time << " ms";
    draw_text(ss.str().c_str(), 5, 25);

    SDL_RenderPresent(renderer);
}
