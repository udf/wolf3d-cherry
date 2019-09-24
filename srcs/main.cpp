#include <stdexcept>

#include "Controller.hpp"

int main(int ac, char **av) {
    try {
        Controller controller;
        controller.model.load_map(ac == 2 ? av[1] : "test.map");
        controller.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
