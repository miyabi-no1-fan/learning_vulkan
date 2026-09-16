#include "app.hpp"

#include <array>
#include <memory>
#include <vector>

#include "objects/my_triangle.hpp"
#include "render_system.hpp"

App::App() {
    load_objects();
}

App::~App() {}

void App::run() {
    RenderSystem render_system{ device, renderer.get_renderpass() };

    while (!window.should_close()) {
        renderer.begin_frame();
        if (renderer.is_frame_in_progress()) {
            renderer.begin_renderpass();
            render_system.render_objects(renderer.get_current_command_buffer(), objects);
            renderer.end_renderpass();
            renderer.end_frame();
        }
        ++current_frame;
        window.poll_events();
    }

    auto _ = vkDeviceWaitIdle(device.device());
}

void App::load_objects() {
    objects.push_back(
        std::make_unique<Triangle>(
            device,
            std::array<Model::Vertex, 3>({
                { { -0.5f, 0.5f }, { 1.0, 0.0, 0.0 } },
                { { 0.0f, -0.5f }, { 0.0, 1.0, 0.0 } },
                { { 0.5f, 0.5f }, { 0.0, 0.0, 1.0 } },
            }))  //
    );
}
