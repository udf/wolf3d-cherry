#pragma once

#include <iostream>
#include <cstdint>

#include <SDL2/SDL.h>

#include "Model.hpp"
#include "View.hpp"

class Controller {
  private:
    Controller(const Controller &other) = delete;
    Controller &operator=(const Controller &other) = delete;

    void process_input(uint32_t elapsed_ms);

    View view;

    bool running = true;

  public:
    Controller();
    ~Controller();

    void run();

    Model model;
};