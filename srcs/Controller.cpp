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
    auto thrust = static_cast<Model::Coord::type>(elapsed_ms) * 0.1f;

    if (state[SDL_SCANCODE_LEFT])
        model.player.pos.x -= thrust;
    if (state[SDL_SCANCODE_RIGHT])
        model.player.pos.x += thrust;
    if (state[SDL_SCANCODE_UP])
        model.player.pos.y -= thrust;
    if (state[SDL_SCANCODE_DOWN])
        model.player.pos.y += thrust;
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