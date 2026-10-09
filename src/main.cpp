#include <cstring>
#include <iostream>
#include <slp_png.hpp>
#include <vulkan/vulkan.hpp>

#include "app.hpp"

const char* HELP =
    "Usage: aglea <PNG file>"
    ""
    "Display the PNG file.";

void help() {
    std::cout << HELP << std::endl;
}

int main(int argc, const char* argv[]) try {
    if (argc < 2) {
        std::cerr << "Error: no input file" << std::endl;
        help();
        return 1;
    }

    if (argc > 2) {
        std::cerr << "Unknown commands" << std::endl;
        help();
        return 1;
    }

    if (std::memcmp(argv[1], "--help", sizeof("--help")) ||
        std::memcmp(argv[1], "-h", sizeof("-h"))) {
        help();
        return 0;
    }

    auto image = *slp::Image::read_png(argv[1]);
    image.to_rgba8();

    aglea::App app(image.get_width(), image.get_height());

    while (!app.should_close()) {
        app.render(image.data());
    }

    return 0;
} catch (const vk::SystemError& e) {
    std::cerr << "Vulkan error: " << e.what() << "\n";
    return 1;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
