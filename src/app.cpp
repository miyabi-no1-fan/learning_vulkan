#include "app.hpp"

#include <algorithm>
#include <chrono>
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

        camera.set_view_yxz(viewer_object.translation, viewer_object.rotation);
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

Object create_floor(Device& device) {
    Model::Builder builder{};
    builder.vertices = {
        { { -10.f, 1.0f, -10.f }, { .9f, .9f, .0f } },
        { { 10.f, 1.0f, 10.f }, { .9f, .9f, .0f } },
        { { -10.f, 1.0f, 10.f }, { .9f, .9f, .0f } },
        { { 10.f, 1.0f, -10.f }, { .9f, .9f, .0f } },
    };
    builder.indices = { 0, 1, 2, 0, 3, 1 };
    Object floor{};
    floor.model = std::make_shared<Model>(device, builder);
    floor.scale = { .5f, .5f, .5f };
    floor.translation = { .0f, .0f, 2.5f };
    floor.render = [](Object& object, float _, const glm::mat4x4& projection_view) -> void {
        object.projection_view = projection_view;
    };
    return floor;
}

void App::load_objects() {
    std::vector<std::string> paths = {
        // "models/colored_cube.obj",
        // "models/cube.obj",
        // "models/firee.obj",
        // "models/flat_vase.obj",
        "models/smooth_vase.obj",
    };
    for (auto&& path : paths) {
        Object obj{};
        obj.model = Model::create_model_from_file(device, path);
        objects.push_back(std::move(obj));
    }
}
