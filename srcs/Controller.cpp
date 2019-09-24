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

    if (state[SDL_SCANCODE_UP]) {
        model.player.pos += model.player.rot_vec * move_thrust;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        model.player.pos -= model.player.rot_vec * move_thrust;
    }
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