#include "View.hpp"

uint32_t *texel(uint32_t *pixels, size_t width, size_t x, size_t y) {
    return &pixels[y * width + x];
}

View::View() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        throw SDLExcept("Failed to initialize SDL video subsystem");
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
        throw SDLExcept("Failed to create SDL window");
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        throw SDLExcept("Failed to create renderer");
    }

    buffer = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB32,
        SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(width),
        static_cast<int>(height)
    );

    background_pixels = std::make_unique<uint32_t[]>(width * height);
    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            auto p = texel(background_pixels.get(), width, x, y);
            if ((x % 2 == 0) ^ (y % 2 == 0)) {
                *p = 0;
            } else {
                *p = 0xFFFFFF00;
            }
        }
    }
}

View::~View() {
    SDL_DestroyTexture(buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void View::draw(const Model &model) {
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
    Model::Coord pos = model.player_pos;
    pos += Model::Coord(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f);
    pos -= static_cast<float>(model.player_tex->w) / 2.f;
    Point<size_t> offset(static_cast<size_t>(pos.x), static_cast<size_t>(pos.y));
    for (size_t x = 0; x < model.player_tex->w; x++) {
        for (size_t y = 0; y < model.player_tex->h; y++) {
            *texel(pixels, width, x + offset.x, y + offset.y) = model.player_tex->get_uint(x, y);
        }
    }

    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
}