#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <slp_png.hpp>
#include <vulkan/vulkan.hpp>

#include "aglea.hpp"
#include "buffer.hpp"
#include "context.hpp"
#include "descriptors.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "utils.hpp"

using vec2 = aglea::vec<float, 2>;
using vec3 = aglea::vec<float, 3>;
using vec4 = aglea::vec<float, 4>;

using uvec2 = aglea::vec<std::uint32_t, 2>;
using uvec3 = aglea::vec<std::uint32_t, 3>;
using uvec4 = aglea::vec<std::uint32_t, 4>;

using mat2x2 = aglea::mat<float, 2, 2>;
using mat3x3 = aglea::mat<float, 3, 3>;
using mat4x4 = aglea::mat<float, 4, 4>;

struct GlobalInfo {
    using uint = unsigned int;

    uint src_width;
    uint src_height;
    float src_width_center;
    float src_height_center;

    uint dst_width;
    uint dst_height;
    float dst_width_center;
    float dst_height_center;

    mat2x2 inverse_matrix;

    uvec4 background_color;
};

int main() try {
    VulkanContext ctx{};

    constexpr mat2x2 matrix = {
        { 1.f, 0.5f },
        { 0.f, 1.f },
    };

    constexpr uvec4 background_color = { 255, 60, 255, 255 };

    auto image = *slp::Image::read_png("resources/images/rover.png");
    image.to_rgba8();

    auto [new_width, new_height] = get_linear_transformed_dimension(image.get_width(), image.get_height(), matrix);
    auto output = slp::Image::uninitialized(slp::Image::Extent(new_width, new_height), image.get_format());

    if (output.get_size() == 0) {
        *output.write_png("a.png");
        return 0;
    }

    Image src_image = staging_buffer(
        ctx,
        image.get_size(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Uint,
        vk::Extent3D(
            static_cast<std::uint32_t>(image.get_width()),
            static_cast<std::uint32_t>(image.get_height()),
            1),
        vk::ImageUsageFlagBits::eStorage,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [&image](void* data) {
            std::memcpy(data, image.data(), image.get_size());
        });
    image.clear();  // free up image pixels data

    Image dst_image_local(
        ctx,
        src_image.get_type(),
        src_image.get_format(),
        vk::Extent3D(
            static_cast<std::uint32_t>(output.get_width()),
            static_cast<std::uint32_t>(output.get_height()),
            1),
        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    Buffer infoBuffer = staging_buffer(
        ctx,
        sizeof(GlobalInfo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [&image, &output, &matrix, &background_color](void* data) {
            const float det = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
            *static_cast<GlobalInfo*>(data) = {
                .src_width = static_cast<std::uint32_t>(image.get_width()),
                .src_height = static_cast<std::uint32_t>(image.get_height()),
                .src_width_center = static_cast<float>(image.get_width() - 1) / 2.f,
                .src_height_center = static_cast<float>(image.get_height() - 1) / 2.f,

                .dst_width = static_cast<std::uint32_t>(output.get_width()),
                .dst_height = static_cast<std::uint32_t>(output.get_height()),
                .dst_width_center = static_cast<float>(output.get_width() - 1) / 2.f,
                .dst_height_center = static_cast<float>(output.get_height() - 1) / 2.f,

                .inverse_matrix = {
                    { matrix[1][1] / det, -matrix[0][1] / det },
                    { -matrix[1][0] / det, matrix[0][0] / det },
                },

                .background_color = background_color,
            };
        });

    auto descriptor_pool =
        DescriptorPool::Builder(ctx)
            .setMaxSets(1)
            .addPoolSize(vk::DescriptorType::eStorageImage, 1)
            .addPoolSize(vk::DescriptorType::eStorageImage, 1)
            .addPoolSize(vk::DescriptorType::eUniformBuffer, 1)
            .build();

    auto descriptor_set_layout =
        DescriptorSetLayout::Builder(ctx)
            .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
            .addBinding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
            .addBinding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute)
            .build();

    auto descriptor_set =
        DescriptorWriter(ctx, descriptor_set_layout, descriptor_pool)
            .writeImage(0, src_image.descriptor_info())
            .writeImage(1, dst_image_local.descriptor_info())
            .writeBuffer(2, infoBuffer.descriptor_info())
            .build();

    ComputePipeline pipeline(
        ctx,
        descriptor_set_layout.getDescriptorSetLayout(),
        "shaders/dist/shader.comp.spv");

    ctx.one_time_submit([&pipeline, &output, &descriptor_set](vk::UniqueCommandBuffer& cmd) {
        pipeline.bind(cmd, PushConstant(0));
        cmd->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, *pipeline.pipeline_layout, 0, *descriptor_set, {});
        constexpr std::uint32_t localSizeX = 16;
        constexpr std::uint32_t localSizeY = 16;
        std::uint32_t groupCountX = div_ceil(output.get_width(), localSizeX);
        std::uint32_t groupCountY = div_ceil(output.get_height(), localSizeY);
        std::uint32_t groupCountZ = 1;
        cmd->dispatch(groupCountX, groupCountY, groupCountZ);
    });

    {  // read output
        Buffer dst_buffer_host(
            ctx,
            1,
            output.get_size(),
            vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        ctx.one_time_submit([&dst_image_local, &dst_buffer_host](vk::UniqueCommandBuffer& cmd) {
            vk::BufferImageCopy region(
                {}, {}, {}, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, dst_image_local.get_extent());
            cmd->copyImageToBuffer(*dst_image_local.get_image(), vk::ImageLayout::eGeneral, *dst_buffer_host.get_buffer(), region);
        });
        dst_buffer_host.map();
        std::memcpy(
            output.data(),
            dst_buffer_host.get_mapped_memory(),
            output.get_size());
        dst_buffer_host.unmap();
    }

    *output.write_png("a.png");
    return 0;
} catch (const vk::SystemError& e) {
    std::cerr << "Vulkan error: " << e.what() << "\n";
    return 1;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
