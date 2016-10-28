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
    mat4x4 inv_vp;
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

#define RENDER_COLOR  0
#define RENDER_NORMAL 1

#define RENDER_TARGET RENDER_COLOR

using namespace mhe;

struct Scene
{
    std::vector<vk::Mesh> meshes;
};

class MeshRenderer
{
public:
    VkResult init(vk::VulkanContext& context, vk::RenderPass* render_pass)
    {
        VkGraphicsPipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.renderPass = *render_pass;

        VkPipelineVertexInputStateCreateInfo vi_create_info;
        vk::GeometryLayout::vertex_input_info(vi_create_info);
        // input assembly
        VkPipelineInputAssemblyStateCreateInfo ia_create_info = {};
        ia_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // rasterization
        VkPipelineRasterizationStateCreateInfo rs_create_info = {};
        rs_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs_create_info.cullMode = VK_CULL_MODE_NONE;
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
        VkPipelineColorBlendAttachmentState blend_attachment_state[2] = {{}, {}};
        blend_attachment_state[0].blendEnable = VK_FALSE;
        blend_attachment_state[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_attachment_state[0].colorBlendOp = VK_BLEND_OP_ADD;
        blend_attachment_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
        blend_attachment_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blend_attachment_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend_attachment_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend_attachment_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend_attachment_state[1] = blend_attachment_state[0];
        VkPipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_create_info.logicOpEnable = VK_FALSE;
        blend_create_info.attachmentCount = 2;
        blend_create_info.pAttachments = blend_attachment_state;
        // multisampling
        VkPipelineMultisampleStateCreateInfo ms_create_info = {};
        ms_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        ms_create_info.minSampleShading = 1.0f;
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

        VkDescriptorSetLayout descriptor_set_layouts[3] =
        {
            context.descriptor_set_layouts.camera_layout, context.descriptor_set_layouts.mesh_layout,
            context.descriptor_set_layouts.material_layout
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
        bool res = read_entire_file(shader_data, "../../shaders/01_fill.vert.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file 01_fill.vert.spv");
        shader_create_info.pCode = reinterpret_cast<const uint32_t*>(&shader_data[0]);
        shader_create_info.codeSize = shader_data.size();
        VK_CHECK(vkCreateShaderModule(*context.main_device, &shader_create_info, context.allocation_callbacks, &vsm));

        VkShaderModule fsm;
        res = read_entire_file(shader_data, "../../shaders/01_fill.frag.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file 01_fill.frag.spv");
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

        per_camera_uniform_.destroy(context);

        vkDestroyPipeline(*context.main_device, pipeline_, context.allocation_callbacks);
        vkDestroyPipelineLayout(*context.main_device, pipeline_layout_, context.allocation_callbacks);
    }

    VkPipeline pipeline() const
    {
        return pipeline_;
    }

    void render(vk::CommandBuffer& command_buffer, vk::VulkanContext& context, const Scene& scene)
    {
        command_buffer.bind_pipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, &camera_descriptor_set_, 1, 0);
        for (const vk::Mesh& mesh : scene.meshes)
        {
            VkDescriptorSet mesh_descriptor_sets[1] =
            {
                mesh.descriptor_set()
            };
            command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, mesh_descriptor_sets, array_size(mesh_descriptor_sets), 1);
            for (size_t i = 0, size = mesh.parts().size(); i < size; ++i)
            {
                VkDescriptorSet part_descriptor_sets[1] =
                {
                    mesh.parts()[i].material->descriptor_set()
                };
                command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, part_descriptor_sets, array_size(part_descriptor_sets), 2);
                command_buffer.draw(mesh, i);
            }
        }
    }
private:
    VkResult create_uniforms(vk::VulkanContext& context)
    {
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

        return VK_SUCCESS;
    }

    void create_descriptor_sets(vk::VulkanContext& context)
    {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = context.descriptor_pools.main_descriptor_pool;
        descriptor_set_allocate_info.pSetLayouts = &context.descriptor_set_layouts.camera_layout;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        VK_CHECK(vkAllocateDescriptorSets(*context.main_device, &descriptor_set_allocate_info, &camera_descriptor_set_));

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

        context.descriptor_sets.main_camera_descriptor_set = camera_descriptor_set_;
    }

    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;

    VkDescriptorSet camera_descriptor_set_;

    vk::Buffer per_camera_uniform_;
};

struct GBuffer
{
    vk::ImageView layer0;
    vk::ImageView layer1;
    vk::RenderPass render_pass;
    vk::Framebuffer framebuffer;
};

class GBufferRenderer
{
public:
    VkResult init(vk::VulkanContext& context, GBuffer* gbuffer)
    {
        gbuffer_ = gbuffer;

        VkGraphicsPipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.renderPass = context.render_passes.main_render_pass;

        VkPipelineVertexInputStateCreateInfo vi_create_info;
        vk::FullscreenLayout::vertex_input_info(vi_create_info);
        // input assembly
        VkPipelineInputAssemblyStateCreateInfo ia_create_info = {};
        ia_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // rasterization
        VkPipelineRasterizationStateCreateInfo rs_create_info = {};
        rs_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs_create_info.cullMode = VK_CULL_MODE_NONE;
        rs_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rs_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        // depth-stencil
        VkPipelineDepthStencilStateCreateInfo ds_create_info = {};
        ds_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds_create_info.stencilTestEnable = VK_FALSE;
        ds_create_info.depthTestEnable = VK_FALSE;
        ds_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
        ds_create_info.depthWriteEnable = VK_FALSE;
        ds_create_info.maxDepthBounds = 1.0f;
        // blend state
        VkPipelineColorBlendAttachmentState blend_attachment_state = {};
        blend_attachment_state.blendEnable = VK_FALSE;
        blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_create_info.logicOpEnable = VK_FALSE;
        blend_create_info.attachmentCount = 1;
        blend_create_info.pAttachments = &blend_attachment_state;
        // multisampling
        VkPipelineMultisampleStateCreateInfo ms_create_info = {};
        ms_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        ms_create_info.minSampleShading = 1.0f;
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
        VkDescriptorSetLayout descriptor_set_layouts[2] =
        {
            context.descriptor_set_layouts.camera_layout,
            context.descriptor_set_layouts.gbuffer_layout
        };

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = context.descriptor_pools.main_descriptor_pool;
        descriptor_set_allocate_info.pSetLayouts = &context.descriptor_set_layouts.gbuffer_layout;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        VK_CHECK(vkAllocateDescriptorSets(*context.main_device, &descriptor_set_allocate_info, &descriptor_set_));
        // update the descriptor set
        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = descriptor_set_;
        write_descriptor_set.dstBinding = 0;

        VkSamplerCreateInfo sampler_create_info = vk::SamplerCreateInfo(VK_FILTER_LINEAR, VK_FILTER_LINEAR,
            VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
        VK_CHECK(vkCreateSampler(*context.main_device, &sampler_create_info, context.allocation_callbacks, &sampler_));

        VkDescriptorImageInfo image_info[3];
        image_info[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_info[0].imageView = gbuffer->layer0.image_view_id();
        image_info[0].sampler = sampler_;
        image_info[1] = image_info[0];
        image_info[2] = image_info[0];
        image_info[1].imageView = gbuffer->layer1.image_view_id();
        image_info[2].imageView = context.main_depth_stencil_image_view.image_view_id();

        write_descriptor_set.pImageInfo = image_info;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = array_size(image_info);
        vkUpdateDescriptorSets(*context.main_device, 1, &write_descriptor_set, 0, nullptr);

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
        bool res = read_entire_file(shader_data, "../../shaders/01_deferred.vert.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file 01_deferred.vert.spv");
        shader_create_info.pCode = reinterpret_cast<const uint32_t*>(&shader_data[0]);
        shader_create_info.codeSize = shader_data.size();
        VK_CHECK(vkCreateShaderModule(*context.main_device, &shader_create_info, context.allocation_callbacks, &vsm));

        VkShaderModule fsm;
        res = read_entire_file(shader_data, "../../shaders/01_deferred.frag.spv", "rb");
        ASSERT(res == true, "Can't read shader data from file 01_deferred.frag.spv");
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

        VK_CHECK(quad_.create_quad(context, context.default_gpu_interface));

        return VK_SUCCESS;
    }

    void destroy(vk::VulkanContext& context)
    {
        quad_.destroy(context);
        vkDestroySampler(*context.main_device, sampler_, context.allocation_callbacks);
        vkFreeDescriptorSets(*context.main_device, context.descriptor_pools.main_descriptor_pool, 1, &descriptor_set_);
        vkDestroyPipelineLayout(*context.main_device, pipeline_layout_, context.allocation_callbacks);
        vkDestroyPipeline(*context.main_device, pipeline_, context.allocation_callbacks);
    }

    void render(vk::CommandBuffer& command_buffer, vk::VulkanContext& context)
    {
        command_buffer.bind_pipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, &context.descriptor_sets.main_camera_descriptor_set, 1, 0);
        command_buffer.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, &descriptor_set_, 1, 1);
        command_buffer.draw(quad_, 0);
    }
private:
    VkPipeline pipeline_;
    VkPipelineLayout pipeline_layout_;
    VkDescriptorSet descriptor_set_;
    VkSampler sampler_;
    vk::Mesh quad_;
    GBuffer* gbuffer_;
};

struct Renderers
{
    MeshRenderer mesh_renderer;
    GBufferRenderer gbuffer_renderer;
};

void create_renderers(Renderers& renderers, vk::VulkanContext& context, GBuffer& gbuffer)
{
    renderers.mesh_renderer.init(context, &gbuffer.render_pass);
    renderers.gbuffer_renderer.init(context, &gbuffer);
}

void destroy_renderers(Renderers& renderers, vk::VulkanContext& context)
{
    renderers.gbuffer_renderer.destroy(context);
    renderers.mesh_renderer.destroy(context);
}

VkResult create_gbuffer(GBuffer& gbuffer, vk::VulkanContext& context)
{
    vk::ImageView::Settings settings;
    settings.width = context.width;
    settings.height = context.height;
    settings.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    settings.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    settings.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VK_CHECK(gbuffer.layer0.init(context, context.default_gpu_interface, settings, VK_NULL_HANDLE, nullptr, 0));
    VK_CHECK(gbuffer.layer1.init(context, context.default_gpu_interface, settings, VK_NULL_HANDLE, nullptr, 0));

    // create a render pass
    vk::RenderPass::Settings::AttachmentDesc attachment_desc[3];
    attachment_desc[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachment_desc[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_desc[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachment_desc[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_desc[2].format = context.main_depth_stencil_image_view.format();
    attachment_desc[2].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    vk::RenderPass::Settings::DependencyDesc dependencies[1];
    dependencies[0].render_pass = &context.render_passes.main_render_pass;
    dependencies[0].src_access = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dst_access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vk::RenderPass::Settings render_pass_settings;
    render_pass_settings.count = 3;
    render_pass_settings.descs = attachment_desc;
    render_pass_settings.dependencies = dependencies;
    render_pass_settings.dependencies_count = array_size(dependencies);
    VK_CHECK(gbuffer.render_pass.init(context, context.default_gpu_interface, render_pass_settings));

    // create a framebuffer
    vk::Framebuffer::Settings framebuffer_settings;
    const vk::ImageView* image_views[] = { &gbuffer.layer0, &gbuffer.layer1, &context.main_depth_stencil_image_view };
    framebuffer_settings.attachments = image_views;
    framebuffer_settings.count = 3;
    framebuffer_settings.render_pass = &gbuffer.render_pass;
    framebuffer_settings.width = context.width;
    framebuffer_settings.height = context.height;
    VK_CHECK(gbuffer.framebuffer.init(context, context.default_gpu_interface, framebuffer_settings));
    return VK_SUCCESS;
}

void destroy_gbuffer(GBuffer& gbuffer, vk::VulkanContext& context)
{
    gbuffer.framebuffer.destroy(context);
    gbuffer.render_pass.destroy(context);
    gbuffer.layer0.destroy(context);
    gbuffer.layer1.destroy(context);
}

int main(int argc, char** argv)
{
    vk::VulkanContext context;
    VkResult res = vk::init_vulkan_context(context, "vk_deferred", 1280, 720, true);
    VERIFY(res == VK_SUCCESS, "init_vulkan_context failed", -1);

    GBuffer gbuffer;
    VK_CHECK(create_gbuffer(gbuffer, context));

    Renderers renderers;
    create_renderers(renderers, context, gbuffer);

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
    
    vk::Texture texture;
    VK_CHECK(texture.init(context, context.default_gpu_interface, image_settings, vk::Texture::SamplerSettings(),
        &image_data.data[0], image_data.data.size()));

    vk::Material material;
    VK_CHECK(material.init(context, context.default_gpu_interface));
    material.set_albedo(&texture);
    scene.meshes[0].set_material(0, &material);

    vk::CommandBuffer command_buffers[2];
    context.command_pools.main_graphics_command_pool.create_command_buffers(context, command_buffers, 2);

    command_buffers[0]
        .begin()
            .transfer_image_layout(context.main_swapchain.color_images()[0].image(),
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT)
            .transfer_image_layout(context.main_swapchain.color_images()[1].image(),
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT)
            .transfer_image_layout(context.main_depth_stencil_image_view.image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
            .transfer_image_layout(gbuffer.layer0.image(),
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT)
            .transfer_image_layout(gbuffer.layer1.image(),
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT)
        .end();
    context.main_device->graphics_queue().submit(command_buffers, 1);
    context.main_device->graphics_queue().wait_idle();

    const VkSemaphoreCreateInfo& semaphore_create_info = vk::SemaphoreCreateInfo();
    VkSemaphore semaphore;
    VK_CHECK(vkCreateSemaphore(*context.main_device, &semaphore_create_info, context.allocation_callbacks, &semaphore));

    while (app_message_loop(context))
    {
        vk::Queue& graphics_queue = context.main_device->graphics_queue();
        context.main_swapchain.acquire_next_image();

        VkSemaphore submit_wait_semaphore = context.main_swapchain.next_image_semaphore();

        command_buffers[0]
            .begin()
            .begin_render_pass_command(&gbuffer.framebuffer, vec4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0, 2, true, true)
            .set_viewport_command({ 0, 0, context.width, context.height })
            .set_scissor_command({ 0, 0, context.width, context.height });
        renderers.mesh_renderer.render(command_buffers[0], context, scene);
        command_buffers[0]
            .end_render_pass_command()
            .end();

        graphics_queue.submit(command_buffers, 1, &submit_wait_semaphore, 1, &semaphore, 1);

        command_buffers[1]
            .begin()
            .begin_render_pass_command(&context.main_swapchain.current_framebuffer(), vec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f, 0, 1, true, true)
            .set_viewport_command({ 0, 0, context.width, context.height })
            .set_scissor_command({ 0, 0, context.width, context.height });
        renderers.gbuffer_renderer.render(command_buffers[1], context);
        command_buffers[1]
            .end_render_pass_command()
            .end();

        VkSemaphore present_semaphore = graphics_queue.present_semaphore();
        graphics_queue.submit(command_buffers + 1, 1, &semaphore, 1, &present_semaphore, 1);
        graphics_queue.present(&context.main_swapchain);
        graphics_queue.wait_idle();
    }

    vkDestroySemaphore(*context.main_device, semaphore, context.allocation_callbacks);

    mesh.destroy(context);
    material.destroy(context);
    texture.destroy(context);

    destroy_gbuffer(gbuffer, context);

    destroy_renderers(renderers, context);

    context.command_pools.main_graphics_command_pool.destroy_command_buffers(context, command_buffers, 2);

    vk::destroy_vulkan_context(context);
    return 0;
}
