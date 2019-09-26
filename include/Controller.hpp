#pragma once

#include <iostream>
#include <cstdint>
#include <unordered_map>

#include <SDL2/SDL.h>

#include "Model.hpp"
#include "View.hpp"
#include "util.hpp"

class Controller {
  private:
    Controller(const Controller &other) = delete;
    Controller &operator=(const Controller &other) = delete;

    void process_input(uint32_t elapsed_ms);
    void calculate_fps();

    const static std::unordered_map<std::string, std::string> map_shortnames;
    View view;

    bool running = true;

  public:
    Controller();
    ~Controller();

    void run();

    Model model;
};
