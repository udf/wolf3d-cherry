#include "texture_store.hpp"
#include <iostream>
#include <bitset>

const decltype(TextureStore::filename_mapping) TextureStore::filename_mapping{
    {"W1", "walls/Bricks.png"},
    {"W2", "walls/BlueWall.png"},
    {"W3", "walls/WhiteWall.png"},
    {"W4", "walls/DarkBricks.png"},
    {"W5", "tiles/ReceptionDesk.png"},
    {"W6", "walls/Wood.png"},
    {"V1", "windows/Window1.png"},
    {"V2", "windows/CoveredWindow1.png"},
    {"V3", "windows/Window2.png"},
    {"V4", "windows/CoveredWindow2.png"},
    {"V5", "windows/Window3.png"},
    {"V6", "windows/CoveredWindow3.png"},
    {"C1", "ceilings/Grey_ceiling.png"},
    {"C2", "ceilings/concrete_ceiling1.png"},
    {"C3", "ceilings/White_ceiling.png"},
    {"C4", "ceilings/Blue_ceiling.png"},
    {"C5", "ceilings/concrete_ceiling1_pill.png"},
    {"F1", "floors/concrete_floor1.png"},
    {"F2", "floors/concrete_floor2.png"},
    {"F3", "floors/DarkTiles.png"},
    {"F4", "floors/Tiles.png"},
    {"F5", "floors/Wood1.png"},
    {"F6", "floors/Wood1Test.png"},
    {"F7", "floors/Wood1Test2.png"},
    {"F8", "floors/Wood1Test3.png"},
    {"F9", "floors/Wood2.png"},
    {"D1", "doors/Door1.png"},
    {"D2", "doors/Blue_door.png"},
    {"G1", "walls/GlassWall.png"},
    {"H1", "halves/WTC1.png"},
    {"H2", "halves/WTC2.png"},
    {"H3", "halves/WWTC1.png"},
    {"H4", "halves/WWTC2.png"},
    {"H5", "halves/White_Board1.png"},
    {"H6", "halves/White_Board2.png"},
    {"H7", "halves/White_Board3.png"},
    {"H8", "halves/SafeDoor1.png"},
    {"H9", "halves/SafeDoor2.png"},
    {"L1", "tiles/table_front.png"},
    {"L2", "tiles/table_inside.png"},
    {"T1", "tiles/table_top.png"},
    {"Z1", "ceilings/void.png"},
    {"Z2", "floors/void.png"},
    {"R1", "tiles/Railing.png"},
    {"S1", "Sprites/Projector.png"},
    {"S2", "Sprites/Red_BB.png"},
    {"S6", "Sprites/Blue_BB.png"},
    {"S3", "Sprites/Blue2_BB.png"},
    {"S4", "Sprites/Yellow_BB.png"},
    {"S5", "Sprites/Stool.png"},
    {"GG", "waifu.png"}
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
    this->col_has_alpha = std::make_unique<bool[]>(w);
    this->has_alpha = false;
    for (size_t x = 0; x < w; x++) {
        col_has_alpha[x] = false;
    }
    for (size_t i = 0; i < total; i++) {
        pixels[i] = surface_get_pixel(surface, i);
        size_t x = i % w;
        col_has_alpha[x] |= (pixels[i].a < 255);
        has_alpha |= col_has_alpha[x];
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
    if (short_name == "__" || short_name == "--")
        return nullptr;
    try {
        return &textures.at(short_name);
    } catch (std::out_of_range &e) {
        throw Exception("Unknown texture short name:")
            .set_hint(short_name);
    }
}
