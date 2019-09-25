#include "Controller.hpp"

Controller::Controller() {
}

Controller::~Controller() {
}

void Controller::process_input(uint32_t elapsed_ms) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;

        case SDL_KEYUP:
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = false;
            if (event.key.keysym.scancode == SDL_SCANCODE_F)
                model.debug = true;
            break;

        default:
            break;
        }
    }

    const uint8_t *state = SDL_GetKeyboardState(NULL);
    const auto elapsed_sec = static_cast<Model::Coord::type>(elapsed_ms) / 1000.f;
    const float move_thrust = elapsed_sec * 1.5f;
    const float rot_thrust = elapsed_sec * 180.f;

    if (state[SDL_SCANCODE_LEFT]) {
        model.player.rot -= rot_thrust;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        model.player.rot += rot_thrust;
    }

    model.player.rot_vec = {cos_d(model.player.rot), sin_d(model.player.rot)};
    const float cam_rot = model.player.rot - 90.f;
    model.cam_rot_vec = {cos_d(cam_rot), sin_d(cam_rot)};

    Model::Coord move_vec = {0, 0};
    if (state[SDL_SCANCODE_UP]) {
        move_vec += model.player.rot_vec;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        move_vec -= model.player.rot_vec;
    }
    auto new_pos = model.player.pos + move_vec * move_thrust;
    auto hit = model.cast_ray(move_vec);
    auto hit_dist = comp_euc_dist(model.player.pos, hit.pos);
    auto new_dist = comp_euc_dist(model.player.pos, new_pos);
    // clip x or y if we hit a wall
    if (new_dist >= hit_dist) {
        auto ray_pos = hit.pos - move_vec * 0.02f;
        if (hit.is_ns) {
            new_pos.y = ray_pos.y;
        } else {
            new_pos.x = ray_pos.x;
        }
        // do a second ray cast to fix clipping through corners
        auto ray_dir = (new_pos - model.player.pos).normalize();
        hit = model.cast_ray(ray_dir);
        auto hit_dist = comp_euc_dist(model.player.pos, hit.pos);
        auto new_dist = comp_euc_dist(model.player.pos, new_pos);
        if (hit_dist < new_dist) {
            ray_pos = hit.pos - ray_dir * 0.02f;
            if (hit.is_ns) {
                new_pos.y = ray_pos.y;
            } else {
                new_pos.x = ray_pos.x;
            }
        }
    }
    model.player.pos = new_pos;
}

void Controller::calculate_fps() {
    static uint32_t start_ms = 0;
    static uint32_t frames = 0;
    if (start_ms == 0) {
        start_ms = SDL_GetTicks();
        return;
    }

    frames += 1;
    uint32_t cur_ms = SDL_GetTicks();
    uint32_t elapsed_ms = cur_ms - start_ms;
    if (elapsed_ms >= 1000) {
        model.fps = frames;
        frames = 0;
        start_ms = cur_ms;
    }
}

void Controller::run() {
    model.frame_start_ms = SDL_GetTicks();
    while (running) {
        uint32_t elapsed_ms = SDL_GetTicks() - model.frame_start_ms;
        model.frame_start_ms = SDL_GetTicks();

        model.debug = false;
        process_input(elapsed_ms);

        // insert game logic here
        calculate_fps();

        view.draw(model);
    }
}