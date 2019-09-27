#pragma once

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <algorithm>
#include <cmath>

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

    void draw_text(const char *text, int x, int y, SDL_Color col);
    void draw_text2(const char *text, int x, int y);
    void draw_overlay(const Model &model);
    void draw_sprites(const Model &m, u_int32_t *pixels);

    static const uint32_t width = 960;
    static const uint32_t height = 630;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *buffer = nullptr;
    SDL_PixelFormat *buffer_format = nullptr;
    float z_buf[width][height];

    TTF_Font *font = nullptr;

  public:
    View();
    ~View();

    void draw(const Model &model);
};
