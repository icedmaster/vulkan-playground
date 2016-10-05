#include "mhevk.hpp"

#include <limits>

#define CUBE

using namespace mhe;

struct PerModelUniformData
{
    mat4x4 world;
};

struct PerCameraUniformData
{
    mat4x4 vp;
};

struct MaterialUniformData
{
    vec4 diffuse;
};

struct LightUniformData
{
    vec4 diffuse;
    vec4 position;
    vec4 direction;
};
/*
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
        vec2 tex;
    };

    VkVertexInputBindingDescription binding_description;
    binding_description.binding = 0;
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_description.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription vi_attribute_desc[3] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vec3)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, 2 * sizeof(vec3)}
    };

    vi_create_info.pVertexAttributeDescriptions = vi_attribute_desc;
    vi_create_info.vertexAttributeDescriptionCount = 3;
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
    // those states are used with a pipeline but the pipeline doesn't keep them
    VkPipelineDynamicStateCreateInfo dyn_state_create_info;
    init_defaults(dyn_state_create_info);
    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dyn_state_create_info.dynamicStateCount = 2;
    dyn_state_create_info.pDynamicStates = dynamic_states;
    pipeline_create_info.pDynamicState = &dyn_state_create_info;
    // layout
    VkPipelineLayoutCreateInfo layout_create_info;
    init_defaults(layout_create_info);
    VkDescriptorSetLayoutBinding layout_binding[5] = {
        // binding, descriptorType,                    descriptorCount, VkShaderStageFlags,       samples
        {0,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_VERTEX_BIT,   nullptr},  // camera uniform
        {1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_VERTEX_BIT,   nullptr},  // model uniform
        {2,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},  // material uniform
        {3,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // light uniform
        {4,         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,       VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}   // texture 
    };
    VkDescriptorSetLayout desciptor_set_layout;
    VkDescriptorSetLayoutCreateInfo desciptor_set_layout_create_info;
    init_defaults(desciptor_set_layout_create_info);
    desciptor_set_layout_create_info.bindingCount = 5;
    desciptor_set_layout_create_info.pBindings = layout_binding;
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
    const char* vert_shader_name = "../../shaders/00_cube.vert.spv";
    const char* frag_shader_name = "../../shaders/00_cube.frag.spv";
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
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}};

    Vertex cube_vertices[24] = {
        // front
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f, +0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{+0.5f, +0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{+0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        // back
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        // left
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, +0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-0.5f, +0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        // right
        {{+0.5f, -0.5f, -0.5f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{+0.5f, +0.5f, -0.5f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{+0.5f, +0.5f, +0.5f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{+0.5f, -0.5f, +0.5f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        // top
        {{-0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{+0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{+0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        // bottom
        {{-0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{+0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{+0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}
    };
#ifndef CUBE
    res = create_static_buffer(vbuffer, context, reinterpret_cast<const uint8_t*>(vertices), sizeof(Vertex) * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
#else
    res = create_static_buffer(vbuffer, context, reinterpret_cast<const uint8_t*>(cube_vertices), sizeof(Vertex) * 24, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
#endif
    ASSERT(res == VK_SUCCESS, "create_static_buffer failed");

    Buffer ibuffer;
    uint16_t indices[3] = {0, 2, 1};
    uint16_t cube_indices[36] = {0, 2, 1, 0, 3, 2, 7, 4, 6, 4, 5, 6, 8, 10, 9, 11, 10, 8, 12, 13, 14, 12, 14, 15, 16, 18, 17, 16, 19, 18, 20, 21, 22, 20, 22, 23};
#ifndef CUBE
    res = create_static_buffer(ibuffer, context, reinterpret_cast<const uint8_t*>(indices), sizeof(uint16_t) * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
#else
    res = create_static_buffer(ibuffer, context, reinterpret_cast<const uint8_t*>(cube_indices), sizeof(uint16_t) * 36, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
#endif
    ASSERT(res == VK_SUCCESS, "create_static_buffer failed");

    // uniforms
    UniformBuffer model_uniform_buffer;
    PerModelUniformData per_model_uniform_data;
    per_model_uniform_data.world = mat4x4::scaling(4.0f);
    res = model_uniform_buffer.init(context,
        reinterpret_cast<const uint8_t*>(&per_model_uniform_data), sizeof(PerModelUniformData));
    ASSERT(res == VK_SUCCESS, "model_uniform_buffer initialization failed");

    UniformBuffer camera_uniform_buffer;
    PerCameraUniformData per_camera_uniform_data;
    per_camera_uniform_data.vp = mat4x4::look_at(vec3(-2.0f, 4.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3::up()) * 
        mat4x4::perspective(deg_to_rad(60.0f), 1.0f, 0.1f, 20.0f);
    res = camera_uniform_buffer.init(context,
        reinterpret_cast<const uint8_t*>(&per_camera_uniform_data), sizeof(PerCameraUniformData));
    ASSERT(res == VK_SUCCESS, "camera_uniform_buffer initialization failed");

    UniformBuffer material_uniform_buffer;
    MaterialUniformData material_uniform_data;
    material_uniform_data.diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    res = material_uniform_buffer.init(context,
        reinterpret_cast<const uint8_t*>(&material_uniform_data), sizeof(MaterialUniformData));
    ASSERT(res == VK_SUCCESS, "material_uniform_buffer initialization failed");

    UniformBuffer light_uniform_buffer;
    LightUniformData light_uniform_data;
    light_uniform_data.diffuse = vec4(0.8f, 1.0f, 0.8f, 1.0f);
    light_uniform_data.position = vec4::zero();
#ifndef CUBE
    light_uniform_data.direction = vec4(0.0f, 0.0f, 1.0f, 0.0f);
#else
    light_uniform_data.direction = vec4(-0.707f, 0.707f, 0.0f, 0.0f);
#endif
    res = light_uniform_buffer.init(context,
        reinterpret_cast<const uint8_t*>(&light_uniform_data), sizeof(LightUniformData));
    ASSERT(res == VK_SUCCESS, "light_uniform_buffer initialization failed");

    // load an image
    Image image;
    res = load_image(image, "../../assets/checker.tga");
    ASSERT(res == VK_SUCCESS, "image loading failed");
    // create a texture
    Texture texture;
    TextureDesc texture_desc;
    texture_desc.width = image.width;
    texture_desc.height = image.height;
    texture_desc.format = image.format;
    res = texture.init(context, texture_desc, &image.data[0], image.data.size());
    ASSERT(res == VK_SUCCESS, "texture initialization failed");

    // create descriptor sets
    VkDescriptorPool descriptor_pool;
    VkDescriptorPoolCreateInfo descriptor_pool_create_info;
    init_defaults(descriptor_pool_create_info);

    VkDescriptorPoolSize descriptor_pool_size[2] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};

    descriptor_pool_create_info.maxSets = 3;
    descriptor_pool_create_info.poolSizeCount = 2;
    descriptor_pool_create_info.pPoolSizes = descriptor_pool_size;
    res = vkCreateDescriptorPool(context.device, &descriptor_pool_create_info, context.allocation_callbacks, &descriptor_pool);
    ASSERT(res == VK_SUCCESS, "vkCreateDescriptorPool failed");

    VkDescriptorSet descriptor_set[1];

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
    init_defaults(descriptor_set_allocate_info);
    descriptor_set_allocate_info.descriptorPool = descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts = &desciptor_set_layout;
    res = vkAllocateDescriptorSets(context.device, &descriptor_set_allocate_info, descriptor_set);
    ASSERT(res == VK_SUCCESS, "vkAllocateDescriptorSets failed");

    // and update our descriptor set
    VkDescriptorBufferInfo descriptor_buffer_info[4] = {
        camera_uniform_buffer.descriptor_buffer_info(),
        model_uniform_buffer.descriptor_buffer_info(),
        material_uniform_buffer.descriptor_buffer_info(),
        light_uniform_buffer.descriptor_buffer_info()};

    VkWriteDescriptorSet write_descriptor_set;
    init_defaults(write_descriptor_set);
    write_descriptor_set.descriptorCount = 4;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstSet = descriptor_set[0];
    write_descriptor_set.pBufferInfo = descriptor_buffer_info;
    vkUpdateDescriptorSets(context.device, 1, &write_descriptor_set, 0, nullptr);

    VkWriteDescriptorSet image_write_descriptor_set;
    init_defaults(image_write_descriptor_set);
    image_write_descriptor_set.descriptorCount = 1;
    image_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_write_descriptor_set.dstBinding = 4;
    image_write_descriptor_set.dstSet = descriptor_set[0];
    image_write_descriptor_set.pImageInfo = &texture.descriptor_image_info();
    vkUpdateDescriptorSets(context.device, 1, &image_write_descriptor_set, 0, nullptr);

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

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_create_info.layout, 0, 1, descriptor_set, 0, nullptr);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offsets = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vbuffer.buffer, &offsets);
        vkCmdBindIndexBuffer(command_buffer, ibuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
#ifndef CUBE
        vkCmdDrawIndexed(command_buffer, 3, 1, 0, 0, 1);
#else
        vkCmdDrawIndexed(command_buffer, 36, 1, 0, 0, 1);
#endif

        vkCmdEndRenderPass(command_buffer);
    }

    res = vkEndCommandBuffer(command_buffer);
    ASSERT(res == VK_SUCCESS, "vkEndCommandBuffer failed");

    uint32_t current_buffer = 0;
    float angle = 0.0f;
    float rotation_speed = 0.0001f;

    while (app_message_loop(context))
    {
        // update uniforms
        angle += rotation_speed;
        per_model_uniform_data.world = mat4x4::scaling(4.0f) * mat4x4::rotation_around_y(angle);
        model_uniform_buffer.update(context, reinterpret_cast<const uint8_t*>(&per_model_uniform_data), sizeof(PerModelUniformData));

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

    texture.destroy(context);

    vkFreeDescriptorSets(context.device, descriptor_pool, 1, descriptor_set);
    vkDestroyDescriptorPool(context.device, descriptor_pool, context.allocation_callbacks);

    light_uniform_buffer.destroy(context);
    model_uniform_buffer.destroy(context);
    camera_uniform_buffer.destroy(context);
    material_uniform_buffer.destroy(context);

    destroy_buffer(ibuffer, context);
    destroy_buffer(vbuffer, context);

    vkDestroyPipeline(context.device, pipeline, context.allocation_callbacks);

    vkDestroyPipelineLayout(context.device, pipeline_create_info.layout, context.allocation_callbacks);
    vkDestroyDescriptorSetLayout(context.device, desciptor_set_layout, context.allocation_callbacks);

    vkFreeCommandBuffers(context.device, context.main_command_pool, 1, &command_buffer);

    destroy_vulkan_context(context);

    return 0;
}*/

using namespace mhe;

struct Scene
{
    std::vector<vk::Mesh> meshes;
};

class MeshRenderer
{
public:
    VkResult init(vk::VulkanContext& context)
    {
        VkGraphicsPipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.renderPass = context.render_passes.main_render_pass;

        VkPipelineVertexInputStateCreateInfo vi_create_info;
        vk::GeometryLayout::vertex_input_info(vi_create_info);
        // input assembly
        VkPipelineInputAssemblyStateCreateInfo ia_create_info = {};
        ia_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // rasterization
        VkPipelineRasterizationStateCreateInfo rs_create_info = {};
        rs_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rs_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rs_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        // depth-stencil
        VkPipelineDepthStencilStateCreateInfo ds_create_info = {};
        ds_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds_create_info.stencilTestEnable = VK_FALSE;
        ds_create_info.depthTestEnable = VK_TRUE;
        ds_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
        ds_create_info.depthWriteEnable = VK_TRUE;
        ds_create_info.maxDepthBounds = 1.0f;
        // blend state
        VkPipelineColorBlendAttachmentState blend_attachment_state = {};
        blend_attachment_state.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_create_info.logicOpEnable = VK_FALSE;
        blend_create_info.attachmentCount = 1;
        blend_create_info.pAttachments = &blend_attachment_state;
        // multisampling
        VkPipelineMultisampleStateCreateInfo ms_create_info = {};
        ms_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms_create_info.sampleShadingEnable = VK_TRUE;
        ms_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        // viewport
        VkPipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        VkViewport viewport;
        vk::init_fullscreen_viewport(viewport, context);
        VkRect2D scissor;
        scissor.offset = { 0, 0 };
        scissor.extent = { context.width, context.height };
        viewport_create_info.pViewports = &viewport;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pScissors = &scissor;
        viewport_create_info.scissorCount = 1;

        // dynamic states
        VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_states_info = {};
        dynamic_states_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_states_info.pDynamicStates = dynamic_states;
        dynamic_states_info.dynamicStateCount = 2;

        // layout
        // TODO: create a cache of layouts
        VkDescriptorSetLayoutBinding per_camera_layout_binding[1] =
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}
        };

        VkDescriptorSetLayoutBinding material_layout_binding[2] =
        {
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
        };

        VkDescriptorSetLayoutBinding per_light_layout_binding[1] =
        {
            { 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        };

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pBindings = per_camera_layout_binding;
        descriptor_set_layout_create_info.bindingCount = array_size(per_camera_layout_binding);
        VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &per_camera_descriptor_set_layout_));

        descriptor_set_layout_create_info.pBindings = material_layout_binding;
        descriptor_set_layout_create_info.bindingCount = array_size(material_layout_binding);
        VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &material_descriptor_set_layout_));

        descriptor_set_layout_create_info.pBindings = per_light_layout_binding;
        descriptor_set_layout_create_info.bindingCount = array_size(per_light_layout_binding);
        VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &per_light_descriptor_set_layout_));

        VkDescriptorSetLayout descriptor_set_layouts[4] =
        {
            per_camera_descriptor_set_layout_, context.descriptor_set_layouts.mesh_layout,
            material_descriptor_set_layout_, per_light_descriptor_set_layout_
        };

        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.pSetLayouts = descriptor_set_layouts;
        layout_create_info.setLayoutCount = array_size(descriptor_set_layouts);
        VK_CHECK(vkCreatePipelineLayout(*context.main_device, &layout_create_info, context.allocation_callbacks, &pipeline_layout_));

        // shaders
        VkShaderModule vsm;
        VkShaderModuleCreateInfo shader_create_info = {};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        std::vector<uint8_t> shader_data;
        bool res = read_entire_file(shader_data, "../../shaders/00_cube.vert.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file cube_00.vert.spv");
        shader_create_info.pCode = reinterpret_cast<const uint32_t*>(&shader_data[0]);
        shader_create_info.codeSize = shader_data.size();
        VK_CHECK(vkCreateShaderModule(*context.main_device, &shader_create_info, context.allocation_callbacks, &vsm));

        VkShaderModule fsm;
        res = read_entire_file(shader_data, "../../shaders/00_cube.frag.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file cube_00.frag.spv");
        shader_create_info.pCode = reinterpret_cast<const uint32_t*>(&shader_data[0]);
        shader_create_info.codeSize = shader_data.size();
        VK_CHECK(vkCreateShaderModule(*context.main_device, &shader_create_info, context.allocation_callbacks, &fsm));

        VkPipelineShaderStageCreateInfo shader_stage_create_info[2] = { {},{} };
        shader_stage_create_info[0].sType = shader_stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stage_create_info[0].pName = "main";
        shader_stage_create_info[0].module = vsm;
        shader_stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stage_create_info[1].pName = "main";
        shader_stage_create_info[1].module = fsm;

        create_info.pVertexInputState = &vi_create_info;
        create_info.pInputAssemblyState = &ia_create_info;
        create_info.pRasterizationState = &rs_create_info;
        create_info.pDepthStencilState = &ds_create_info;
        create_info.pColorBlendState = &blend_create_info;
        create_info.pMultisampleState = &ms_create_info;
        create_info.pViewportState = &viewport_create_info;
        create_info.pDynamicState = &dynamic_states_info;
        create_info.pStages = shader_stage_create_info;
        create_info.stageCount = 2;
        create_info.layout = pipeline_layout_;

        VK_CHECK(vkCreateGraphicsPipelines(*context.main_device, context.main_pipeline_cache, 1, &create_info, context.allocation_callbacks, &pipeline_));

        vkDestroyShaderModule(*context.main_device, vsm, context.allocation_callbacks);
        vkDestroyShaderModule(*context.main_device, fsm, context.allocation_callbacks);

        create_uniforms(context);
        create_descriptor_sets(context);

        return VK_SUCCESS;
    }

    void destroy(vk::VulkanContext& context)
    {
        vkFreeDescriptorSets(*context.main_device, context.descriptor_pools.main_descriptor_pool, 1, &camera_descriptor_set_);
        vkFreeDescriptorSets(*context.main_device, context.descriptor_pools.main_descriptor_pool, 1, &light_descriptor_set_);

        vkDestroyDescriptorSetLayout(*context.main_device, per_camera_descriptor_set_layout_, context.allocation_callbacks);
        vkDestroyDescriptorSetLayout(*context.main_device, material_descriptor_set_layout_, context.allocation_callbacks);
        vkDestroyDescriptorSetLayout(*context.main_device, per_light_descriptor_set_layout_, context.allocation_callbacks);

        per_camera_uniform_.destroy(context);
        light_uniform_.destroy(context);

        vkDestroyPipeline(*context.main_device, pipeline_, context.allocation_callbacks);
        vkDestroyPipelineLayout(*context.main_device, pipeline_layout_, context.allocation_callbacks);
    }

    VkPipeline pipeline() const
    {
        return pipeline_;
    }

    void render(vk::CommandBuffer& command_buffer, vk::VulkanContext& context, const Scene& scene)
    {
        VkDescriptorSet descriptor_sets[2] =
        {
            camera_descriptor_set_, light_descriptor_set_
        };
        command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, descriptor_sets, array_size(descriptor_sets));
        for (const vk::Mesh& mesh : scene.meshes)
        {
            VkDescriptorSet mesh_descriptor_sets[1] =
            {
                mesh.descriptor_set()
            };
            command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, mesh_descriptor_sets, array_size(mesh_descriptor_sets));
            for (size_t i = 0, size = mesh.parts().size(); i < size; ++i)
            {
                command_buffer.draw(mesh, i);
            }
        }
    }
private:
    VkResult create_uniforms(vk::VulkanContext& context)
    {
        // uniforms
        PerModelUniformData per_model_uniform_data;
        per_model_uniform_data.world = mat4x4::scaling(4.0f);
        
        uint32_t graphics_queue_family_index = context.main_device->physical_device()->graphics_queue_family_index();

        vk::GPUInterface gpu_iface;
        gpu_iface.device = context.main_device;

        vk::Buffer::Settings settings;
        settings.queue_family_indices = &graphics_queue_family_index;
        settings.queue_family_indices_count = 1;
        settings.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        settings.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        PerCameraUniformData per_camera_uniform_data;
        per_camera_uniform_data.vp = mat4x4::look_at(vec3(-2.0f, 4.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3::up()) *
            mat4x4::perspective(deg_to_rad(60.0f), 1.0f, 0.1f, 20.0f);
        VK_CHECK(per_camera_uniform_.init(context, gpu_iface, settings,
            reinterpret_cast<const uint8_t*>(&per_camera_uniform_data), sizeof(PerCameraUniformData)));

        LightUniformData light_uniform_data;
        light_uniform_data.diffuse = vec4(0.8f, 1.0f, 0.8f, 1.0f);
        light_uniform_data.position = vec4::zero();
#ifndef CUBE
        light_uniform_data.direction = vec4(0.0f, 0.0f, 1.0f, 0.0f);
#else
        light_uniform_data.direction = vec4(-0.707f, 0.707f, 0.0f, 0.0f);
#endif
        VK_CHECK(light_uniform_.init(context, gpu_iface, settings,
            reinterpret_cast<const uint8_t*>(&light_uniform_data), sizeof(LightUniformData)));

        return VK_SUCCESS;
    }

    void create_descriptor_sets(vk::VulkanContext& context)
    {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = context.descriptor_pools.main_descriptor_pool;
        descriptor_set_allocate_info.pSetLayouts = &per_camera_descriptor_set_layout_;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        VK_CHECK(vkAllocateDescriptorSets(*context.main_device, &descriptor_set_allocate_info, &camera_descriptor_set_));

        descriptor_set_allocate_info.pSetLayouts = &per_light_descriptor_set_layout_;
        VK_CHECK(vkAllocateDescriptorSets(*context.main_device, &descriptor_set_allocate_info, &light_descriptor_set_));

        VkDescriptorBufferInfo camera_uniforms_info[1] =
        {
            per_camera_uniform_.descriptor_buffer_info()
        };

        VkWriteDescriptorSet write_desc_set = {};
        write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_set.dstSet = camera_descriptor_set_;
        write_desc_set.pBufferInfo = camera_uniforms_info;
        write_desc_set.descriptorCount = array_size(camera_uniforms_info);
        write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_desc_set.dstBinding = 0;
        vkUpdateDescriptorSets(*context.main_device, 1, &write_desc_set, 0, nullptr);

        VkDescriptorBufferInfo light_uniforms_info[1] =
        {
            light_uniform_.descriptor_buffer_info()
        };

        write_desc_set.dstSet = light_descriptor_set_;
        write_desc_set.pBufferInfo = light_uniforms_info;
        write_desc_set.descriptorCount = array_size(light_uniforms_info);
        vkUpdateDescriptorSets(*context.main_device, 1, &write_desc_set, 0, nullptr);
    }

    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;
    VkDescriptorSetLayout per_camera_descriptor_set_layout_;
    VkDescriptorSetLayout material_descriptor_set_layout_;
    VkDescriptorSetLayout per_light_descriptor_set_layout_;

    VkDescriptorSet camera_descriptor_set_;
    VkDescriptorSet light_descriptor_set_;

    vk::Buffer per_camera_uniform_;
    vk::Buffer light_uniform_;
};

struct Renderers
{
    MeshRenderer mesh_renderer;
};

void create_renderers(Renderers& renderers, vk::VulkanContext& context)
{
    renderers.mesh_renderer.init(context);
}

void destroy_renderers(Renderers& renderers, vk::VulkanContext& context)
{
    renderers.mesh_renderer.destroy(context);
}

int main(int argc, char** argv)
{
    vk::VulkanContext context;
    VkResult res = vk::init_vulkan_context(context, "vk_cube", 512, 512, false);
    VERIFY(res == VK_SUCCESS, "init_vulkan_context failed", -1);

    Renderers renderers;
    create_renderers(renderers, context);

    Scene scene;
    vk::Mesh mesh;
    mesh.create_cube(context, context.default_gpu_interface);
    scene.meshes.push_back(mesh);

    // load an image
    vk::ImageData image_data;
    VK_CHECK(vk::load_tga_image(image_data, "../../assets/checker.tga"));
    // create a texture
    vk::ImageView::Settings image_settings;
    image_settings.width = image_data.width;
    image_settings.height = image_data.height;
    image_settings.format = image_data.format;
    image_settings.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_settings.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    
    vk::Texture texture;
    VK_CHECK(texture.init(context, context.default_gpu_interface, image_settings, vk::Texture::SamplerSettings(),
        &image_data.data[0], image_data.data.size()));

    vk::CommandBuffer command_buffer;
    context.command_pools.main_graphics_command_pool.create_command_buffers(context, &command_buffer, 1);
    command_buffer
        .begin()
        .begin_render_pass_command(&context.framebuffers.main_framebuffer, vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0, true, true, true)
        .set_viewport_command({0, 0, 512, 512})
        .set_scissor_command({0, 0, 512, 512});
    renderers.mesh_renderer.render(command_buffer, context, scene);
    command_buffer
        .end_render_pass_command()
        .end();

    while (app_message_loop(context))
    {
        vk::Queue& graphics_queue = context.main_device->graphics_queue();
        context.main_swapchain.acquire_next_image();

        graphics_queue.submit(&command_buffer, 1, &graphics_queue.present_semaphore(), 1);
        graphics_queue.present(&context.main_swapchain);
        graphics_queue.wait_idle();
    }

    texture.destroy(context);

    destroy_renderers(renderers, context);

    context.command_pools.main_graphics_command_pool.destroy_command_buffers(context, &command_buffer, 1);

    vk::destroy_vulkan_context(context);
    return 0;
}
