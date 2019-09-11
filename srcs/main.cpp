#include <stdexcept>

#include "Controller.hpp"

int main() {
    try {
        Controller controller;
        controller.run();
    } catch (std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}