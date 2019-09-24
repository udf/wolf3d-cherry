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
    struct RayHit {
        const Texture *tex = nullptr;
        float dist;
    };

    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

    void draw_text(const char *text, int x, int y);
    RayHit cast_ray(const Model &m, float camX);
    void draw_overlay(const Model &model);

    uint32_t width = 1000;
    uint32_t height = 800;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *buffer = nullptr;
    SDL_PixelFormat *buffer_format = nullptr;

    TTF_Font *font = nullptr;

    std::unique_ptr<uint32_t[]> background_pixels;

  public:
    View();
    ~View();

    void draw(const Model &model);
};
