#pragma once

#include <vector>
#include <sstream>
#include <unordered_map>
#include <array>

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

    const Texture *top;
    float height = 1;
};

struct Sprite { // btw i prefer coke
    const Texture *tex = nullptr;

    Point<float> pos = {0, 0};
    Point<float> scale = {1, 1};
};

struct ParsedCell {
    Cell cell;

    std::optional<float> player_rot;
    std::optional<Sprite> sprite;
};

struct Model {
    // TODO: check if fixed point is faster on pi
    using Coord = Point<float>;
    struct RayHit {
        const Texture *tex = nullptr;
        const Cell *cell = nullptr;
        float dist;
        Model::Coord pos;
        bool is_ns;
        bool is_near;
    };

    Model() {
    }

    void load_map(std::string filename);
    Cell *get_cell(ssize_t x, ssize_t y);
    const Cell *get_cell(ssize_t x, ssize_t y) const;

    static const size_t max_casts = 3;
    auto cast_ray(
        const Model::Coord ray_dir,
        size_t num_casts = max_casts
    ) const -> std::array<RayHit, max_casts>;

    struct Player {
        Coord pos = Coord(0, 0);
        float rot = 0;
        Coord rot_vec;
    };

    Player player;
    Coord cam_rot_vec;
    std::vector<Sprite> sprites;

    static const std::unordered_map<char, float> cardinal_angles;
    static const std::unordered_map<std::string, Coord> sprite_scales;

    size_t map_w = 0;
    size_t map_h = 0;

    bool debug;
    uint32_t fps = 0;
    uint32_t frame_start_ms = 0;

  private:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

    std::vector<ParsedCell> parse_lines(std::array<MapLine, 3> &lines);

    TextureStore texture_store;
    std::vector<Cell> map;
};