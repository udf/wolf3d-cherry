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
            break;

        default:
            break;
        }
    }

    const uint8_t *state = SDL_GetKeyboardState(NULL);
    auto elapsed_f = static_cast<Model::Coord::type>(elapsed_ms);

    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_RIGHT]) {
        auto thrust = elapsed_f * 0.25f;
        if (state[SDL_SCANCODE_RIGHT])
            thrust *= -1;
        model.player.rot += thrust;
    }
    model.player.rot_vec = {sin_d(model.player.rot), cos_d(model.player.rot)};

    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN]) {
        auto thrust = elapsed_f * 1.0f;
        if (state[SDL_SCANCODE_DOWN])
            thrust *= -1;
        model.player.pos += model.player.rot_vec * thrust;
    }
}

void Controller::run() {
    uint32_t prev_ms = SDL_GetTicks();
    while (running) {
        uint32_t elapsed_ms = SDL_GetTicks() - prev_ms;
        prev_ms = SDL_GetTicks();

        process_input(elapsed_ms);

        // insert game logic here

        view.draw(model);
    }
}