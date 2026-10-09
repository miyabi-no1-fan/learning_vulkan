#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "context.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

namespace aglea {

class Renderer {
    Window& window;
    const Context& ctx;
    std::unique_ptr<SwapChain> swap_chain{};
    std::vector<vk::UniqueCommandBuffer> command_buffers{};

   public:
    Renderer(Window& window, const Context& ctx);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    const vk::UniqueRenderPass& get_renderpass() const { return swap_chain->get_render_pass(); }
    std::size_t get_swap_chain_image_count() const { return swap_chain->image_count(); }
    bool is_frame_in_progress() const { return is_frame_started; }

    const vk::UniqueCommandBuffer& get_current_command_buffer() const {
        assert(is_frame_in_progress() && "Can't get command buffer when frame is not in progress");
        return command_buffers[get_current_frame_index()];
    }

    std::uint32_t get_current_frame_index() const {
        assert(is_frame_in_progress() && "Can't get current frame when frame is not in progress");
        return current_frame_index;
    }

    void begin_frame();
    void end_frame();
    void begin_renderpass();
    void end_renderpass();

   private:
    void create_command_buffers();
    void create_swap_chain();

    std::uint32_t current_image_index = 0;
    std::uint32_t current_frame_index = 0;
    bool is_frame_started = false;
};

}  // namespace aglea