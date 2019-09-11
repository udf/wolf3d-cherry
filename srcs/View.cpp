#include "View.hpp"

View::View() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("Failed to initialize SDL video subsystem");
    }

    window = SDL_CreateWindow(
        "wolf3d",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        throw std::runtime_error("Failed to create SDL window");
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        throw std::runtime_error("Failed to create renderer");
    }

    buffer = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB32,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
}

View::~View() {
    SDL_DestroyTexture(buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void View::draw(const Model &model) {
    (void)model;
}