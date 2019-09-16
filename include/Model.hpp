#pragma once

#include <vector>
#include <sstream>

#include "Point.hpp"
#include "texture_store.hpp"

struct MapToken {
    std::string val;
    size_t line = 0;
    size_t col = 0;
};

struct MapLine {
    std::istringstream ss;
    size_t line = 0;
    size_t col = 0;

    MapToken read_token() {
        MapToken token;
        if (!ss.good())
            throw Exception("Unexpected end of line")
                .set_line(line)
                .set_column(col);
        token.col = static_cast<size_t>(ss.tellg()) + 1;
        col = token.col;
        token.line = line;
        ss >> token.val;
        ss >> std::ws;
        return token;
    }
};

struct Cell {
    const Texture *ceil = nullptr;
    const Texture *floor = nullptr;

    const Texture *wall_top = nullptr;
    const Texture *wall_bottom = nullptr;
    const Texture *wall_left = nullptr;
    const Texture *wall_right = nullptr;
};

struct Model {
    Model() {
        player_tex = texture_store.get("W3");
    }

    void load_map(std::string filename);
    Cell *get_cell(ssize_t x, ssize_t y);

    // TODO: check if fixed point is faster on pi
    using Coord = Point<float>;

    Coord player_pos = Coord(0, 0);

    const Texture *player_tex;

  private:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    std::vector<Cell> parse_lines(std::array<MapLine, 3> &lines);

    TextureStore texture_store;
    std::vector<Cell> map;
    size_t map_w = 0;
    size_t map_h = 0;
};