#include <stdexcept>

#include "Controller.hpp"

int main() {
    try {
        Controller controller;
        controller.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}