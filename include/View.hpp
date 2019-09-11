#pragma once

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <SDL2/SDL.h>

#include "Model.hpp"
#include "SDLExcept.hpp"

class View {
  private:
    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

    uint32_t width = 1366;
    uint32_t height = 768;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *buffer = nullptr;

    std::unique_ptr<uint32_t[]> background_pixels;

  public:
    View();
    ~View();

    void draw(const Model &model);
};