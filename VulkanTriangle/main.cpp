#include <iostream>
#include <stdexcept>
#include <functional>

#include "HellloTriangleApplication.hpp"

int main() {
    auto application = app::HelloTriangleApplication{};

    try {
        application.run();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}