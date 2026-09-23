#include "app.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <memory>
#include <numbers>
#include <vector>

#include "buffer.hpp"
#include "camera.hpp"
#include "frame_info.hpp"
#include "keyboard.hpp"
#include "render_system.hpp"

App::App() {
    global_descriptor_pool =
        DescriptorPool::Builder(device)
            .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    load_objects();
}

App::~App() {}

void App::run() {
    std::vector<std::unique_ptr<Buffer>> uniform_buffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto&& buffer : uniform_buffers) {
        buffer = std::make_unique<Buffer>(
            device,
            sizeof(GlobalUniformBuffer),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            device.properties.limits.minUniformBufferOffsetAlignment  //
        );
        buffer->map();
    }

    auto global_descriptor_set_layout =
        DescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();

    std::vector<VkDescriptorSet> global_descriptor_sets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < global_descriptor_sets.size(); i++) {
        auto descriptor_info = uniform_buffers[i]->descriptor_info();
        DescriptorWriter(*global_descriptor_set_layout, *global_descriptor_pool)
            .writeBuffer(0, &descriptor_info)
            .build(global_descriptor_sets[i]);
    }

    RenderSystem render_system(
        device,
        renderer.get_renderpass(),
        global_descriptor_set_layout->getDescriptorSetLayout()  //
    );
    Camera camera{};

    Object viewer_object{};
    Controller camera_controller{};

    auto current_time = std::chrono::steady_clock::now();

    while (!window.should_close()) {
        window.poll_events();

        FrameInfo frame{};

        auto new_time = std::chrono::steady_clock::now();
        frame.time = std::min(std::chrono::duration<double>(new_time - current_time).count(), MAX_FRAME_TIME);
        current_time = new_time;

        camera_controller.move(window, frame.time, viewer_object);
        camera.set_view_yxz(viewer_object.translation, viewer_object.rotation);
        camera.set_perspective_projection(50.f * std::numbers::pi / 180.f, renderer.get_aspect_ratio(), 0.1f, 10.f);

        renderer.begin_frame();
        if (renderer.is_frame_in_progress()) {
            frame.index = renderer.get_current_frame_index();
            frame.command_buffer = renderer.get_current_command_buffer();
            frame.global_descriptor_set = global_descriptor_sets[frame.index];

            GlobalUniformBuffer uniform_buffer{};
            uniform_buffer.projection_view = camera.get_projection() * camera.get_view();
            uniform_buffers[frame.index]->write_to_buffer(&uniform_buffer);

            renderer.begin_renderpass();
            render_system.render_objects(frame, objects);
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
