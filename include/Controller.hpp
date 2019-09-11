#pragma once

#include <iostream>

#include "Model.hpp"
#include "View.hpp"

class Controller {
  private:
    Controller(const Controller &other) = delete;
    Controller &operator=(const Controller &other) = delete;

    View view;
    Model model;

  public:
    Controller();
    ~Controller();

    void run();
};