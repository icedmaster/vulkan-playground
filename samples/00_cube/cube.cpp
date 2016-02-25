#include "vulkan_helper.hpp"

using namespace mhe;

int main(int, char**)
{
    VulkanContext context;
    VkResult res = init_vulkan_context(context, "vk_cube", 512, 512);
    VERIFY(res == VK_SUCCESS, "init_vulkan_context failed", -1);

    // create a command buffer from the main command pool
    VkCommandBuffer command_buffer;

    VkCommandBufferAllocateInfo cb_allocate_info;
    init_defaults(cb_allocate_info);
    cb_allocate_info.commandBufferCount = 1;
    cb_allocate_info.commandPool = context.main_command_pool;
    res = vkAllocateCommandBuffers(context.device, &cb_allocate_info, &command_buffer);
    ASSERT(res == VK_SUCCESS, "vkAllocateCommandBuffers failed");

    // create a couple of simple commands
    VkCommandBufferBeginInfo begin_info;
    init_defaults(begin_info);
    res = vkBeginCommandBuffer(command_buffer, &begin_info);
    ASSERT(res == VK_SUCCESS, "vkBeginCommandBuffer failed");

    {
        VkClearValue clear_values[2];
        clear_values[0].color = {0.0f, 1.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.clearValueCount = 2;
        render_pass_begin_info.framebuffer = context.main_framebuffer;
        render_pass_begin_info.pClearValues = clear_values;
        render_pass_begin_info.pNext = nullptr;
        render_pass_begin_info.renderArea = {0, 0, 512, 512};
        render_pass_begin_info.renderPass = context.main_render_pass;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.height = viewport.width = 512;
        viewport.x = viewport.y = 0;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D sciccor_rect = {0, 0, 512, 512};
        vkCmdSetScissor(command_buffer, 0, 1, &sciccor_rect);

        vkCmdEndRenderPass(command_buffer);
    }

    res = vkEndCommandBuffer(command_buffer);
    ASSERT(res == VK_SUCCESS, "vkEndCommandBuffer failed");

    uint32_t current_buffer = 0;

    res = vkAcquireNextImageKHR(context.device, context.swapchain, std::numeric_limits<uint64_t>::max(), 0, 0, &current_buffer);
    ASSERT(res == VK_SUCCESS, "vkAcquireNextImageKHR failed");

    VkSubmitInfo submit_info;
    init_defaults(submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    res = vkQueueSubmit(context.graphics_queue, 1, &submit_info, 0);
    ASSERT(res == VK_SUCCESS, "vkQueueSubmit failed");

    VkPresentInfoKHR present_info;
    init_defaults(present_info);
    present_info.pSwapchains = &context.swapchain;
    present_info.swapchainCount = 1;
    present_info.pImageIndices = &current_buffer;
    res = vkQueuePresentKHR(context.graphics_queue, &present_info);
    ASSERT(res == VK_SUCCESS, "vkQueuePresentKHR failed");

    res = vkQueueWaitIdle(context.graphics_queue);
    VERIFY(res == VK_SUCCESS, "vkQueueWaitIdle failed", -1);

    while (app_message_loop(context))
    {
    }

    vkFreeCommandBuffers(context.device, context.main_command_pool, 1, &command_buffer);

    destroy_vulkan_context(context);

    return 0;
}