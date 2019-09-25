#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <iostream>

#include <SDL2/SDL_image.h>

#include "exception.hpp"

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Pixel() {
        r = 0;
        g = 0;
        b = 0;
        a = 0;
    }

    Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    Pixel operator*(float val) const {
        return Pixel(
            (uint8_t)((float)r * val),
            (uint8_t)((float)g * val),
            (uint8_t)((float)b * val),
            (uint8_t)((float)a * val)
        );
    }

    Pixel operator+(const Pixel &val) const {
        return Pixel(
            (uint8_t)std::clamp(((uint16_t)r + val.r), 0, 255),
            (uint8_t)std::clamp(((uint16_t)g + val.g), 0, 255),
            (uint8_t)std::clamp(((uint16_t)b + val.b), 0, 255),
            (uint8_t)std::clamp(((uint16_t)a + val.a), 0, 255)
        );
    }

    Pixel &operator+=(const Pixel &val) {
        r = (uint8_t)std::clamp(((uint16_t)r + val.r), 0, 255);
        g = (uint8_t)std::clamp(((uint16_t)g + val.g), 0, 255);
        b = (uint8_t)std::clamp(((uint16_t)b + val.b), 0, 255);
        a = (uint8_t)std::clamp(((uint16_t)a + val.a), 0, 255);
        return *this;
    }

    uint32_t get_int() const {
        return reinterpret_cast<const uint32_t &>(*this);
    }
};

static_assert(sizeof(Pixel) == sizeof(uint32_t));

struct Texture {
    std::unique_ptr<Pixel[]> pixels;
    size_t w;
    size_t h;
    std::string short_name;
    std::unique_ptr<bool[]> col_has_alpha;

    Texture(const std::string &short_name, SDL_Surface *surface);

    const Pixel &get(size_t x, size_t y) const {
        return pixels[y * w + x];
    }

    uint32_t get_uint(size_t x, size_t y) const {
        return get(x, y).get_int();
    }
};

class TextureStore {
  private:
    TextureStore(const TextureStore &other) = delete;
    TextureStore &operator=(const TextureStore &other) = delete;

    Texture load_texture(const std::string &filename);

    std::unordered_map<std::string, Texture> textures;

    static const std::unordered_map<std::string, std::string> filename_mapping;
    const std::string texture_path = "assets/textures/";

  public:
    TextureStore();
    ~TextureStore();

    const Texture *get(std::string short_name) const;
};
