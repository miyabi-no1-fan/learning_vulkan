#include "app.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <slp_png.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"
#include "vulkan/vulkan.hpp"
#include "window.hpp"

namespace aglea {

App::App() {
    swap_chain = std::make_unique<SwapChain>(ctx, window.get_extent());

    descriptor_pool =
        DescriptorPool::Builder(ctx)
            .set_max_sets(swap_chain->image_count())
            .add_pool_size(vk::DescriptorType::eUniformBuffer, swap_chain->image_count())
            .add_pool_size(vk::DescriptorType::eUniformTexelBuffer, swap_chain->image_count())
            .build();

    descriptor_set_layout =
        DescriptorSetLayout::Builder(ctx)
            .add_binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
            .add_binding(1, vk::DescriptorType::eUniformTexelBuffer, vk::ShaderStageFlagBits::eFragment)
            .build();

    graphics_pipeline_layout =
        ctx.device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(),
            *descriptor_set_layout->get_descriptor_set_layout()));

    auto graphics_pipeline_config = GraphicsPipeline::default_config();
    graphics_pipeline_config.input_assembly.setTopology(vk::PrimitiveTopology::eTriangleStrip);

    graphics_pipeline = std::make_unique<GraphicsPipeline>(
        ctx,
        graphics_pipeline_layout,
        swap_chain->get_render_pass(),
        0,
        "shaders/dist/shader.vert.spv",
        "shaders/dist/shader.frag.spv",
        graphics_pipeline_config);

    command_buffers.resize(swap_chain->image_count());
    auto cmd = ctx.create_command_buffers(command_buffers.size());
    for (std::size_t i = 0; i < command_buffers.size(); i++) {
        command_buffers[i].first = false;
        command_buffers[i].second = std::move(cmd[i]);
    }
}

void App::create_swap_chain() {
    auto extent = window.get_extent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.get_extent();
        window.wait_events();
    }
    ctx.device->waitIdle();

    std::unique_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
    swap_chain = std::make_unique<SwapChain>(ctx, extent, old_swap_chain.get());
    if (
        old_swap_chain &&
        (!old_swap_chain->cmpeq_swap_formats(*swap_chain) ||
         old_swap_chain->image_count() != swap_chain->image_count())) {
        throw std::runtime_error("Swap chain has changed unexpectedly");
    }
    old_swap_chain = nullptr;
}

void App::run() {
    auto image = *slp::Image::read_png("resources/images/rover.png");
    image.to_rgba8();

    std::vector<std::unique_ptr<Buffer>> global_ubo(swap_chain->image_count());
    for (auto&& v : global_ubo) {
        v = std::make_unique<Buffer>(
            ctx,
            sizeof(GlobalUBO),
            1,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        v->map();
    }

    std::vector<std::unique_ptr<Buffer>> image_buffer(swap_chain->image_count());
    std::vector<vk::UniqueBufferView> image_view(swap_chain->image_count());
    for (std::size_t i = 0; i < swap_chain->image_count(); i++) {
        image_buffer[i] = std::make_unique<Buffer>(
            ctx,
            4,                                       // sizeof rgba8
            image.get_width() * image.get_height(),  // pixels count
            vk::BufferUsageFlagBits::eUniformTexelBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        image_buffer[i]->map();
        image_view[i] = ctx.device->createBufferViewUnique(vk::BufferViewCreateInfo(
            {},
            *image_buffer[i]->get_buffer(),
            vk::Format::eR8G8B8A8Unorm,
            0,
            vk::WholeSize));
    }

    std::vector<vk::UniqueDescriptorSet> descriptor_sets(swap_chain->image_count());
    for (std::size_t i = 0; i < swap_chain->image_count(); i++) {
        descriptor_sets[i] =
            DescriptorWriter(ctx, *descriptor_set_layout, *descriptor_pool)
                .write_buffer(0, global_ubo[i]->descriptor_info())
                .write_buffer(1, image_buffer[i]->descriptor_info(), *image_view[i])
                .build();
    }

    while (!window.should_close()) {
        window.poll_events();
        if (auto i = acquire_next_frame()) {
            auto extent = window.get_extent();
            float width_ratio = static_cast<float>(extent.width) / static_cast<float>(image.get_width());
            float height_ratio = static_cast<float>(extent.height) / static_cast<float>(image.get_height());
            auto* info = static_cast<GlobalUBO*>(global_ubo[*i]->get_mapped_memory());
            if (width_ratio < height_ratio) {
                info->scale_width = 1;
                info->scale_height = image.get_height() * width_ratio / extent.height;
            } else {
                info->scale_width = image.get_width() * height_ratio / extent.width;
                info->scale_height = 1;
            }
            info->width = image.get_width();
            info->height = image.get_height();
            std::memcpy(image_buffer[*i]->get_mapped_memory(), image.data(), image.get_size());

            record_command_buffer(*i, descriptor_sets[*i]);
            submit_frame(*i);
        }
    }
    ctx.device->waitIdle();
}

std::optional<std::uint32_t> App::acquire_next_frame() {
    std::uint32_t i;
    vk::Result res = swap_chain->acquire_next_image(i);
    if (res == vk::Result::eErrorOutOfDateKHR) {
        create_swap_chain();
        for (auto&& p : command_buffers) p.first = false;
        return {};
    }
    if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Can't acquire swap chain image. Vulkan Error code: " + std::to_string((int)res));
    }
    return i;
}

void App::submit_frame(std::uint32_t i) {
    auto res = swap_chain->submit_command_buffer(command_buffers[i].second, i);
    if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || window.was_window_resized()) {
        window.reset_window_resized_flag();
        create_swap_chain();
        for (auto&& p : command_buffers) p.first = false;
    } else if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Can't present swap chain image. Vulkan Error code: " + std::to_string((int)res));
    }
}

void App::record_command_buffer(std::uint32_t i, const vk::UniqueDescriptorSet& descriptor_set) {
    if (!command_buffers[i].first) {
        auto& cmd = command_buffers[i].second;
        cmd->begin(vk::CommandBufferBeginInfo());
        vk::ClearValue clear_values[2];
        clear_values[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
        clear_values[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
        cmd->beginRenderPass(
            vk::RenderPassBeginInfo(
                *swap_chain->get_render_pass(),
                *swap_chain->get_frame_buffer(i),
                vk::Rect2D({ 0, 0 }, swap_chain->get_swap_chain_extent()),
                clear_values),
            vk::SubpassContents::eInline);
        auto extent = swap_chain->get_swap_chain_extent();
        vk::Viewport viewport(
            0.0f,
            0.0f,
            static_cast<float>(extent.width),
            static_cast<float>(extent.height),
            0.0f,
            1.0f);
        vk::Rect2D scissor = { { 0, 0 }, extent };
        cmd->setViewport(0, viewport);
        cmd->setScissor(0, scissor);

        graphics_pipeline->bind(cmd);
        cmd->bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *graphics_pipeline_layout,
            0,
            *descriptor_set,
            {});
        cmd->draw(4, 1, 0, 0);

        cmd->endRenderPass();
        cmd->end();
        command_buffers[i].first = true;
    }
}

}  // namespace aglea
