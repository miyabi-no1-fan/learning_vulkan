#include <cstdint>
#include <iostream>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "context.hpp"
#include "pipeline.hpp"

int main() try {
    VulkanContext ctx{};

    const std::uint32_t N = 4;

    Buffer bufferA = staging_buffer(
        ctx,
        N * sizeof(float),
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [](void* buf) {
            float* data = static_cast<float*>(buf);
            for (std::uint32_t i = 0; i < N; i++)
                data[i] = static_cast<float>(i + 1);
        });

    Buffer bufferB = staging_buffer(
        ctx,
        N * sizeof(float),
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        [](void* buf) {
            float* data = static_cast<float*>(buf);
            for (std::uint32_t i = 0; i < N; i++)
                data[i] = static_cast<float>(i);
        });

    Buffer outBufferLocal(ctx);
    outBufferLocal.allocate(
        N * sizeof(float),
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    ComputePipeline pipeline(ctx);
    pipeline
        .add_binding(&bufferA)         // binding 0
        .add_binding(&bufferB)         // binding 1
        .add_binding(&outBufferLocal)  // binding 2
        .create_pipeline("shaders/dist/shader.comp.spv")
        .update();

    auto run = [&]() {
        auto cmd = std::move(ctx.create_command_buffers(1)[0]);
        cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        pipeline.bind(cmd, PushConstant(N));
        cmd->dispatch(1, 1, 1);

        cmd->end();
        ctx.submit_command_buffer(cmd);
    };

    // out = A - B
    run();  // expected 1 1 1 1 ...

    pipeline.binding_buffers[0] = &bufferA;
    pipeline.binding_buffers[1] = &outBufferLocal;
    pipeline.binding_buffers[2] = &outBufferLocal;
    pipeline.update();

    // out = A - out
    run();  // expected 0 1 2 3 4 ...

    {  // read output
        Buffer outBufferHost(ctx);
        outBufferHost.allocate(
            outBufferLocal.size,
            vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eHostVisible);
        ctx.copy_buffer(outBufferHost.buffer, outBufferLocal.buffer, outBufferLocal.size);
        float* data = static_cast<float*>(outBufferHost.map());
        for (std::uint32_t i = 0; i < N; i++) {
            std::cout << data[i];

            if (i % 2 == 1)
                std::cout << "\n";
            else
                std::cout << " ";
        }
        outBufferHost.unmap();
    }

    return 0;
} catch (const vk::SystemError& e) {
    std::cerr << "Vulkan error: " << e.what() << "\n";
    return 1;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
