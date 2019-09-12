#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>

#include <SDL2/SDL_image.h>

#include "SDLExcept.hpp"

struct Pixel {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static_assert(sizeof(Pixel) == sizeof(uint32_t));

struct Texture {
    std::unique_ptr<Pixel[]> pixels;
    size_t w;
    size_t h;

    Texture(SDL_Surface *surface);

    const Pixel *get(size_t x, size_t y) const {
        return &pixels[y * w + x];
    }

    uint32_t get_uint(size_t x, size_t y) const {
        return reinterpret_cast<const uint32_t &>(*get(x, y));
    }
};

class TextureStore {
  private:
    TextureStore(const TextureStore &other) = delete;
    TextureStore &operator=(const TextureStore &other) = delete;

    Texture load_texture(const std::string &file_name);

    std::unordered_map<std::string, Texture> textures;

    static const std::unordered_map<std::string, std::string> filename_mapping;
    const std::string texture_path = "textures/";

  public:
    TextureStore();
    ~TextureStore();

    const Texture *get(std::string short_name) const;
};