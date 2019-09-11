#pragma once

#include <cstdint>
#include <stdexcept>

#include <SDL2/SDL.h>

class View {
  private:
    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

    int32_t width = 640;
    int32_t height = 480;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

  public:
    View();
    ~View();
};