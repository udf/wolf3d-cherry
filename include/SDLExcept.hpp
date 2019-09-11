#pragma once

#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>

// std::exception but it grabs SDL_GetError
class SDLExcept : public std::exception {
  private:
    SDLExcept(const SDLExcept &other) = delete;
    SDLExcept &operator=(const SDLExcept &other) = delete;

    std::string message;

  public:
    SDLExcept(const char *msg) {
        this->message = msg;
        std::string sdl_err = SDL_GetError();
        if (!sdl_err.empty())
            this->message += "\nSDL error: " + sdl_err;
    }

    ~SDLExcept() {
    }

    const char* what() const throw() {
        return this->message.c_str();
    }
};