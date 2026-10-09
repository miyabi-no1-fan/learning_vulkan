#include <iostream>
#include <vulkan/vulkan.hpp>

#include "app.hpp"

int main() try {
    aglea::App app;
    app.run();
    return 0;
} catch (const vk::SystemError& e) {
    std::cerr << "Vulkan error: " << e.what() << "\n";
    return 1;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
