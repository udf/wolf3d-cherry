#include "View.hpp"

View::View() {
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
}

View::~View() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}