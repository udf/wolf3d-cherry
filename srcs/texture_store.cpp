#include "texture_store.hpp"
#include <iostream>
#include <bitset>

const decltype(TextureStore::filename_mapping) TextureStore::filename_mapping{
    {"W1", "walls/Bricks.png"},
    {"V1", "walls/Window1.png"},
    {"V2", "walls/CoveredWindow.png"},
    {"W2", "walls/BlueWall.png"},
    {"W3", "walls/WTC.png"},
};

static Pixel surface_get_pixel(SDL_Surface *surface, size_t i) {
    auto pixels = static_cast<uint8_t *>(surface->pixels);
    auto pixel = &pixels[i * surface->format->BytesPerPixel];
    Pixel ret;

    SDL_GetRGBA(
        *reinterpret_cast<uint32_t *>(pixel),
        surface->format,
        &ret.r,
        &ret.g,
        &ret.b,
        &ret.a
    );
    return ret;
}

Texture::Texture(const std::string &short_name, SDL_Surface *surface) {
    SDL_LockSurface(surface);
    this->w = static_cast<size_t>(surface->w);
    this->h = static_cast<size_t>(surface->h);
    const size_t total = w * h;
    this->pixels = std::make_unique<Pixel[]>(total);
    this->short_name = short_name;

    for (size_t i = 0; i < total; i++) {
        pixels[i] = surface_get_pixel(surface, i);
    }

    SDL_UnlockSurface(surface);
}

Texture TextureStore::load_texture(const std::string &short_name) {
    const std::string &filename = filename_mapping.at(short_name);
    SDL_Surface *surface = IMG_Load((texture_path + filename).c_str());
    if (!surface)
        throw Exception("texture store: failed to load " + filename)
            .set_hint(IMG_GetError());

    auto texture = Texture(short_name, surface);
    SDL_FreeSurface(surface);
    return texture;
}

TextureStore::TextureStore() {
    if (!IMG_Init(IMG_INIT_PNG))
        throw Exception("Failed to initialise SDL_image")
            .set_hint(IMG_GetError());

    for (auto& [short_name, filename] : filename_mapping) {
        std::cout << "Texture store: loading " << filename << std::endl;
        textures.emplace(short_name, load_texture(short_name));
    }
}

TextureStore::~TextureStore() {
    IMG_Quit();
}

const Texture *TextureStore::get(std::string short_name) const {
    if (short_name == "__")
        return nullptr;
    try {
        return &textures.at(short_name);
    } catch (std::out_of_range &e) {
        throw Exception("Unknown texture short name:")
            .set_hint(short_name);
    }
}