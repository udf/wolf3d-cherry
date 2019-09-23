#include <stdexcept>

#include "Controller.hpp"

int main(int ac, char **av) {
    try {
        Controller controller;
	ac == 2 ? controller.model.load_map(av[1]) : controller.model.load_map("test.map");
        controller.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
