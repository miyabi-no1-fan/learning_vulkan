#include "app.hpp"

#include <cmath>
#include <memory>
#include <numbers>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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
    auto triangle_model = std::make_shared<Model>(
        device,
        std::vector<Model::Vertex>{
            { { -1.0f, 1.0f }, { 1.0, 1.0, 0.0 } },
            { { 1.0f, 1.0f }, { 1.0, 1.0, 0.0 } },
            { { 1.0f, -1.0f }, { 1.0, 1.0, 0.0 } },
        }  //
    );

    {
        Object triangle{};
        triangle.model = triangle_model;
        triangle.transform2d.matrix = { 0.5f };
        triangle.transform2d.shift = { 0.0f, 0.0f };
        objects.push_back(std::move(triangle));
    }

    {
        glm::mat2x2 mat;
        {
            float angle = std::numbers::pi;
            auto rotate = glm::mat2x2({
                { std::cos(angle), -std::sin(angle) },
                { std::sin(angle), std::cos(angle) },
            });
            mat = rotate * 0.5f;
        }

        Object triangle{};
        triangle.model = triangle_model;
        triangle.transform2d.matrix = mat;
        triangle.transform2d.shift = { 0.0f, 0.0f };
        objects.push_back(std::move(triangle));
    }
}
