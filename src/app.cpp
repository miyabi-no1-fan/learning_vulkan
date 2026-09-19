#include "app.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <numbers>
#include <vector>

#include "camera.hpp"
#include "keyboard.hpp"
#include "render_system.hpp"

App::App() {
    load_objects();
}

App::~App() {}

void App::run() {
    RenderSystem render_system{ device, renderer.get_renderpass() };
    Camera camera{};

    Object viewer_object{};
    Controller camera_controller{};

    auto current_time = std::chrono::steady_clock::now();

    while (!window.should_close()) {
        window.poll_events();

        auto new_time = std::chrono::steady_clock::now();
        double frame_time = std::chrono::duration<double>(new_time - current_time).count();
        current_time = new_time;

        frame_time = std::min(frame_time, MAX_FRAME_TIME);

        camera_controller.move(window, frame_time, viewer_object);

        camera.set_view_yxz(viewer_object.transform.translation, viewer_object.transform.rotation);
        camera.set_perspective_projection(50.f * std::numbers::pi / 180.f, renderer.get_aspect_ratio(), 0.1f, 10.f);

        renderer.begin_frame();
        if (renderer.is_frame_in_progress()) {
            renderer.begin_renderpass();
            render_system.render_objects(renderer.get_current_command_buffer(), objects, camera, frame_time);
            renderer.end_renderpass();
            renderer.end_frame();
        }
        ++current_frame;
    }

    auto _ = vkDeviceWaitIdle(device.device());
}

void App::load_objects() {
    Model::Builder builder{};

    builder.vertices = {
        // left face (white)
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },

        // right face (yellow)
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .8f, .1f } },

        // top face (orange)
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },

        // bottom face (red)
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .1f, .1f } },

        // nose face (blue)
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },

        // tail face (green)
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
    };
    builder.indices = { 0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9, 12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

    {
        Object cube{};
        cube.model = std::make_shared<Model>(device, builder);
        cube.transform.scale = { .5f, .5f, .5f };
        cube.transform.translation = { .0f, .0f, 2.5f };
        cube.render = [](Object& object, float dt, const glm::mat4x4& projection_view) -> void {
            object.transform.projection_view = projection_view;
            object.transform.rotation.y += dt * .9f;

            for (int i = 0; i < object.transform.rotation.length(); i++) {
                object.transform.rotation[i] =
                    std::fmodf(object.transform.rotation[i],
                               static_cast<float>(std::numbers::pi) * 2.f);
            }
        };
        objects.push_back(std::move(cube));
    }

    builder.vertices = {
        { { -10.f, .5f, -10.f }, { .9f, .9f, .9f } },
        { { 10.f, .5f, 10.f }, { .9f, .9f, .9f } },
        { { -10.f, .5f, 10.f }, { .9f, .9f, .9f } },
        { { 10.f, .5f, -10.f }, { .9f, .9f, .9f } },
    };
    builder.indices = { 0, 1, 2, 0, 3, 1 };

    {
        Object floor{};
        floor.model = std::make_shared<Model>(device, builder);
        floor.transform.scale = { .5f, .5f, .5f };
        floor.transform.translation = { .0f, .0f, 2.5f };
        floor.render = [](Object& object, float _, const glm::mat4x4& projection_view) -> void {
            object.transform.projection_view = projection_view;
        };
        objects.push_back(std::move(floor));
    }
}
