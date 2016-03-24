#include "vulkan_helper.hpp"

#include <limits>

using namespace mhe;

int main(int, char**)
{
    VulkanContext context;
    VkResult res = init_vulkan_context(context, "vk_cube", 512, 512, false);
    VERIFY(res == VK_SUCCESS, "init_vulkan_context failed", -1);

    // create a pipeline
    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo pipeline_create_info;
    init_defaults(pipeline_create_info);
    // vertex input
    VkPipelineVertexInputStateCreateInfo vi_create_info;
    init_defaults(vi_create_info);

    struct Vertex
    {
        vec3 pos;
        vec3 nrm;
    };

    VkVertexInputBindingDescription binding_description;
    binding_description.binding = 0;
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_description.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription vi_attribute_desc[2] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vec3)}
    };

    vi_create_info.pVertexAttributeDescriptions = vi_attribute_desc;
    vi_create_info.vertexAttributeDescriptionCount = 2;
    vi_create_info.pVertexBindingDescriptions = &binding_description;
    vi_create_info.vertexBindingDescriptionCount = 1;

    pipeline_create_info.pVertexInputState = &vi_create_info;
    // input assembly
    VkPipelineInputAssemblyStateCreateInfo ia_create_info;
    init_defaults(ia_create_info);
    pipeline_create_info.pInputAssemblyState = &ia_create_info;
    // rasterization
    VkPipelineRasterizationStateCreateInfo rs_create_info;
    init_defaults(rs_create_info);
    pipeline_create_info.pRasterizationState = &rs_create_info;
    // depth-stencil state
    VkPipelineDepthStencilStateCreateInfo ds_create_info;
    init_defaults(ds_create_info);
    pipeline_create_info.pDepthStencilState = &ds_create_info;
    // blend
    VkPipelineColorBlendStateCreateInfo blend_create_info;
    init_defaults(blend_create_info);
    VkPipelineColorBlendAttachmentState attachment_state;
    init_defaults(attachment_state);
    blend_create_info.attachmentCount = 1;
    blend_create_info.pAttachments = &attachment_state;
    pipeline_create_info.pColorBlendState = &blend_create_info;
    // multisampling
    VkPipelineMultisampleStateCreateInfo ms_create_info;
    init_defaults(ms_create_info);
    pipeline_create_info.pMultisampleState = &ms_create_info;
    // viewport
    VkPipelineViewportStateCreateInfo viewport_state_create_info;
    init_defaults(viewport_state_create_info);
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    // dynamic states
    // TODO: need to find out what those parameters mean
    VkPipelineDynamicStateCreateInfo dyn_state_create_info;
    init_defaults(dyn_state_create_info);
    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dyn_state_create_info.dynamicStateCount = 2;
    dyn_state_create_info.pDynamicStates = dynamic_states;
    pipeline_create_info.pDynamicState = &dyn_state_create_info;
    // layout
    VkPipelineLayoutCreateInfo layout_create_info;
    init_defaults(layout_create_info);
    VkDescriptorSetLayoutBinding layout_binding[3] = {
        // binding, descriptorType,                    descriptorCount, VkShaderStageFlags,       samples
        {0,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_VERTEX_BIT,   nullptr},  // camera uniform
        {1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_VERTEX_BIT,   nullptr},  // model uniform
        {2,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // material uniform
    };
    VkDescriptorSetLayout desciptor_set_layout;
    VkDescriptorSetLayoutCreateInfo desciptor_set_layout_create_info;
    init_defaults(desciptor_set_layout_create_info);
    //desciptor_set_layout_create_info.bindingCount = 3;
    desciptor_set_layout_create_info.bindingCount = 0;
    //desciptor_set_layout_create_info.pBindings = layout_binding;
    desciptor_set_layout_create_info.pBindings = nullptr;
    res = vkCreateDescriptorSetLayout(context.device, &desciptor_set_layout_create_info, context.allocation_callbacks, &desciptor_set_layout);
    ASSERT(res == VK_SUCCESS, "vkCreateDescriptorSetLayout failed");
    layout_create_info.pSetLayouts = &desciptor_set_layout;
    layout_create_info.setLayoutCount = 1;
    res = vkCreatePipelineLayout(context.device, &layout_create_info, context.allocation_callbacks, &pipeline_create_info.layout);
    ASSERT(res == VK_SUCCESS, "vkCreatePipelineLayout failed");
    // shaders
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
    init_defaults(shader_stage_create_info);
    // load the shaders
    const char* vert_shader_name = "../../shaders/00_cube.vertspv";
    const char* frag_shader_name = "../../shaders/00_cube.fragspv";
    VkShaderModule vsm;
    res = load_shader_module(vsm, context, vert_shader_name);
    ASSERT(res == VK_SUCCESS, "vertex shader loading failed");
    VkShaderModule fsm;
    res = load_shader_module(fsm, context, frag_shader_name);
    ASSERT(res == VK_SUCCESS, "fragment shader loading failed");

    VkPipelineShaderStageCreateInfo shader_stages[2];
    init_defaults(shader_stages[0]);
    init_defaults(shader_stages[1]);
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vsm;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = fsm;

    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.stageCount = 2;

    pipeline_create_info.renderPass = context.main_render_pass;

    res = vkCreateGraphicsPipelines(context.device, context.pipeline_cache, 1, &pipeline_create_info, context.allocation_callbacks, &pipeline);
    ASSERT(res == VK_SUCCESS, "vkCreateGraphicsPipelines failed");

    destroy_shader_module(vsm, context);
    destroy_shader_module(fsm, context);

    // create actual data
    Buffer vbuffer;
    Vertex vertices[3] = {
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
    res = create_static_buffer(vbuffer, context, reinterpret_cast<const uint8_t*>(vertices), sizeof(Vertex) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    ASSERT(res == VK_SUCCESS, "create_static_buffer failed");

    Buffer ibuffer;
    uint16_t indices[3] = {0, 2, 1};
    res = create_static_buffer(ibuffer, context, reinterpret_cast<const uint8_t*>(indices), sizeof(uint16_t) * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    ASSERT(res == VK_SUCCESS, "create_static_buffer failed");

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
        VkClearColorValue clear_color_value;
        clear_color_value.float32[0] = 0.0f;
        clear_color_value.float32[1] = 0.0f;
        clear_color_value.float32[2] = 0.0f;
        clear_color_value.float32[3] = 1.0f;
        clear_values[0].color = clear_color_value;
        VkClearDepthStencilValue clear_depth_stencil_value;
        clear_depth_stencil_value.depth = 1.0f;
        clear_depth_stencil_value.stencil = 0;
        clear_values[1].depthStencil = clear_depth_stencil_value;

        VkRect2D rect;
        rect.offset.x = rect.offset.y = 0;
        rect.extent.width = rect.extent.height = 512;
        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.clearValueCount = 2;
        render_pass_begin_info.framebuffer = context.main_framebuffer;
        render_pass_begin_info.pClearValues = clear_values;
        render_pass_begin_info.pNext = nullptr;
        render_pass_begin_info.renderArea = rect;
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

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offsets = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vbuffer.buffer, &offsets);
        vkCmdBindIndexBuffer(command_buffer, ibuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(command_buffer, 3, 1, 0, 0, 1);

        vkCmdEndRenderPass(command_buffer);
    }

    res = vkEndCommandBuffer(command_buffer);
    ASSERT(res == VK_SUCCESS, "vkEndCommandBuffer failed");

    uint32_t current_buffer = 0;

    while (app_message_loop(context))
    {
        res = vkAcquireNextImageKHR(context.device, context.swapchain, std::numeric_limits<uint64_t>::max(), context.present_semaphore, 0, &current_buffer);
        ASSERT(res == VK_SUCCESS, "vkAcquireNextImageKHR failed");

        VkSubmitInfo submit_info;
        init_defaults(submit_info);
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.pWaitSemaphores = &context.present_semaphore;
        submit_info.waitSemaphoreCount = 1;

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
    }

    destroy_static_buffer(ibuffer, context);
    destroy_static_buffer(vbuffer, context);

    vkDestroyPipeline(context.device, pipeline, context.allocation_callbacks);

    vkDestroyPipelineLayout(context.device, pipeline_create_info.layout, context.allocation_callbacks);
    vkDestroyDescriptorSetLayout(context.device, desciptor_set_layout, context.allocation_callbacks);

    vkFreeCommandBuffers(context.device, context.main_command_pool, 1, &command_buffer);

    destroy_vulkan_context(context);

    return 0;
}
