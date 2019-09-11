#pragma once

#include "Point.hpp"

struct Model {
    // TODO: check if fixed point is faster on pi
    using Coord = Point<float>;

    Coord player_pos = Coord(0, 0);

  private:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;
};