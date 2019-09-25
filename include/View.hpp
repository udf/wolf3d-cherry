#pragma once

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Model.hpp"
#include "texture_store.hpp"
#include "exception.hpp"
#include "util.hpp"

class View {
  private:

    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

    void draw_text(const char *text, int x, int y);
    void draw_overlay(const Model &model);

    uint32_t width = 640;
    uint32_t height = 480;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *buffer = nullptr;
    SDL_PixelFormat *buffer_format = nullptr;

    TTF_Font *font = nullptr;

  public:
    View();
    ~View();

    void draw(const Model &model);
};
