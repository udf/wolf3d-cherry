#pragma once

#include <cstdint>
#include <stdexcept>

#include <SDL2/SDL.h>

#include "Model.hpp"

class View {
  private:
    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

    int32_t width = 1366;
    int32_t height = 768;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

  public:
    View();
    ~View();

    void draw(const Model &model);
};