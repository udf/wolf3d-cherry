#include "texture_store.hpp"

const decltype(TextureStore::filename_mapping) TextureStore::filename_mapping{
    {"W1", "walls/Bricks.png"},
    {"V1", "walls/Window1.png"},
    {"V2", "walls/CoveredWindow.png"},
    {"W2", "walls/BlueWall.png"},
    {"W3", "walls/WTC.pngh"},
};

static uint32_t surface_get_pixel(SDL_Surface *surface, size_t i) {
    auto pixels = static_cast<uint8_t *>(surface->pixels);
    void *pixel = &pixels[i * surface->format->BytesPerPixel];

    switch (surface->format->BytesPerPixel) {
    case 1:
        return static_cast<uint32_t>(*static_cast<uint8_t *>(pixel));
    case 2:
        return static_cast<uint32_t>(*static_cast<uint16_t *>(pixel));
    case 3:
    {
        auto p = static_cast<uint8_t *>(pixel);
        if constexpr (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            return static_cast<uint32_t>(p[0] << 16U | p[1] << 8U | p[2]);
        } else {
            return static_cast<uint32_t>(p[2] << 16U | p[1] << 8U | p[0]);
        }
    }
    case 4:
        return *static_cast<uint32_t *>(pixel);

    default:
        return 0XFF00FF00;
    }
}

Texture::Texture(SDL_Surface *surface) {
    SDL_LockSurface(surface);
    this->w = static_cast<size_t>(surface->w);
    this->h = static_cast<size_t>(surface->h);
    const size_t total = w * h;
    this->pixels = std::make_unique<Pixel[]>(total);

    for (size_t i = 0; i < total; i++) {
        auto pixel = surface_get_pixel(surface, i);
        pixels[i] = reinterpret_cast<Pixel &>(pixel);
    }

    SDL_UnlockSurface(surface);
}

Texture TextureStore::load_texture(const std::string &filename) {
    SDL_Surface *surface = IMG_Load((texture_path + filename).c_str());
    if (!surface)
        throw Exception("texture store: failed to load " + filename)
            .set_hint(SDL_GetError());

    auto texture = Texture(surface);
    SDL_FreeSurface(surface);
    return texture;
}

TextureStore::TextureStore() {
    if (!IMG_Init(IMG_INIT_PNG))
        throw Exception("Failed to initialise SDL_image")
            .set_hint(SDL_GetError());

    for (auto& [short_name, filename] : filename_mapping) {
        std::cout << "Texture store: loading " << filename << std::endl;
        textures.emplace(short_name, load_texture(filename));
    }
}

TextureStore::~TextureStore() {
    IMG_Quit();
}

const Texture *TextureStore::get(std::string short_name) const {
    return &textures.at(short_name);
}