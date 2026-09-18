#include "app.hpp"

#include <memory>
#include <vector>

#include "render_system.hpp"
#include "transform.hpp"

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
    std::vector<Model::Vertex> vertices{

        // left face (white)
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
        { { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },
        { { -.5f, .5f, .5f }, { .9f, .9f, .9f } },

        // right face (yellow)
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
        { { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .8f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .8f, .1f } },

        // top face (orange)
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
        { { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },
        { { .5f, -.5f, .5f }, { .9f, .6f, .1f } },

        // bottom face (red)
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
        { { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, -.5f }, { .8f, .1f, .1f } },
        { { .5f, .5f, .5f }, { .8f, .1f, .1f } },

        // nose face (blue)
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
        { { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
        { { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },

        // tail face (green)
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
        { { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
        { { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },

    };

    Object cube{};
    cube.model = std::make_shared<Model>(device, vertices);
    cube.transform.scalar = { .5f, .5f, .5f };
    cube.transform.offset = { .0f, .0f, .5f };
    objects.push_back(std::move(cube));
}
