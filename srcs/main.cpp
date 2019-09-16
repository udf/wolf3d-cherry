#include <stdexcept>

#include "Controller.hpp"

int main() {
    try {
        Controller controller;
        controller.model.load_map("test.map");
        controller.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}