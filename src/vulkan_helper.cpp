#include "vulkan_helper.hpp"

#include <limits>

namespace mhe {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                                              uint64_t srcObject, size_t location, int32_t msgCode,
                                              const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
    printf("%s %s", pLayerPrefix, pMsg);
    return VK_TRUE;
}

#ifdef _WIN32
LRESULT CALLBACK wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        break;
    case WM_SIZE:
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// copied from vulkan sample code
HWND create_window(const char* name, uint32_t width, uint32_t height, HMODULE module_handle)
{
    WNDCLASSEX win_class;

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = wndproc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = module_handle;
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = nullptr;
    win_class.lpszClassName = name;
    win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    if (!RegisterClassEx(&win_class))
    {
        assert(0);
    }
    // Create window with the registered class:
    RECT wr = { 0, 0, (LONG)width, (LONG)height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND handle = CreateWindowEx(0,
        name,           // class name
        name,           // app name
        WS_OVERLAPPEDWINDOW | // window style
        WS_VISIBLE | WS_SYSMENU,
        0, 0,           // x/y coords
        wr.right - wr.left, // width
        wr.bottom - wr.top, // height
        NULL,               // handle to parent
        NULL,               // handle to menu
        0,                  // hInstance
        NULL);              // no extra parameters
    return handle;
}

bool wndprocess(HWND hwnd)
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != FALSE)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            return false;
    }
    return true;
}
#endif

uint32_t get_memory_type_index(const VkMemoryRequirements& requirements, VkFlags properties, const VkPhysicalDeviceMemoryProperties& memory_properties)
{
    for (uint32_t i = 0; i < 32; ++i)
    {
        if (requirements.memoryTypeBits & (1 << i))
        {
            if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }
    }
    return 0;
}

void init_defaults(VkApplicationInfo& appinfo)
{
    appinfo.apiVersion = VK_API_VERSION;
    appinfo.applicationVersion = 0;
    appinfo.engineVersion = 0;
    appinfo.pApplicationName = "";
    appinfo.pEngineName = "";
    appinfo.pNext = nullptr;
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
}

void init_defaults(VkInstanceCreateInfo& create_info)
{
    create_info.enabledExtensionCount = 0;
    create_info.enabledLayerCount = 0;
    create_info.flags = 0;
    create_info.pApplicationInfo = nullptr;
    create_info.pNext = nullptr;
    create_info.ppEnabledExtensionNames = nullptr;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
}

void init_defaults(VkDeviceQueueCreateInfo& device_queue_create_info)
{
    device_queue_create_info.flags = 0;
    device_queue_create_info.pNext = nullptr;
    device_queue_create_info.pQueuePriorities = nullptr;
    device_queue_create_info.queueCount = 0;
    device_queue_create_info.queueFamilyIndex = 0;
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
}

void init_defaults(VkDeviceCreateInfo& device_create_info)
{
    device_create_info.enabledExtensionCount = 0;
    device_create_info.enabledLayerCount = 0;
    device_create_info.flags = 0;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.pNext = nullptr;
    device_create_info.ppEnabledExtensionNames = nullptr;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.pQueueCreateInfos = nullptr;
    device_create_info.queueCreateInfoCount = 0;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
}

void init_defaults(VkImageCreateInfo& image_create_info)
{
    VkExtent3D extent;
    extent.width = extent.height = extent.depth = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.extent = extent;
    image_create_info.flags = 0;
    image_create_info.format = VK_FORMAT_END_RANGE;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.mipLevels = 1;
    image_create_info.pNext = nullptr;
    // check out the next two parameters meaning
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
}

void init_defaults(VkImageViewCreateInfo& image_view_create_info)
{
    VkComponentMapping component_mapping;
    component_mapping.r = component_mapping.g = component_mapping.b = component_mapping.a = 
        VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components = component_mapping;
    image_view_create_info.flags = 0;
    image_view_create_info.format = VK_FORMAT_END_RANGE;
    image_view_create_info.image = 0;
    image_view_create_info.pNext = nullptr;
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
}

void init_defaults(VkSwapchainCreateInfoKHR& swapchain_create_info)
{
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.flags = 0;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchain_create_info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
}

void init_defaults(VkAttachmentDescription& attachment_description)
{
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_description.flags = 0;
    attachment_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
}

void init_defaults(VkSubpassDescription& subpass_desc)
{
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.flags = 0;
    subpass_desc.inputAttachmentCount = 0;
    subpass_desc.pColorAttachments = nullptr;
    subpass_desc.pDepthStencilAttachment = nullptr;
    subpass_desc.pInputAttachments = nullptr;
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.pPreserveAttachments = nullptr;
    subpass_desc.preserveAttachmentCount = 0;
    subpass_desc.pResolveAttachments = nullptr;
}

void init_defaults(VkRenderPassCreateInfo& render_pass_create_info)
{
    render_pass_create_info.attachmentCount = 0;
    render_pass_create_info.dependencyCount = 0;
    render_pass_create_info.flags = 0;
    render_pass_create_info.pAttachments = nullptr;
    render_pass_create_info.pDependencies = nullptr;
    render_pass_create_info.pNext = nullptr;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.subpassCount = 0;
    render_pass_create_info.pSubpasses = nullptr;
}

void init_defaults(VkFramebufferCreateInfo& framebuffer_create_info)
{
    framebuffer_create_info.attachmentCount = 0;
    framebuffer_create_info.flags = 0;
    framebuffer_create_info.height = 0;
    framebuffer_create_info.layers = 0;
    framebuffer_create_info.pAttachments = nullptr;
    framebuffer_create_info.pNext = nullptr;
    framebuffer_create_info.renderPass = 0;
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.width = 0;
}

void init_defaults(VkCommandPoolCreateInfo& command_pool_create_info)
{
    command_pool_create_info.flags = 0;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.queueFamilyIndex = 0;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
}

void init_defaults(VkCommandBufferAllocateInfo& cb_allocate_info)
{
    cb_allocate_info.commandBufferCount = 0;
    cb_allocate_info.commandPool = 0;
    cb_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cb_allocate_info.pNext = nullptr;
    cb_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
}

void init_defaults(VkCommandBufferBeginInfo& begin_info)
{
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;
    begin_info.pNext = nullptr;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
}

void init_defaults(VkSubmitInfo& submit_info)
{
    submit_info.commandBufferCount = 0;
    submit_info.pCommandBuffers = nullptr;
    submit_info.pNext = nullptr;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 0;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 0;
}

void init_defaults(VkPresentInfoKHR& present_info)
{
    present_info.pNext = nullptr;
    present_info.pResults = nullptr;
    present_info.pSwapchains = nullptr;
    present_info.pWaitSemaphores = nullptr;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 0;
    present_info.waitSemaphoreCount = 0;
    present_info.pImageIndices = nullptr;
}

void init_defaults(VkSemaphoreCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
}

void init_defaults(VkGraphicsPipelineCreateInfo& create_info)
{
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = 0;
    create_info.flags = 0;
    create_info.layout = VK_NULL_HANDLE;
    create_info.pColorBlendState = nullptr;
    create_info.pDepthStencilState = nullptr;
    create_info.pDynamicState = nullptr;
    create_info.pInputAssemblyState = nullptr;
    create_info.pMultisampleState = nullptr;
    create_info.pNext = nullptr;
    create_info.pRasterizationState = nullptr;
    create_info.pStages = nullptr;
    create_info.pTessellationState = nullptr;
    create_info.pVertexInputState = nullptr;
    create_info.pViewportState = nullptr;
    create_info.renderPass = VK_NULL_HANDLE;
    create_info.stageCount = 0;
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.subpass = 0;
}

void init_defaults(VkPipelineVertexInputStateCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.pVertexAttributeDescriptions = nullptr;
    create_info.pVertexBindingDescriptions = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    create_info.vertexAttributeDescriptionCount = 0;
    create_info.vertexBindingDescriptionCount = 0;
}

void init_defaults(VkPipelineInputAssemblyStateCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.primitiveRestartEnable = VK_FALSE;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

void init_defaults(VkPipelineRasterizationStateCreateInfo& create_info)
{
    create_info.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
    create_info.depthBiasClamp = 0.0f;
    create_info.depthBiasConstantFactor = 0.0f;
    create_info.depthBiasEnable = VK_FALSE;
    create_info.depthBiasSlopeFactor = 0.0f;
    create_info.depthClampEnable = VK_FALSE;
    create_info.flags = 0;
    create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    create_info.lineWidth = 0.0f;
    create_info.pNext = nullptr;
    create_info.polygonMode = VK_POLYGON_MODE_FILL;
    create_info.rasterizerDiscardEnable = VK_FALSE;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
}

void init_defaults(VkPipelineDepthStencilStateCreateInfo& create_info)
{
    VkStencilOpState stencil_default_state;
    stencil_default_state.compareMask = 0xffffffff;
    stencil_default_state.compareOp = VK_COMPARE_OP_ALWAYS;
    stencil_default_state.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil_default_state.failOp = VK_STENCIL_OP_KEEP;
    stencil_default_state.passOp = VK_STENCIL_OP_KEEP;
    stencil_default_state.reference = 0;
    stencil_default_state.writeMask = 0xffffffff;
    create_info.back = stencil_default_state;
    create_info.depthBoundsTestEnable = VK_FALSE;
    create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    create_info.depthTestEnable = VK_TRUE;
    create_info.depthWriteEnable = VK_TRUE;
    create_info.flags = 0;
    create_info.front = stencil_default_state;
    create_info.maxDepthBounds = 1.0f;
    create_info.minDepthBounds = 0.0f;
    create_info.pNext = nullptr;
    create_info.stencilTestEnable = VK_FALSE;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
}

void init_defaults(VkPipelineColorBlendStateCreateInfo& create_info)
{
    create_info.attachmentCount = 0;
    for (int i = 0; i < 4; ++i) create_info.blendConstants[i] = 0.0f;
    create_info.flags = 0;
    create_info.logicOp = VK_LOGIC_OP_CLEAR;
    create_info.logicOpEnable = VK_FALSE;
    create_info.pAttachments = nullptr;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
}

void init_defaults(VkPipelineColorBlendAttachmentState& state)
{
    state.alphaBlendOp = VK_BLEND_OP_ADD;
    state.blendEnable = VK_FALSE;
    state.colorBlendOp = VK_BLEND_OP_ADD;
    state.colorWriteMask = vk_all_color_components;
    state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
}

void init_defaults(VkPipelineMultisampleStateCreateInfo& create_info)
{
    create_info.alphaToCoverageEnable = VK_FALSE;
    create_info.alphaToOneEnable = VK_FALSE;
    create_info.flags = 0;
    create_info.minSampleShading = 0.0f;
    create_info.pNext = nullptr;
    create_info.pSampleMask = nullptr;
    create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    create_info.sampleShadingEnable = VK_FALSE;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
}

void init_defaults(VkPipelineDynamicStateCreateInfo& create_info)
{
    create_info.dynamicStateCount = 0;
    create_info.flags = 0;
    create_info.pDynamicStates = nullptr;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
}

void init_defaults(VkPipelineLayoutCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.pPushConstantRanges = nullptr;
    create_info.pSetLayouts = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.setLayoutCount = 0;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
}

void init_defaults(VkDescriptorSetLayoutCreateInfo& create_info)
{
    create_info.bindingCount = 0;
    create_info.flags = 0;
    create_info.pBindings = nullptr;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
}

void init_defaults(VkShaderModuleCreateInfo& create_info)
{
    create_info.codeSize = 0;
    create_info.flags = 0;
    create_info.pCode = nullptr;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
}

void init_defaults(VkPipelineShaderStageCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.module = VK_NULL_HANDLE;
    create_info.pName = "main";
    create_info.pNext = nullptr;
    create_info.pSpecializationInfo = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
}

void init_defaults(VkPipelineCacheCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.initialDataSize = 0;
    create_info.pInitialData = nullptr;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
}

void init_defaults(VkPipelineViewportStateCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.pScissors = nullptr;
    create_info.pViewports = nullptr;
    create_info.scissorCount = 1;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info.viewportCount = 1;
}

void init_defaults(VkBufferCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.queueFamilyIndexCount = 0;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
}

void init_defaults(VkMemoryAllocateInfo& allocate_info)
{
    allocate_info.pNext = nullptr;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
}

void init_defaults(VkDescriptorSetAllocateInfo& allocate_info)
{
    allocate_info.pNext = nullptr;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
}

void init_defaults(VkDescriptorPoolCreateInfo& create_info)
{
    create_info.flags = 0;
    create_info.pNext = nullptr;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
}

void init_defaults(VkWriteDescriptorSet& write_descriptor_set)
{
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.pBufferInfo = nullptr;
    write_descriptor_set.pImageInfo = nullptr;
    write_descriptor_set.pNext = nullptr;
    write_descriptor_set.pTexelBufferView = nullptr;
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
}

VkResult create_depth_image_view(VkImageView& imageview, VkImage& image, uint32_t width, uint32_t height, const VulkanContext& vulkan_context)
{
    VkImageCreateInfo depth_image_create_info;
    init_defaults(depth_image_create_info);

    VkExtent3D extent;
    extent.width = width;
    extent.height = height;
    extent.depth = 1;
    depth_image_create_info.extent = extent;
    depth_image_create_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImage depth_image;
    VkResult res = vkCreateImage(vulkan_context.device, &depth_image_create_info, vulkan_context.allocation_callbacks, &depth_image);
    if (res != VK_SUCCESS) return res;

    VkMemoryRequirements depth_image_memory_requirements;
    vkGetImageMemoryRequirements(vulkan_context.device, depth_image, &depth_image_memory_requirements);

    // allocate memory for the depth buffer
    VkDeviceMemory depth_image_memory;
    VkMemoryAllocateInfo depth_image_allocate_info;
    depth_image_allocate_info.allocationSize = depth_image_memory_requirements.size;
    depth_image_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    depth_image_allocate_info.pNext = nullptr;
    depth_image_allocate_info.memoryTypeIndex = get_memory_type_index(depth_image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan_context.gpu_memory_properties);
    res = vkAllocateMemory(vulkan_context.device, &depth_image_allocate_info, vulkan_context.allocation_callbacks, &depth_image_memory);
    assert(res == VK_SUCCESS);

    res = vkBindImageMemory(vulkan_context.device, depth_image, depth_image_memory, 0);
    assert(res == VK_SUCCESS);

    VkImageViewCreateInfo depth_image_view_create_info;
    init_defaults(depth_image_view_create_info);
    depth_image_view_create_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_image_view_create_info.image = depth_image;
    depth_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    res = vkCreateImageView(vulkan_context.device, &depth_image_view_create_info, vulkan_context.allocation_callbacks, &imageview);
    assert(res == VK_SUCCESS);

    image = depth_image;
    return VK_SUCCESS;
}

void destroy_image_view(VkImageView& image_view, VkImage& vk_image, const VulkanContext& vulkan_context)
{
    vkDestroyImageView(vulkan_context.device, image_view, vulkan_context.allocation_callbacks);
    vkDestroyImage(vulkan_context.device, vk_image, vulkan_context.allocation_callbacks);
}

VkResult create_shader_module(VkShaderModule& shader, const VulkanContext& context, const uint8_t* text, uint32_t size)
{
    VkShaderModuleCreateInfo create_info;
    init_defaults(create_info);
    create_info.pCode = reinterpret_cast<const uint32_t*>(text);
    create_info.codeSize = size;
    return vkCreateShaderModule(context.device, &create_info, context.allocation_callbacks, &shader);
}

void destroy_shader_module(VkShaderModule& shader, const VulkanContext& context)
{
    vkDestroyShaderModule(context.device, shader, context.allocation_callbacks);
}

bool read_whole_file(std::vector<uint8_t>& data, const char* filename, const char* mode)
{
    FILE* f = fopen(filename, mode);
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    data.resize(size);
    size_t res = fread(&data[0], 1, size, f);
    fclose(f);
    return res == size;
}

VkResult load_shader_module(VkShaderModule& shader, const VulkanContext& context, const char* filename)
{
    std::vector<uint8_t> shader_data;
    if (!read_whole_file(shader_data, filename))
        return VK_ERROR_OUT_OF_DATE_KHR;
    return create_shader_module(shader, context, &shader_data[0], static_cast<uint32_t>(shader_data.size()));
}

VkResult create_static_buffer(Buffer& buffer, const VulkanContext& context, const uint8_t* data, uint32_t size, VkBufferUsageFlags usage)
{
    VkBuffer src_buffer;
    VkBufferCreateInfo buffer_create_info;
    init_defaults(buffer_create_info);
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.size = size;

    VkResult res = vkCreateBuffer(context.device, &buffer_create_info, context.allocation_callbacks, &src_buffer);
    VULKAN_VERIFY(res, "vkCreateBuffer failed");

    VkDeviceMemory src_device_memory;
    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(context.device, src_buffer, &buffer_memory_requirements);
    uint32_t memory_type = get_memory_type_index(buffer_memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, context.gpu_memory_properties);
    VkMemoryAllocateInfo memory_allocate_info;
    init_defaults(memory_allocate_info);
    memory_allocate_info.memoryTypeIndex = memory_type;
    memory_allocate_info.allocationSize = size;
    res = vkAllocateMemory(context.device, &memory_allocate_info, context.allocation_callbacks, &src_device_memory);
    VULKAN_VERIFY(res, "vkAllocateMemory failed");

    void* mapped_memory = nullptr;
    res = vkMapMemory(context.device, src_device_memory, 0, size, 0, &mapped_memory);
    VULKAN_VERIFY(res, "vkMapMemory failed");
    memcpy(mapped_memory, data, size);
    vkUnmapMemory(context.device, src_device_memory);

    res = vkBindBufferMemory(context.device, src_buffer, src_device_memory, 0);
    VULKAN_VERIFY(res, "vkBindBufferMemory failed");

    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
    res = vkCreateBuffer(context.device, &buffer_create_info, context.allocation_callbacks, &buffer.buffer);
    VULKAN_VERIFY(res, "vkCreateBuffer failed");

    vkGetBufferMemoryRequirements(context.device, buffer.buffer, &buffer_memory_requirements);
    memory_allocate_info.memoryTypeIndex = get_memory_type_index(buffer_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.gpu_memory_properties);
    res = vkAllocateMemory(context.device, &memory_allocate_info, context.allocation_callbacks, &buffer.memory);
    VULKAN_VERIFY(res, "vkAllocateMemory failed");

    res = vkBindBufferMemory(context.device, buffer.buffer, buffer.memory, 0);
    VULKAN_VERIFY(res, "vkBindBufferMemory failed");

    // command buffer for copying data to the VRAM
    VkCommandBuffer command_buffer;

    VkCommandBufferAllocateInfo cb_allocate_info;
    init_defaults(cb_allocate_info);
    cb_allocate_info.commandBufferCount = 1;
    cb_allocate_info.commandPool = context.main_command_pool;
    res = vkAllocateCommandBuffers(context.device, &cb_allocate_info, &command_buffer);
    VULKAN_VERIFY(res, "vkAllocateCommandBuffers failed");

    VkCommandBufferBeginInfo begin_info;
    init_defaults(begin_info);
    res = vkBeginCommandBuffer(command_buffer, &begin_info);
    VULKAN_VERIFY(res, "vkBeginCommandBuffer failed");

    {
        VkBufferCopy regions;
        regions.srcOffset = 0;
        regions.dstOffset = 0;
        regions.size = size;
        vkCmdCopyBuffer(command_buffer, src_buffer, buffer.buffer, 1, &regions);
    }

    res = vkEndCommandBuffer(command_buffer);
    VULKAN_VERIFY(res, "vkEndCommandBuffer failed");

    VkSubmitInfo submit_info;
    init_defaults(submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    res = vkQueueSubmit(context.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    VULKAN_VERIFY(res, "vkQueueSubmit failed");
    res = vkQueueWaitIdle(context.graphics_queue);

    vkFreeCommandBuffers(context.device, context.main_command_pool, 1, &command_buffer);
    vkDestroyBuffer(context.device, src_buffer, context.allocation_callbacks);
    vkFreeMemory(context.device, src_device_memory, context.allocation_callbacks);

    return VK_SUCCESS;
}

void destroy_buffer(Buffer& buffer, const VulkanContext& context)
{
    vkDestroyBuffer(context.device, buffer.buffer, context.allocation_callbacks);
    vkFreeMemory(context.device, buffer.memory, context.allocation_callbacks);
}

VkResult create_dynamic_buffer(Buffer& buffer, const VulkanContext& context, const uint8_t* data, uint32_t size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo buffer_create_info;
    init_defaults(buffer_create_info);
    buffer_create_info.usage = usage;
    buffer_create_info.size = size;

    VkResult res = vkCreateBuffer(context.device, &buffer_create_info, context.allocation_callbacks, &buffer.buffer);
    VULKAN_VERIFY(res, "vkCreateBuffer failed");

    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(context.device, buffer.buffer, &buffer_memory_requirements);
    uint32_t memory_type = get_memory_type_index(buffer_memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, context.gpu_memory_properties);
    VkMemoryAllocateInfo memory_allocate_info;
    init_defaults(memory_allocate_info);
    memory_allocate_info.memoryTypeIndex = memory_type;
    memory_allocate_info.allocationSize = size;
    res = vkAllocateMemory(context.device, &memory_allocate_info, context.allocation_callbacks, &buffer.memory);
    VULKAN_VERIFY(res, "vkAllocateMemory failed");

    void* mapped_memory = nullptr;
    res = vkMapMemory(context.device, buffer.memory, 0, size, 0, &mapped_memory);
    VULKAN_VERIFY(res, "vkMapMemory failed");
    memcpy(mapped_memory, data, size);
    vkUnmapMemory(context.device, buffer.memory);

    res = vkBindBufferMemory(context.device, buffer.buffer, buffer.memory, 0);
    VULKAN_VERIFY(res, "vkBindBufferMemory failed");

    return VK_SUCCESS;
}

VkResult init_vulkan_context(VulkanContext& context, const char* appname, uint32_t width, uint32_t height, bool enable_default_debug_layers)
{
    context.width = width;
    context.height = height;
#ifdef _WIN32
    context.platform_data.hinstance = GetModuleHandle(nullptr);
#endif

    if (enable_default_debug_layers)
    {
         // put the names of the default debug layers (took them from the LunarG sample)
         const char* debug_layer_extensions[] = 
         {
            "VK_LAYER_LUNARG_threading",      "VK_LAYER_LUNARG_mem_tracker",
            "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_draw_state",
            "VK_LAYER_LUNARG_param_checker",  "VK_LAYER_LUNARG_swapchain",
            "VK_LAYER_LUNARG_device_limits",  "VK_LAYER_LUNARG_image",
            "VK_LAYER_GOOGLE_unique_objects",
        };
        context.instance_debug_layers_extensions.resize(ARRAY_SIZE(debug_layer_extensions));
        for (size_t i = 0, size = context.instance_debug_layers_extensions.size(); i < size; ++i)
            context.instance_debug_layers_extensions[i] = debug_layer_extensions[i];
        context.device_debug_layers_extensions = context.instance_debug_layers_extensions;
    }

    // check validation layers
    uint32_t instance_layer_count = 0;
    VkResult res = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumerateInstanceLayerProperties failed");
    if (instance_layer_count > 0)
    {
        context.instance_layer_properties.resize(instance_layer_count);
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, &context.instance_layer_properties[0]);
        VULKAN_VERIFY(res, "vkEnumerateInstanceLayerProperties failed");
        context.enabled_instance_debug_layers_extensions.reserve(instance_layer_count);
        for (uint32_t i = 0; i < instance_layer_count; ++i)
        {
            for (size_t j = 0, size = context.instance_debug_layers_extensions.size(); j < size; ++j)
            {
                if (!strcmp(context.instance_layer_properties[i].layerName, context.instance_debug_layers_extensions[j]))
                {
                    context.enabled_instance_debug_layers_extensions.push_back(context.instance_debug_layers_extensions[j]);
                }
            }
        }
    }

    // check extensions
#ifdef _WIN32
    const char* plaform_surface_extension_name = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif

    bool surface_extension_found = false;
    bool platform_surface_extension_found = false;
    uint32_t instance_extension_count = 0;
    res = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumerateInstanceExtensionProperties failed");
    uint32_t enabled_extensions_count = 0;
    if (instance_extension_count > 0)
    {
        context.instance_extension_properties.resize(instance_extension_count);
        context.enabled_extensions.resize(instance_extension_count);
        res = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, &context.instance_extension_properties[0]);
        VULKAN_VERIFY(res, "vkEnumerateInstanceExtensionProperties failed");
        // we need the surface extensions
        for (uint32_t i = 0; i < instance_extension_count; ++i)
        {
            const VkExtensionProperties& property = context.instance_extension_properties[i];
            if (!strcmp(property.extensionName, VK_KHR_SURFACE_EXTENSION_NAME))
            {
                surface_extension_found = true;
                context.enabled_extensions[enabled_extensions_count++] = property.extensionName;
            }
            else if (!strcmp(property.extensionName, plaform_surface_extension_name))
            {
                platform_surface_extension_found = true;
                context.enabled_extensions[enabled_extensions_count++] = property.extensionName;
            }
        }
    }

    VERIFY(surface_extension_found && platform_surface_extension_found, "Surface extensions are not available", VK_ERROR_INITIALIZATION_FAILED);

    // create VK instance
    VkApplicationInfo app_info;
    init_defaults(app_info);
    app_info.pApplicationName = appname;
    app_info.pEngineName = appname;

    VkInstanceCreateInfo create_info;
    init_defaults(create_info);
    create_info.enabledExtensionCount = enabled_extensions_count;
    create_info.pApplicationInfo = &app_info;
    create_info.ppEnabledExtensionNames = &context.enabled_extensions[0];
    create_info.ppEnabledLayerNames = context.enabled_instance_debug_layers_extensions.empty() ? nullptr : 
        &context.enabled_instance_debug_layers_extensions[0];
    create_info.enabledLayerCount = static_cast<uint32_t>(context.enabled_instance_debug_layers_extensions.size());

    res = vkCreateInstance(&create_info, nullptr, &context.instance);
    VULKAN_VERIFY(res, "vkCreateInstance failed");

    // get instance's extensions function pointers
    context.extension_functions.vkCreateDebugReportCallbackEXT = 
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugReportCallbackEXT");
    context.extension_functions.vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugReportCallbackEXT");

    auto t = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(context.instance, "vkEnumeratePhysicalDevices");

    /*if (enable_default_debug_layers)
    {
        VERIFY(context.extension_functions.vkCreateDebugReportCallbackEXT != nullptr &&
            context.extension_functions.vkDestroyDebugReportCallbackEXT != nullptr,
            "Invalid driver", VK_ERROR_INITIALIZATION_FAILED);

        VkDebugReportCallbackCreateInfoEXT dbg_create_info;
        dbg_create_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | 
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        dbg_create_info.pfnCallback = debug_callback;
        dbg_create_info.pNext = nullptr;
        dbg_create_info.pUserData = nullptr;
        dbg_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        res = context.extension_functions.vkCreateDebugReportCallbackEXT(context.instance, &dbg_create_info, context.allocation_callbacks, &context.debug_report_callback);
        VULKAN_VERIFY(res, "vkCreateDebugReportCallbackEXT failed");
    }*/

    // GPUs
    uint32_t physical_device_count = 0;
    res = vkEnumeratePhysicalDevices(context.instance, &physical_device_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumeratePhysicalDevices failed");
    VERIFY(physical_device_count > 0, "Invalid number of GPUs", VK_ERROR_INITIALIZATION_FAILED);
    std::vector<VkPhysicalDevice> devices(physical_device_count);
    res = vkEnumeratePhysicalDevices(context.instance, &physical_device_count, &devices[0]);
    VULKAN_VERIFY(res, "vkEnumeratePhysicalDevices failed");
    context.main_gpu = devices[0];

    // now we're going to check layers and extension available for the GPU
    uint32_t device_layer_count = 0;
    res = vkEnumerateDeviceLayerProperties(context.main_gpu, &device_layer_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumerateDeviceLayerProperties failed");
    if (device_layer_count > 0)
    {
        std::vector<VkLayerProperties> properties(device_layer_count);
        res = vkEnumerateDeviceLayerProperties(context.main_gpu, &device_layer_count, &properties[0]);
        VULKAN_VERIFY(res, "vkEnumerateDeviceLayerProperties failed");
        context.enabled_device_debug_layers_extensions.reserve(device_layer_count);
        for (uint32_t i = 0; i < device_layer_count; ++i)
        {
            for (size_t j = 0, size = context.enabled_device_debug_layers_extensions.size(); j < size; ++j)
            {
                if (!strcmp(properties[i].layerName, context.device_debug_layers_extensions[j]))
                {
                    context.enabled_device_debug_layers_extensions.push_back(context.device_debug_layers_extensions[j]);
                }
            }
        }
    }

    uint32_t device_extension_count = 0;
    bool swapchain_extension_found = false;
    res = vkEnumerateDeviceExtensionProperties(context.main_gpu, nullptr, &device_extension_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumerateDeviceExtensionProperties failed");
    std::vector<VkExtensionProperties> device_extensions;
    uint32_t device_enabled_extensions_count = 0;
    if (device_extension_count > 0)
    {
        device_extensions.resize(device_extension_count);
        context.device_enabled_extensions.resize(device_extension_count);
        res = vkEnumerateDeviceExtensionProperties(context.main_gpu, nullptr, &device_extension_count, &device_extensions[0]);
        VULKAN_VERIFY(res, "vkEnumerateDeviceExtensionProperties failed");
        // we need the swapchain extensions
        for (uint32_t i = 0; i < device_extension_count; ++i)
        {
            const VkExtensionProperties& property = device_extensions[i];
            if (!strcmp(property.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            {
                swapchain_extension_found = true;
                context.device_enabled_extensions[device_enabled_extensions_count++] = property.extensionName;
            }
        }
    }

    VERIFY(swapchain_extension_found, "Swapchain extension hasn't been found", VK_ERROR_INITIALIZATION_FAILED);

    vkGetPhysicalDeviceProperties(context.main_gpu, &context.device_properties);
    vkGetPhysicalDeviceMemoryProperties(context.main_gpu, &context.gpu_memory_properties);

    const uint32_t max_uint32 = std::numeric_limits<uint32_t>::max();

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context.main_gpu, &queue_count, nullptr);
    VERIFY(queue_count > 0, "Invalid queues number", VK_ERROR_INITIALIZATION_FAILED);
    context.queue_properties.resize(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(context.main_gpu, &queue_count, &context.queue_properties[0]);
    // check that we have a queue supporting rendering
    bool rendering_queue_found = false;
    bool compute_queue_found = false;
    context.graphics_queue_family_index = max_uint32;
    for (uint32_t i = 0; i < queue_count; ++i)
    {
        const VkQueueFamilyProperties& properties = context.queue_properties[i];
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            rendering_queue_found = true;
            context.graphics_queue_family_index = i;
        }
        if (properties.queueCount & VK_QUEUE_COMPUTE_BIT)
            compute_queue_found = true;
    }

    VERIFY(rendering_queue_found, "Rendering queue hasn't been found", VK_ERROR_INITIALIZATION_FAILED);

#ifdef _WIN32
    context.platform_data.hwnd = create_window(appname, width, height, context.platform_data.hinstance);
#endif

    // create a surface
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info;
    surface_info.flags = 0;
    surface_info.hinstance = context.platform_data.hinstance;
    surface_info.hwnd = context.platform_data.hwnd;
    surface_info.pNext = nullptr;
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

    res = vkCreateWin32SurfaceKHR(context.instance, &surface_info, context.allocation_callbacks, &context.surface);
    VULKAN_VERIFY(res, "vkCreateWin32SurfaceKHR failed");
#endif

    context.present_queue = max_uint32;

    std::vector<VkBool32> supports_present(queue_count);
    for (uint32_t i = 0; i < queue_count; ++i)
    {
        res = vkGetPhysicalDeviceSurfaceSupportKHR(context.main_gpu, i, context.surface, &supports_present[i]);
        VULKAN_VERIFY(res, "vkGetPhysicalDeviceFurfaceSupportKHR failed");
        context.present_queue = i;
        if (supports_present[i] == VK_TRUE && context.queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            context.graphics_queue_family_index = i;
            break;
        }
    }

    VERIFY(context.graphics_queue_family_index != max_uint32 && context.present_queue != max_uint32,
           "Invalid queues", VK_ERROR_INITIALIZATION_FAILED);

    const float queue_priority = 0.0f;
    VkDeviceQueueCreateInfo device_queue_create_info;
    init_defaults(device_queue_create_info);
    device_queue_create_info.pQueuePriorities = &queue_priority;
    device_queue_create_info.queueCount = 1;
    device_queue_create_info.queueFamilyIndex = context.graphics_queue_family_index;

    VkDeviceCreateInfo device_create_info;
    init_defaults(device_create_info);
    device_create_info.enabledExtensionCount = device_enabled_extensions_count;
    device_create_info.ppEnabledExtensionNames = &context.device_enabled_extensions[0];
    device_create_info.pQueueCreateInfos = &device_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.ppEnabledLayerNames = context.enabled_device_debug_layers_extensions.empty() ? nullptr :
        &context.enabled_device_debug_layers_extensions[0];
    device_create_info.enabledLayerCount = static_cast<uint32_t>(context.enabled_device_debug_layers_extensions.size());
    res = vkCreateDevice(context.main_gpu, &device_create_info, context.allocation_callbacks, &context.device);
    VULKAN_VERIFY(res, "vkCreateDevice failed");

    vkGetDeviceQueue(context.device, context.graphics_queue_family_index, 0, &context.graphics_queue);

    res = create_depth_image_view(context.depth_image_view, context.depth_image, width, height, context);
    VULKAN_VERIFY(res, "create_depth_image_view failed");

    // get possible swapchain modes
    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.main_gpu, context.surface, &present_modes_count, nullptr);
    context.present_modes.resize(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context.main_gpu, context.surface, &present_modes_count, &context.present_modes[0]);

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < present_modes_count; ++i)
    {
        if (context.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        if (present_mode != VK_PRESENT_MODE_MAILBOX_KHR &&
            context.present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.main_gpu, context.surface, &context.surface_capabilities);

    // create a swapchain
    VERIFY(context.surface_capabilities.currentExtent.width == static_cast<uint32_t>(-1) ||
           context.surface_capabilities.currentExtent.width == width, "Invalid current extent", VK_ERROR_INITIALIZATION_FAILED);
    VkExtent2D swapchain_extent = { width, height };

    VkSwapchainCreateInfoKHR swapchain_create_info;
    init_defaults(swapchain_create_info);
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    swapchain_create_info.minImageCount = context.surface_capabilities.minImageCount;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.preTransform = context.surface_capabilities.currentTransform;
    swapchain_create_info.surface = context.surface;
    res = vkCreateSwapchainKHR(context.device, &swapchain_create_info, context.allocation_callbacks, &context.swapchain);
    VULKAN_VERIFY(res, "vkCreateSwapchainKHR failed");

    uint32_t swapchain_images_count = 0;
    res = vkGetSwapchainImagesKHR(context.device, context.swapchain, &swapchain_images_count, nullptr);
    VULKAN_VERIFY(res, "vkGetSwapchainImagesKHR failed");
    context.color_images.resize(swapchain_images_count);
    res = vkGetSwapchainImagesKHR(context.device, context.swapchain, &swapchain_images_count, &context.color_images[0]);
    VULKAN_VERIFY(res, "vkGetSwapchainImagesKHR failed");

    context.color_imageviews.resize(swapchain_images_count);
    for (uint32_t i = 0; i < swapchain_images_count; ++i)
    {
        VkImageViewCreateInfo image_view_create_info;
        init_defaults(image_view_create_info);
        image_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;;
        image_view_create_info.image = context.color_images[i];
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        res = vkCreateImageView(context.device, &image_view_create_info, context.allocation_callbacks, &context.color_imageviews[i]);
        VULKAN_VERIFY(res, "vkCreateImageView failed");
    }

    // create the main render pass
    VkAttachmentDescription attachment_descriptions[2];
    init_defaults(attachment_descriptions[0]);
    init_defaults(attachment_descriptions[1]);
    attachment_descriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;

    attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_descriptions[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
    attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref;
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref;
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_desc;
    init_defaults(subpass_desc);
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_attachment_ref;
    subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;

    VkRenderPassCreateInfo render_pass_create_info;
    init_defaults(render_pass_create_info);
    render_pass_create_info.attachmentCount = 2;
    render_pass_create_info.pAttachments = attachment_descriptions;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_desc;

    res = vkCreateRenderPass(context.device, &render_pass_create_info, context.allocation_callbacks, &context.main_render_pass);
    VULKAN_VERIFY(res, "vkCreateRenderPass failed");

    // create the main framebuffer
    VkImageView main_framebuffer_attachments[2] = { context.color_imageviews[0], context.depth_image_view };

    VkFramebufferCreateInfo framebuffer_create_info;
    init_defaults(framebuffer_create_info);
    framebuffer_create_info.attachmentCount = 2;
    framebuffer_create_info.height = 512;
    framebuffer_create_info.layers = 1;
    framebuffer_create_info.pAttachments = main_framebuffer_attachments;
    framebuffer_create_info.renderPass = context.main_render_pass;
    framebuffer_create_info.width = 512;
    res = vkCreateFramebuffer(context.device, &framebuffer_create_info, context.allocation_callbacks, &context.main_framebuffer);
    VULKAN_VERIFY(res, "vkCreateFramebuffer failed");

    VkCommandPoolCreateInfo command_pool_create_info;
    init_defaults(command_pool_create_info);
    command_pool_create_info.queueFamilyIndex = context.graphics_queue_family_index;

    res = vkCreateCommandPool(context.device, &command_pool_create_info, context.allocation_callbacks, &context.main_command_pool);
    VULKAN_VERIFY(res, "vkCreateCommandPool failed");

    // create a semaphore that can be used as a signal when present is finished
    VkSemaphoreCreateInfo semaphore_create_info;
    init_defaults(semaphore_create_info);

    res = vkCreateSemaphore(context.device, &semaphore_create_info, context.allocation_callbacks, &context.present_semaphore);
    VULKAN_VERIFY(res, "vkCreateSemaphore failed");

    VkPipelineCacheCreateInfo pipeline_cache_create_info;
    init_defaults(pipeline_cache_create_info);
    res = vkCreatePipelineCache(context.device, &pipeline_cache_create_info, context.allocation_callbacks, &context.pipeline_cache);
    VULKAN_VERIFY(res, "vkCreatePipelineCache failed");

    return VK_SUCCESS;
}

void destroy_vulkan_context(VulkanContext& context)
{
    vkDestroyPipelineCache(context.device, context.pipeline_cache, context.allocation_callbacks);

    vkDestroySemaphore(context.device, context.present_semaphore, context.allocation_callbacks);

    vkDestroyCommandPool(context.device, context.main_command_pool, context.allocation_callbacks);

    vkDestroyFramebuffer(context.device, context.main_framebuffer, context.allocation_callbacks);
    vkDestroyRenderPass(context.device, context.main_render_pass, context.allocation_callbacks);

    for (size_t i = 0, size = context.color_imageviews.size(); i < size; ++i)
        vkDestroyImageView(context.device, context.color_imageviews[i], context.allocation_callbacks);

    vkDestroySwapchainKHR(context.device, context.swapchain, context.allocation_callbacks);

    destroy_image_view(context.depth_image_view, context.depth_image, context);

    vkDestroyDevice(context.device, context.allocation_callbacks);

    vkDestroySurfaceKHR(context.instance, context.surface, context.allocation_callbacks);

    if (context.extension_functions.vkDestroyDebugReportCallbackEXT != nullptr)
        context.extension_functions.vkDestroyDebugReportCallbackEXT(context.instance, context.debug_report_callback, context.allocation_callbacks);

    vkDestroyInstance(context.instance, context.allocation_callbacks);
}

bool app_message_loop(VulkanContext& context)
{
#ifdef _WIN32
    return wndprocess(context.platform_data.hwnd);
#endif
}

}
