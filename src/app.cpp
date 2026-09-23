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
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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

        auto new_time = std::chrono::steady_clock::now();
        double frame_time = std::min(std::chrono::duration<double>(new_time - current_time).count(), MAX_FRAME_TIME);
        current_time = new_time;

        camera_controller.move(window, frame_time, viewer_object);
        camera.set_view_yxz(viewer_object.translation, viewer_object.rotation);
        camera.set_perspective_projection(50.f * std::numbers::pi / 180.f, renderer.get_aspect_ratio(), 0.1f, 100.f);

        renderer.begin_frame();
        if (renderer.is_frame_in_progress()) {
            uint32_t frame_index = renderer.get_current_frame_index();

            GlobalUniformBuffer uniform_buffer{};
            uniform_buffer.projection_view = camera.get_projection() * camera.get_view();
            uniform_buffers[frame_index]->write_to_buffer(&uniform_buffer);

            FrameInfo frame{
                .index = frame_index,
                .time = frame_time,
                .command_buffer = renderer.get_current_command_buffer(),
                .global_descriptor_set = global_descriptor_sets[frame_index],
                .objects = objects,
            };

            renderer.begin_renderpass();
            render_system.render_objects(frame);
            renderer.end_renderpass();
            renderer.end_frame();
        }

        ++current_frame;
    }

    auto _ = vkDeviceWaitIdle(device.device());
}

void App::load_objects() {
    std::vector<std::string> paths = {
        "models/colored_cube.obj",
        "models/cube.obj",
        "models/firee.obj",
        "models/flat_vase.obj",
        "models/smooth_vase.obj",
        "models/floor.obj",
    };
    {
        Object obj{};
        obj.model = Model::create_model_from_file(device, paths[4]);
        obj.translation = { 1.f, .0f, .0f };
        objects[obj.get_id()] = std::move(obj);
    }
    {
        Object obj{};
        obj.model = Model::create_model_from_file(device, paths[4]);
        obj.translation = { -1.f, .0f, .0f };
        objects[obj.get_id()] = std::move(obj);
    }
    {
        Object obj{};
        obj.model = Model::create_model_from_file(device, paths[5]);
        obj.translation = { .0f, .0f, .0f };
        obj.scale = { 5.f, 5.f, 5.f };
        objects[obj.get_id()] = std::move(obj);
    }
}
