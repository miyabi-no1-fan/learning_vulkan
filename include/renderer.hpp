#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "device.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

class Renderer {
   private:
    Window& window;
    Device& device;
    std::unique_ptr<SwapChain> swap_chain{};
    std::vector<VkCommandBuffer> command_buffers{};

   public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    VkRenderPass get_renderpass() const { return swap_chain->getRenderPass(); }

    bool is_frame_in_progress() const { return is_frame_started; }

    VkCommandBuffer get_current_command_buffer() const {
        assert(is_frame_in_progress() && "Can't get command buffer when frame is not in progress");
        return command_buffers[current_frame_index];
    }

    uint32_t get_current_frame_index() const {
        assert(is_frame_in_progress() && "Can't get current frame when frame is not in progress");
        return current_frame_index;
    }

    void begin_frame();
    void end_frame();
    void begin_renderpass();
    void end_renderpass();

   private:
    void create_command_buffers();
    void free_command_buffers();

    void create_swap_chain();

    uint32_t current_image_index = 0;
    uint32_t current_frame_index = 0;
    bool is_frame_started = false;
};
