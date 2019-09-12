#pragma once

#include "Point.hpp"
#include "texture_store.hpp"

struct Model {
    Model() {
        player_tex = texture_store.get("W3");
    }

    // TODO: check if fixed point is faster on pi
    using Coord = Point<float>;

    Coord player_pos = Coord(0, 0);

    const Texture *player_tex;

  private:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    TextureStore texture_store;
};