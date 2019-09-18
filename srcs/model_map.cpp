#include <fstream>
#include <iostream>
#include <array>
#include <utility>
#include <algorithm>
#include <cctype>

#include "Model.hpp"
#include "exception.hpp"

const decltype(Model::cardinal_angles) Model::cardinal_angles{
    {'n', 180},
    {'e', 90},
    {'w', 270},
    {'s', 0}
};

template<size_t N>
static std::array<MapLine, N> get_n_lines(
    std::istream &s,
    size_t last_line
) {
    std::array<MapLine, N> lines;

    for (auto &line : lines) {
        while (1) {
            if (!s.good()) {
                throw Exception("Unexpected end of input")
                    .set_line(last_line);
            }
            last_line++;
            line.line = last_line;
            std::string tmp_line;
            std::getline(s, tmp_line);
            s >> std::ws;
            if (!std::all_of(tmp_line.begin(), tmp_line.end(), ::isspace)) {
                line.ss.str(tmp_line);
                break;
            }
        }
    }
    return lines;
}

template<size_t N>
static std::array<MapToken, N> get_n_tokens(
    MapLine &line
) {
    std::array<MapToken, N> tokens;

    for (auto &token : tokens) {
        token = line.read_token();
    }
    return tokens;
}

std::vector<ParsedCell> Model::parse_lines(std::array<MapLine, 3> &lines) {
    std::vector<ParsedCell> cells;
    while (
        std::any_of(
            lines.begin(),
            lines.end(),
            [](MapLine &line) {
                return line.ss.good();
            }
        )
    ) {
        cells.emplace_back();
        ParsedCell &c = cells.back();

        // TODO: make a function to wrap these in a try/catch so we can add
        // line/column info
        auto tokens = get_n_tokens<3>(lines[0]);
        c.cell.ceil = texture_store.get(tokens[0].val);
        c.cell.wall_top = texture_store.get(tokens[1].val);
        // tokens[2] unused (sprite scale?)

        tokens = get_n_tokens<3>(lines[1]);
        c.cell.wall_left = texture_store.get(tokens[0].val);
        if (tokens[1].val.size() == 2 && tokens[1].val[0] == '@') {
            char direction = tokens[1].val[1];
            try {
                c.player_rot = cardinal_angles.at(direction);
            } catch (std::out_of_range) {
                throw Exception("Unknown direction")
                    .set_hint(std::string(1, direction));
            }
        }
        // tokens[1] unused (sprite?)
        c.cell.wall_right = texture_store.get(tokens[2].val);

        tokens = get_n_tokens<3>(lines[2]);
        c.cell.floor = texture_store.get(tokens[0].val);
        c.cell.wall_bottom = texture_store.get(tokens[1].val);
        // tokens[2] unused (sprite offset?)
    }

    return cells;
}

void Model::load_map(std::string filename) {
    std::ifstream fs(filename);
    if (fs.fail()) {
        throw Exception("Failed to load map file: ")
            .set_hint(strerror(errno));
    }

    size_t last_line = 0;
    std::vector<std::vector<ParsedCell>> tmp_map;
    while (fs.good()) {
        auto lines = get_n_lines<3>(fs, last_line);
        last_line = lines.back().line;
        tmp_map.push_back(parse_lines(lines));
        map_w = std::max(map_w, tmp_map.back().size());
    }
    map_h = tmp_map.size();

    // resize each vector to the max width
    for (auto &row : tmp_map) {
        row.resize(map_w);
    }

    // parse optional data from cells
    for (size_t x = 0; x < map_w; x++) {
        for (size_t y = 0; y < map_h; y++) {
            auto &cell = tmp_map[y][x];
            // TODO: error on multiple player positions
            if (cell.player_rot) {
                player.pos.x = static_cast<float>(x);
                player.pos.y = static_cast<float>(y);
                player.rot = cell.player_rot.value();
            }
        }
    }

    for (auto &row : tmp_map) {
        for (auto &item : row) {
            map.push_back(item.cell);
        }
    }
}

Cell *Model::get_cell(ssize_t x, ssize_t y) {
    if (x < 0 || y < 0)
        return nullptr;

    size_t x_ = static_cast<size_t>(x);
    size_t y_ = static_cast<size_t>(y);
    if (x_ >= map_w || y_ >= map_h)
        return nullptr;

    return &map[y_ * map_w + x_];
}