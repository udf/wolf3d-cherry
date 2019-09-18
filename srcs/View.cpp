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
    font = TTF_OpenFont("assets/fonts/CyberpunkWaifus.ttf", 32);
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
        center.x + model.player.rot_vec.x * 10,
        center.y + model.player.rot_vec.y * 10
    );


    // TODO: move this into a function
    // auto font_surface = TTF_RenderText_Solid(
    //     font,
    //     "Hack me like you've never hacked anything before!!",
    //     {255, 255, 255, 0}
    // );
    // auto font_tex = SDL_CreateTextureFromSurface(renderer, font_surface);
    // SDL_FreeSurface(font_surface);
    // SDL_Rect src_rect = {5, 5, 0, 0};
    // SDL_QueryTexture(font_tex, NULL, NULL, &src_rect.w, &src_rect.h);
    // SDL_RenderCopy(renderer, font_tex, NULL, &src_rect);
    // SDL_DestroyTexture(font_tex);

    SDL_RenderPresent(renderer);
}