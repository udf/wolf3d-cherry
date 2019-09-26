#include "Model.hpp"

auto Model::cast_ray(
    const Model::Coord ray_dir,
    size_t num_casts,
    bool collision
) const -> std::array<RayHit, max_casts> {
    num_casts = std::min(num_casts, max_casts);

    // Which cell of the map we're in
    Point<ssize_t> map = player.pos.cast_to<ssize_t>();

    // Length of ray from current position to next side
    Model::Coord side_dist;

    // Length of ray from one side to next
    const Model::Coord delta_dist = {
        std::abs(1.f / ray_dir.x), std::abs(1.f / ray_dir.y)
    };

    // What direction to step in
    const Point<ssize_t> step = {
        ray_dir.x < 0 ? -1 : 1,
        ray_dir.y < 0 ? -1 : 1
    };

    std::array<RayHit, max_casts> hits;

    // Which wall to check for the near and far cell
    const Texture *Cell::* ew_near_texture;
    const Texture *Cell::* ew_far_texture;
    const Texture *Cell::* ns_near_texture;
    const Texture *Cell::* ns_far_texture;

    // Calculate initial sideDist
    if (ray_dir.x < 0) {
        side_dist.x = (player.pos.x - (float)map.x) * delta_dist.x;
        ew_near_texture = &Cell::wall_left;
        ew_far_texture = &Cell::wall_right;
    } else {
        side_dist.x = ((float)map.x + 1.0f - player.pos.x) * delta_dist.x;
        ew_near_texture = &Cell::wall_right;
        ew_far_texture = &Cell::wall_left;
    }
    if (ray_dir.y < 0) {
        side_dist.y = (player.pos.y - (float)map.y) * delta_dist.y;
        ns_near_texture = &Cell::wall_top;
        ns_far_texture = &Cell::wall_bottom;
    } else {
        side_dist.y = ((float)map.y + 1.0f - player.pos.y) * delta_dist.y;
        ns_near_texture = &Cell::wall_bottom;
        ns_far_texture = &Cell::wall_top;
    }

    // Do the thing
    size_t i = 0;
    while (i < num_casts) {
        auto &hit = hits[i];
        if (map.x < 0 || map.y < 0 || (size_t)map.x >= map_w || (size_t)map.y >= map_h)
            break; // down and cry
        if (side_dist.x < side_dist.y) {
            map.x += step.x;
            hit.is_ns = false;
        } else {
            map.y += step.y;
            hit.is_ns = true;
        }

        auto check_cell = [this, collision, &hit](
            bool is_near, ssize_t x, ssize_t y, const Texture *Cell::* tex_member
        ) {
            if (hit.tex)
                return;
            auto cell = get_cell(x, y);
            if (cell) {
                auto tex = cell->*tex_member;
                if (!tex)
                    return;
                if (collision && !tex->is_solid)
                    return;
                hit.tex = cell->*tex_member;
                hit.cell = cell;
                hit.is_near = is_near;
            }
        };

        ssize_t mapNearX = map.x;
        ssize_t mapNearY = map.y;
        if (hit.is_ns) {
            mapNearY -= step.y;
            check_cell(true, mapNearX, mapNearY, ns_near_texture);
            check_cell(false, map.x, map.y, ns_far_texture);
            hit.pos = player.pos + ray_dir * side_dist.y;
        } else {
            mapNearX -= step.x;
            check_cell(true, mapNearX, mapNearY, ew_near_texture);
            check_cell(false, map.x, map.y, ew_far_texture);
            hit.pos = player.pos + ray_dir * side_dist.x;
        }

        if (hit.tex) {
            hit.dist = (
                hit.is_ns
                ? ((float)map.y - player.pos.y + (1.f - (float)step.y) / 2.f) / ray_dir.y
                : ((float)map.x - player.pos.x + (1.f - (float)step.x) / 2.f) / ray_dir.x
            );
            if (!hit.tex->has_alpha)
                break;
            i++;
        }

        if (hit.is_ns) {
            side_dist.y += delta_dist.y;
        } else {
            side_dist.x += delta_dist.x;
        }
    }
    return hits;
}