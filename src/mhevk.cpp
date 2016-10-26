#include "mhevk.hpp"

namespace mhe {
namespace vk {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject, size_t location, int32_t msgCode,
    const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
    char buff[256];
    sprintf(buff, "%s %s\n", pLayerPrefix, pMsg);
    printf(buff);
#ifdef _MSC_VER
    OutputDebugString(buff);
#endif
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

namespace
{

VkResult create_instance(VulkanContext& context, const char* appname, bool enable_validation)
{
    // check extensions
#ifdef _WIN32
    const char* plaform_surface_extension_name = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif

    bool surface_extension_found = false;
    bool platform_surface_extension_found = false;
    uint32_t instance_extension_count = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
    uint32_t enabled_extensions_count = 0;
    if (instance_extension_count > 0)
    {
        context.instance_extension_properties.resize(instance_extension_count);
        context.enabled_extensions.resize(instance_extension_count);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, &context.instance_extension_properties[0]));
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
            else if (enable_validation && !strcmp(property.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
                context.enabled_extensions[enabled_extensions_count++] = property.extensionName;
        }
    }

    VERIFY(surface_extension_found && platform_surface_extension_found, "Surface extensions are not available", VK_ERROR_INITIALIZATION_FAILED);

    ApplicationInfo app_info(appname, 0, appname, 0);

    InstanceCreateInfo create_info(&app_info,
        static_cast<uint32_t>(context.enabled_instance_debug_layers_extensions.size()),
        context.enabled_instance_debug_layers_extensions.empty() ? nullptr : &context.enabled_instance_debug_layers_extensions[0],
        enabled_extensions_count, &context.enabled_extensions[0]);

    VK_CHECK(vkCreateInstance(create_info.c_struct(), context.allocation_callbacks, &context.instance));

    // get instance's extensions function pointers
    context.extension_functions.vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugReportCallbackEXT");
    context.extension_functions.vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugReportCallbackEXT");

    return VK_SUCCESS;
}

VkResult init_physical_device(VulkanContext& context)
{
    uint32_t physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physical_device_count, nullptr));
    VERIFY(physical_device_count > 0, "Invalid number of GPUs", VK_ERROR_INITIALIZATION_FAILED);
    context.gpus.resize(physical_device_count);
    std::vector<VkPhysicalDevice> devices(physical_device_count);
    VK_CHECK(vkEnumeratePhysicalDevices(context.instance, &physical_device_count, &devices[0]));

    for (uint32_t i = 0; i < physical_device_count; ++i)
        VK_CHECK(context.gpus[i].init(context, devices[i]));

    context.main_gpu = &context.gpus[0];

    return VK_SUCCESS;
}

VkResult init_window(VulkanContext& context, const char* appname)
{
#ifdef _WIN32
    context.platform_data.hwnd = create_window(appname, context.width, context.height, context.platform_data.hinstance);
    if (context.platform_data.hwnd == 0)
        return VK_ERROR_INITIALIZATION_FAILED;
    return VK_SUCCESS;
#endif
    return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult init_surface(VulkanContext& context)
{
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info;
    surface_info.flags = 0;
    surface_info.hinstance = context.platform_data.hinstance;
    surface_info.hwnd = context.platform_data.hwnd;
    surface_info.pNext = nullptr;
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

    VkResult res = vkCreateWin32SurfaceKHR(context.instance, &surface_info, context.allocation_callbacks, &context.surface);
    VULKAN_VERIFY(res, "vkCreateWin32SurfaceKHR failed");
#endif
    return VK_SUCCESS;
}

VkResult init_device(VulkanContext& context)
{
    context.devices.resize(context.gpus.size());
    for (size_t i = 0, size = context.gpus.size(); i < size; ++i)
    {
        VK_CHECK(context.devices[i].init(context, &context.gpus[i]));
    }
    context.main_device = &context.devices[0];

    return VK_SUCCESS;
}

VkResult init_pipeline_cache(VulkanContext& context)
{
    VkPipelineCacheCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK(vkCreatePipelineCache(*context.main_device, &create_info, context.allocation_callbacks, &context.main_pipeline_cache));
    return VK_SUCCESS;
}

VkResult init_descriptor_pools(VulkanContext& context)
{
    VkDescriptorPoolSize descriptor_pool_size[2] =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 16 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 16 }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets = 8;
    descriptor_pool_create_info.poolSizeCount = 2;
    descriptor_pool_create_info.pPoolSizes = descriptor_pool_size;
    VK_CHECK(vkCreateDescriptorPool(*context.main_device, &descriptor_pool_create_info, context.allocation_callbacks, &context.descriptor_pools.main_descriptor_pool));

    return VK_SUCCESS;
}

VkResult init_descriptor_set_layouts(VulkanContext& context)
{
    VkDescriptorSetLayoutBinding per_model_layout_binding[1] =
    {
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pBindings = per_model_layout_binding;
    descriptor_set_layout_create_info.bindingCount = array_size(per_model_layout_binding);
    VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &context.descriptor_set_layouts.mesh_layout));

    VkDescriptorSetLayoutBinding material_layout_binding[2] =
    {
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };

    descriptor_set_layout_create_info.pBindings = material_layout_binding;
    descriptor_set_layout_create_info.bindingCount = array_size(material_layout_binding);
    VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &context.descriptor_set_layouts.material_layout));

    VkDescriptorSetLayoutBinding posteffect_layout_binding[1] =
    {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    };
    descriptor_set_layout_create_info.pBindings = posteffect_layout_binding;
    descriptor_set_layout_create_info.bindingCount = array_size(posteffect_layout_binding);
    VK_CHECK(vkCreateDescriptorSetLayout(*context.main_device, &descriptor_set_layout_create_info, context.allocation_callbacks, &context.descriptor_set_layouts.posteffect_layout));

    return VK_SUCCESS;
}

void destroy_descriptor_set_layouts(VulkanContext& context)
{
    vkDestroyDescriptorSetLayout(*context.main_device, context.descriptor_set_layouts.material_layout, context.allocation_callbacks);
    vkDestroyDescriptorSetLayout(*context.main_device, context.descriptor_set_layouts.mesh_layout, context.allocation_callbacks);
}
}

VkSamplerCreateInfo SamplerCreateInfo(VkFilter mag_filter, VkFilter min_filter, VkSamplerMipmapMode mipmap_mode,
    VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v, VkSamplerAddressMode address_mode_w)
{
    VkSamplerCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.addressModeU = address_mode_u;
    create_info.addressModeV = address_mode_v;
    create_info.addressModeW = address_mode_w;
    create_info.mipmapMode = mipmap_mode;
    create_info.magFilter = mag_filter;
    create_info.minFilter = min_filter;

    return create_info;
}

VkSemaphoreCreateInfo SemaphoreCreateInfo()
{
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return create_info;
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
            "VK_LAYER_GOOGLE_unique_objects", "VK_LAYER_LUNARG_standard_validation",
            "VK_LAYER_LUNARG_core_validation"//, "VK_LAYER_RENDERDOC_Capture"
        };
        context.instance_debug_layers_extensions.resize(ARRAY_SIZE(debug_layer_extensions));
        for (size_t i = 0, size = context.instance_debug_layers_extensions.size(); i < size; ++i)
            context.instance_debug_layers_extensions[i] = debug_layer_extensions[i];
        context.device_debug_layers_extensions = context.instance_debug_layers_extensions;
    }

    // check validation layers
    uint32_t instance_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
    if (instance_layer_count > 0)
    {
        context.instance_layer_properties.resize(instance_layer_count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, &context.instance_layer_properties[0]));
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

    // create VK instance
    VK_CHECK(create_instance(context, appname, enable_default_debug_layers));

    if (enable_default_debug_layers)
    {
        VERIFY(context.extension_functions.vkCreateDebugReportCallbackEXT != nullptr &&
            context.extension_functions.vkDestroyDebugReportCallbackEXT != nullptr,
            "Invalid driver", VK_ERROR_INITIALIZATION_FAILED);

        VkDebugReportCallbackCreateInfoEXT dbg_create_info;dbg_create_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        dbg_create_info.pfnCallback = debug_callback;
        dbg_create_info.pNext = nullptr;
        dbg_create_info.pUserData = nullptr;
        dbg_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        VK_CHECK(context.extension_functions.vkCreateDebugReportCallbackEXT(context.instance, &dbg_create_info, context.allocation_callbacks, &context.debug_report_callback));
    }

    VK_CHECK(init_window(context, appname));
    VK_CHECK(init_surface(context));
    // GPUs
    VK_CHECK(init_physical_device(context));
    VK_CHECK(init_device(context));

    context.default_gpu_interface.device = context.main_device;

    // swapchain
    Swapchain::Settings swapchain_settings;
    VK_CHECK(context.main_swapchain.init(context, context.main_device, swapchain_settings));

    // depth stencil
    ImageView::Settings ds_settings;
    ds_settings.width = context.width;
    ds_settings.height = context.height;
    ds_settings.format = VK_FORMAT_D24_UNORM_S8_UINT;
    ds_settings.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ds_settings.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    GPUInterface gpu_iface;
    gpu_iface.device = context.main_device;
    VK_CHECK(context.main_depth_stencil_image_view.init(context, gpu_iface, ds_settings, VK_NULL_HANDLE, nullptr, 0));

    // default render passes
    RenderPass::Settings::AttachmentDesc attachment_descs[2];
    attachment_descs[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attachment_descs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_descs[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
    attachment_descs[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    RenderPass::Settings render_pass_settings;
    render_pass_settings.descs = attachment_descs;
    render_pass_settings.count = 2;
    VK_CHECK(context.render_passes.main_render_pass.init(context, gpu_iface, render_pass_settings));

    VK_CHECK(context.main_swapchain.create_framebuffers(context, &context.render_passes.main_render_pass));

    VK_CHECK(context.command_pools.main_graphics_command_pool.init(context, gpu_iface));
    VK_CHECK(context.command_pools.resource_uploading_command_pool.init(context, gpu_iface));

    VK_CHECK(init_pipeline_cache(context));

    VK_CHECK(init_descriptor_pools(context));
    VK_CHECK(init_descriptor_set_layouts(context));

    return VK_SUCCESS;
}

void destroy_vulkan_context(VulkanContext& context)
{
    destroy_descriptor_set_layouts(context);

    vkDestroyDescriptorPool(*context.main_device, context.descriptor_pools.main_descriptor_pool, context.allocation_callbacks);

    vkDestroyPipelineCache(*context.main_device, context.main_pipeline_cache, context.allocation_callbacks);

    context.command_pools.resource_uploading_command_pool.destroy(context);
    context.command_pools.main_graphics_command_pool.destroy(context);

    context.render_passes.main_render_pass.destroy(context);

    context.main_depth_stencil_image_view.destroy(context);

    context.main_swapchain.destroy(context);

    for (PhysicalDevice& physical_device : context.gpus)
        physical_device.destroy(context);

    for (Device& device : context.devices)
        device.destroy(context);

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

VkResult PhysicalDevice::init(VulkanContext& context, VkPhysicalDevice id)
{
    id_ = id;
    // we're going to check layers and extension available for the GPU
    uint32_t device_layer_count = 0;
    VK_VERIFY(vkEnumerateDeviceLayerProperties(id_, &device_layer_count, nullptr));
    if (device_layer_count > 0)
    {
        std::vector<VkLayerProperties> properties(device_layer_count);
        VK_CHECK(vkEnumerateDeviceLayerProperties(id_, &device_layer_count, &properties[0]));
        enabled_device_debug_layers_extensions_.reserve(device_layer_count);
        for (uint32_t i = 0; i < device_layer_count; ++i)
        {
            for (size_t j = 0, size = context.device_debug_layers_extensions.size(); j < size; ++j)
            {
                if (!strcmp(properties[i].layerName, context.device_debug_layers_extensions[j]))
                {
                    enabled_device_debug_layers_extensions_.push_back(context.device_debug_layers_extensions[j]);
                }
            }
        }
    }

    return check_properties(context);
}

VkResult PhysicalDevice::check_properties(VulkanContext& context)
{
    vkGetPhysicalDeviceProperties(id_, &properties_);
    vkGetPhysicalDeviceMemoryProperties(id_, &memory_properties_);

    const uint32_t max_uint32 = std::numeric_limits<uint32_t>::max();

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(id_, &queue_count, nullptr);
    VERIFY(queue_count > 0, "Invalid queues number", VK_ERROR_INITIALIZATION_FAILED);
    queue_properties_.resize(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(id_, &queue_count, &queue_properties_[0]);
    // check that we have a queue supporting rendering
    bool rendering_queue_found = false;
    bool compute_queue_found = false;
    for (uint32_t i = 0; i < queue_count; ++i)
    {
        const VkQueueFamilyProperties& properties = queue_properties_[i];
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            rendering_queue_found = true;
            graphics_queue_family_index_ = i;
        }
        if (properties.queueCount & VK_QUEUE_COMPUTE_BIT)
            compute_queue_found = true;
    }

    VERIFY(rendering_queue_found, "Rendering queue hasn't been found", VK_ERROR_INITIALIZATION_FAILED);

    // and find the rendering queue's family index
    std::vector<VkBool32> supports_present(queue_count);
    for (uint32_t i = 0; i < queue_count; ++i)
    {
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(id_, i, context.surface, &supports_present[i]));
        present_queue_family_index_ = i;
        if (supports_present[i] == VK_TRUE && queue_properties_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_queue_family_index_ = i;
            break;
        }
    }

    VERIFY(graphics_queue_family_index_ != invalid_index && present_queue_family_index_ != invalid_index,
        "Invalid queues", VK_ERROR_INITIALIZATION_FAILED);

    // get possible swapchain modes
    uint32_t present_modes_count = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(id_, context.surface, &present_modes_count, nullptr));
    present_modes_.resize(present_modes_count);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(id_, context.surface, &present_modes_count, &present_modes_[0]));
    // get possible swapchain formats
    uint32_t surface_formats_count = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(id_, context.surface, &surface_formats_count, nullptr));
    surface_formats_.resize(surface_formats_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(id_, context.surface, &surface_formats_count, &surface_formats_[0]));

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(id_, context.surface, &surface_capabilities_));

    return VK_SUCCESS;
}

void PhysicalDevice::destroy(VulkanContext&)
{}

uint32_t PhysicalDevice::get_memory_type_index(const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags flags) const
{
    for (uint32_t i = 0; i < 32; ++i)
    {
        if (memory_requirements.memoryTypeBits & (1 << i))
        {
            if ((memory_properties_.memoryTypes[i].propertyFlags & flags) == flags)
                return i;
        }
    }
    return 0;
}

VkResult Queue::init(VulkanContext& context, const GPUInterface& gpu_iface, VkQueue id)
{
    id_ = id;
    gpu_iface_ = gpu_iface;
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(vkCreateSemaphore(*gpu_iface.device, &create_info, context.allocation_callbacks, &present_semaphore_));
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(*gpu_iface.device, &fence_create_info, context.allocation_callbacks, &submit_fence_));
    return VK_SUCCESS;
}

void Queue::destroy(VulkanContext& context)
{
    vkDestroyFence(*gpu_iface_.device, submit_fence_, context.allocation_callbacks);
    vkDestroySemaphore(*gpu_iface_.device, present_semaphore_, context.allocation_callbacks);
}

VkResult Queue::submit(const CommandBuffer* command_buffers, uint32_t count,
    const VkSemaphore* wait_semaphores, uint32_t wait_semaphores_count,
    const VkSemaphore* signal_semaphores, uint32_t signal_semaphores_count)
{
    std::vector<VkCommandBuffer> buffers(count);
    for (uint32_t i = 0; i < count; ++i)
        buffers[i] = command_buffers[i];

    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = count;
    submit_info.pCommandBuffers = &buffers[0];
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.signalSemaphoreCount = signal_semaphores_count;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.waitSemaphoreCount = wait_semaphores_count;
    submit_info.pWaitDstStageMask = &wait_stages;

    VK_CHECK(vkQueueSubmit(id_, 1, &submit_info, submit_fence_));

    return VK_SUCCESS;
}

VkResult Queue::present(const Swapchain* swapchain)
{
    VkResult res = VK_TIMEOUT;
    do
    {
        res = vkWaitForFences(*gpu_iface_.device, 1, &submit_fence_, VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    while (res == VK_TIMEOUT);

    VkSwapchainKHR tmp_swapchain = *swapchain;

    uint32_t current_buffer = swapchain->current_buffer();

    VkSemaphore wait_semaphore = present_semaphore_;

    VkPresentInfoKHR present_info = {};
    present_info.pImageIndices = &current_buffer;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pSwapchains = &tmp_swapchain;
    present_info.swapchainCount = 1;
    present_info.pWaitSemaphores = &wait_semaphore;
    present_info.waitSemaphoreCount = 1;

    VK_CHECK(vkQueuePresentKHR(id_, &present_info));

    return VK_SUCCESS;
}

VkResult Queue::wait_idle()
{
    VK_CHECK(vkQueueWaitIdle(id_));
    return VK_SUCCESS;
}

VkResult Device::init(VulkanContext& context, PhysicalDevice* physical_device)
{
    ASSERT(physical_device != nullptr, "Invalid physical device");

    physical_device_ = physical_device;

    const bool use_validation = !context.enabled_instance_debug_layers_extensions.empty();

    uint32_t device_extension_count = 0;
    bool swapchain_extension_found = false;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device->id(), nullptr, &device_extension_count, nullptr));
    std::vector<VkExtensionProperties> device_extensions;
    uint32_t device_enabled_extensions_count = 0;
    if (device_extension_count > 0)
    {
        device_extensions.resize(device_extension_count);
        device_enabled_extensions_.resize(device_extension_count);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device->id(), nullptr, &device_extension_count, &device_extensions[0]));
        // we need the swapchain extensions
        for (uint32_t i = 0; i < device_extension_count; ++i)
        {
            const VkExtensionProperties& property = device_extensions[i];
            if (!strcmp(property.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            {
                swapchain_extension_found = true;
                device_enabled_extensions_[device_enabled_extensions_count++] = property.extensionName;
            }
        }
    }

    VERIFY(swapchain_extension_found, "Swapchain extension hasn't been found", VK_ERROR_INITIALIZATION_FAILED);

    const auto& device_debug_layers = physical_device->enabled_debug_layers();
    uint32_t validation_layers_count = use_validation ? device_debug_layers.size() : 0;
    const char* const* validation_layers = validation_layers_count > 0 ? &device_debug_layers[0] : nullptr;

    const float queue_priority = 0.0f;
    DeviceQueueCreateInfo queue_create_info(physical_device->graphics_queue_family_index(), 1, &queue_priority);
    DeviceCreateInfo device_create_info(1, &queue_create_info,
        validation_layers_count, validation_layers,
        device_enabled_extensions_count, &device_enabled_extensions_[0],
        nullptr);
    VK_VERIFY(vkCreateDevice(physical_device->id(), device_create_info.c_struct(), context.allocation_callbacks, &id_));

    VkQueue graphics_queue_id;
    vkGetDeviceQueue(id_, physical_device->graphics_queue_family_index(), 0, &graphics_queue_id);

    GPUInterface gpu_iface;
    gpu_iface.device = this;
    VK_CHECK(graphics_queue_.init(context, gpu_iface, graphics_queue_id));

    return VK_SUCCESS;
}

void Device::destroy(VulkanContext& context)
{
    graphics_queue_.destroy(context);
    vkDestroyDevice(id_, context.allocation_callbacks);
}

uint32_t Device::get_memory_type_index(const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags flags) const
{
    return physical_device_->get_memory_type_index(memory_requirements, flags);
}

VkResult Swapchain::init(VulkanContext& context, Device* device, const Settings& settings)
{
    device_ = device;
    ASSERT(device_ != nullptr, "Invalid GPU");

    PhysicalDevice* physical_device = device->physical_device();

    settings_ = settings;

    const auto& present_modes = physical_device->present_modes();

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0, size = present_modes.size(); i < size; ++i)
    {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        if (present_mode != VK_PRESENT_MODE_MAILBOX_KHR && present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    const VkSurfaceCapabilitiesKHR& surface_capabilities = physical_device->surface_capabilities();

    const auto& surface_formats = physical_device->surface_formats();

    // create a swapchain
    VERIFY(surface_capabilities.currentExtent.width == static_cast<uint32_t>(-1) ||
        (surface_capabilities.currentExtent.width == context.width &&
         surface_capabilities.currentExtent.height == context.height), "Invalid current extent", VK_ERROR_INITIALIZATION_FAILED);

    VkExtent2D swapchain_extent = { context.width, context.height };

    VkSwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.flags = 0;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchain_create_info.imageSharingMode = settings.image_sharing_mode;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageFormat = settings.format;
    swapchain_create_info.minImageCount = surface_capabilities.minImageCount;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.surface = context.surface;
    VK_CHECK(vkCreateSwapchainKHR(device_->id(), &swapchain_create_info, context.allocation_callbacks, &id_));

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK(vkCreateSemaphore(*device, &semaphore_create_info, context.allocation_callbacks, &next_image_semaphore_));

    return init_images(context);
}

void Swapchain::destroy(VulkanContext& context)
{
    for (auto& fb : framebuffers_)
        fb.destroy(context);
    vkDestroySemaphore(*device_, next_image_semaphore_, context.allocation_callbacks);
    for (auto& imageview : color_images_)
        imageview.destroy(context);
    vkDestroySwapchainKHR(device_->id(), id_, context.allocation_callbacks);
}

VkResult Swapchain::init_images(VulkanContext& context)
{
    auto device = device_->id();

    uint32_t swapchain_images_count = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(device, id_, &swapchain_images_count, nullptr));
    color_images_.resize(swapchain_images_count);
    std::vector<VkImage> images(swapchain_images_count);
    VK_CHECK(vkGetSwapchainImagesKHR(device, id_, &swapchain_images_count, &images[0]));

    GPUInterface gpu_iface;
    gpu_iface.device = device_;
    for (uint32_t i = 0; i < swapchain_images_count; ++i)
    {
        ImageView::Settings settings;
        settings.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
        settings.format = settings_.format;

        VK_CHECK(color_images_[i].init(context, gpu_iface, settings, images[i], nullptr, 0));
    }

    return VK_SUCCESS;
}

VkResult Swapchain::acquire_next_image()
{
    VK_CHECK(vkAcquireNextImageKHR(*device_, id_, std::numeric_limits<uint64_t>::max(), next_image_semaphore_, VK_NULL_HANDLE, &current_buffer_));
    return VK_SUCCESS;
}

VkResult Swapchain::create_framebuffers(VulkanContext& context, RenderPass* render_pass)
{
    framebuffers_.resize(color_images_.size());

    GPUInterface gpu_iface;
    gpu_iface.device = device_;

    Framebuffer::Settings framebuffer_settings;
    for (size_t i = 0, size = color_images_.size(); i < size; ++i)
    {
        const ImageView* image_views[2] = { &color_images_[i], &context.main_depth_stencil_image_view };
        framebuffer_settings.attachments = image_views;
        framebuffer_settings.count = 2;
        framebuffer_settings.height = context.height;
        framebuffer_settings.width = context.width;
        framebuffer_settings.render_pass = render_pass;
        VK_CHECK(framebuffers_[i].init(context, gpu_iface, framebuffer_settings));
    }

    return VK_SUCCESS;
}

VkResult ImageView::init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings, VkImage image, const uint8_t* data, uint32_t size)
{
    settings_ = settings;
    gpu_iface_ = gpu_iface;

    const VkDevice device = gpu_iface.device->id();

    VkExtent3D extent = {settings.width, settings.height, settings.depth};
    ImageCreateInfo image_create_info(VK_IMAGE_TYPE_2D, settings.format, extent, settings.mip_levels, settings.array_layers, VK_SAMPLE_COUNT_1_BIT,
        settings.usage, VK_SHARING_MODE_EXCLUSIVE);
    if (data != nullptr)
        image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkMemoryRequirements image_memory_requirements;
    if (image == VK_NULL_HANDLE)
    {
        VK_CHECK(vkCreateImage(device, image_create_info.c_struct(), context.allocation_callbacks, &image_));

        vkGetImageMemoryRequirements(device, image_, &image_memory_requirements);
        MemoryAllocateInfo allocate_info(image_memory_requirements.size,
            gpu_iface.device->get_memory_type_index(image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        VK_CHECK(vkAllocateMemory(device, allocate_info.c_struct(), context.allocation_callbacks, &memory_));
        VK_CHECK(vkBindImageMemory(device, image_, memory_, 0));
    }
    else
        image_ = image;

    if (data != nullptr)
    {
        VkImage src_image;
        VkDeviceMemory src_memory;

        image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
        image_create_info.usage = settings.usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        VK_CHECK(vkCreateImage(device, image_create_info.c_struct(), context.allocation_callbacks, &src_image));
        vkGetImageMemoryRequirements(device, src_image, &image_memory_requirements);

        // allocate memory for the image
        MemoryAllocateInfo allocate_info(image_memory_requirements.size,
            gpu_iface.device->get_memory_type_index(image_memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
        VK_CHECK(vkAllocateMemory(device, allocate_info.c_struct(), context.allocation_callbacks, &src_memory));
        VK_CHECK(vkBindImageMemory(device, src_image, src_memory, 0));

        // init source image
        void* dst = nullptr;
        VK_CHECK(vkMapMemory(device, src_memory, 0, size, 0, &dst));
        memcpy(dst, data, size);
        vkUnmapMemory(device, src_memory);

        // command buffer for copying data to the VRAM
        CommandBuffer command_buffer;
        context.command_pools.resource_uploading_command_pool.create_command_buffers(context, &command_buffer, 1);

        VkImageCopy image_copy = {};
        image_copy.extent = { settings.width, settings.height, settings.depth };
        image_copy.srcSubresource.aspectMask = settings.aspect_mask;
        image_copy.srcSubresource.baseArrayLayer = 0;
        image_copy.srcSubresource.layerCount = settings.array_layers;
        image_copy.srcSubresource.mipLevel = 0;
        image_copy.dstSubresource = image_copy.srcSubresource;

        command_buffer
        .begin()
            .copy_image_command(src_image, image_, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &image_copy, 1)
        .end();

        context.main_device->graphics_queue().submit(&command_buffer, 1);
        context.main_device->graphics_queue().wait_idle();

        context.command_pools.resource_uploading_command_pool.destroy_command_buffers(context, &command_buffer, 1);

        vkFreeMemory(device, src_memory, context.allocation_callbacks);
        vkDestroyImage(device, src_image, context.allocation_callbacks);
    }

    VkImageViewCreateInfo image_view_create_info;
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = nullptr;
    image_view_create_info.format = settings.format;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = image_;
    image_view_create_info.flags = 0;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.layerCount = settings.array_layers;
    image_view_create_info.subresourceRange.levelCount = settings.mip_levels;
    image_view_create_info.subresourceRange.aspectMask = settings.aspect_mask;
    VK_CHECK(vkCreateImageView(device, &image_view_create_info, context.allocation_callbacks, &imageview_));

    return VK_SUCCESS;
}

void ImageView::destroy(VulkanContext& context)
{
    vkDestroyImageView(gpu_iface_.device->id(), imageview_, context.allocation_callbacks);
    if (memory_ != VK_NULL_HANDLE)
    {
        vkFreeMemory(gpu_iface_.device->id(), memory_, context.allocation_callbacks);
        vkDestroyImage(gpu_iface_.device->id(), image_, context.allocation_callbacks);
    }
}

VkResult Buffer::init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings, const uint8_t* data, uint32_t size)
{
    gpu_iface_ = gpu_iface;
    settings_ = settings;

    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pQueueFamilyIndices = settings.queue_family_indices;
    create_info.queueFamilyIndexCount = settings.queue_family_indices_count;
    create_info.sharingMode = settings.sharing_mode;
    create_info.usage = settings.usage;
    create_info.size = size;
    VK_CHECK(vkCreateBuffer(*gpu_iface_.device, &create_info, context.allocation_callbacks, &buffer_));

    desc_buffer_info_.buffer = buffer_;
    desc_buffer_info_.offset = 0;
    desc_buffer_info_.range = size;

    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements(*gpu_iface_.device, buffer_, &buffer_memory_requirements);
    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = buffer_memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = gpu_iface_.device->get_memory_type_index(buffer_memory_requirements, settings.memory_properties);
    VK_CHECK(vkAllocateMemory(*gpu_iface_.device, &memory_allocate_info, context.allocation_callbacks, &memory_));
    VK_CHECK(vkBindBufferMemory(*gpu_iface_.device, buffer_, memory_, 0));

    if (data != nullptr)
        return update(context, data, size);

    return VK_SUCCESS;
}

void Buffer::destroy(VulkanContext& context)
{
    if (memory_ != VK_NULL_HANDLE)
        vkFreeMemory(*gpu_iface_.device, memory_, context.allocation_callbacks);
    if (buffer_ != VK_NULL_HANDLE)
        vkDestroyBuffer(*gpu_iface_.device, buffer_, context.allocation_callbacks);
}

VkResult Buffer::update(VulkanContext& context, const uint8_t* data, uint32_t size)
{
    if (settings_.memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        void* mapped_memory = nullptr;
        VK_CHECK(vkMapMemory(*gpu_iface_.device, memory_, 0, size, 0, &mapped_memory));
        memcpy(mapped_memory, data, size);
        vkUnmapMemory(*gpu_iface_.device, memory_);
        return VK_SUCCESS;
    }

    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pQueueFamilyIndices = settings_.queue_family_indices;
    create_info.queueFamilyIndexCount = settings_.queue_family_indices_count;
    create_info.sharingMode = settings_.sharing_mode;
    create_info.usage = settings_.usage;
    create_info.size = size;

    VkMemoryRequirements buffer_memory_requirements;
    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;;

    VkBuffer src_buffer;
    VkDeviceMemory src_memory;
    VK_CHECK(vkCreateBuffer(*gpu_iface_.device, &create_info, context.allocation_callbacks, &src_buffer));
    vkGetBufferMemoryRequirements(*gpu_iface_.device, src_buffer, &buffer_memory_requirements);
    memory_allocate_info.allocationSize = buffer_memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = gpu_iface_.device->get_memory_type_index(buffer_memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VK_CHECK(vkAllocateMemory(*gpu_iface_.device, &memory_allocate_info, context.allocation_callbacks, &src_memory));
    VK_CHECK(vkBindBufferMemory(*gpu_iface_.device, src_buffer, src_memory, 0));

    void* mapped_memory = nullptr;
    VK_CHECK(vkMapMemory(*gpu_iface_.device, src_memory, 0, size, 0, &mapped_memory));
    memcpy(mapped_memory, data, size);
    vkUnmapMemory(*gpu_iface_.device, src_memory);

    // upload data
    // command buffer for copying data to the VRAM
    CommandBuffer command_buffer;
    context.command_pools.resource_uploading_command_pool.create_command_buffers(context, &command_buffer, 1);

    VkBufferCopy buffer_copy = {};
    buffer_copy.size = size;

    command_buffer
        .begin()
        .copy_buffer(src_buffer, buffer_, &buffer_copy, 1)
        .end();

    context.main_device->graphics_queue().submit(&command_buffer, 1);
    context.main_device->graphics_queue().wait_idle();

    context.command_pools.resource_uploading_command_pool.destroy_command_buffers(context, &command_buffer, 1);

    vkFreeMemory(*gpu_iface_.device, src_memory, context.allocation_callbacks);
    vkDestroyBuffer(*gpu_iface_.device, src_buffer, context.allocation_callbacks);

    return VK_SUCCESS;
}

VkResult RenderPass::init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings)
{
    gpu_iface_ = gpu_iface;

    VkAttachmentDescription attachment_descriptions[max_attachments];
    for (uint32_t i = 0; i < settings.count; ++i)
    {
        if (settings.descs[i].layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            attachment_descriptions[i].finalLayout = settings.descs[i].layout;
        else
            attachment_descriptions[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment_descriptions[i].flags = 0;
        attachment_descriptions[i].format = settings.descs[i].format;
        attachment_descriptions[i].initialLayout = settings.descs[i].layout;
        attachment_descriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

    VkAttachmentReference color_attachment_refs[max_attachments];
    VkAttachmentReference depth_attachment_ref;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference* depth_attachment_ref_ptr = nullptr;

    for (uint32_t i = 0, color_attachment_index = 0; i < settings.count; ++i)
    {
        if (settings.descs[i].layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            ASSERT(depth_attachment_ref_ptr == nullptr, "Invalid number of depth-stencil attachments");
            depth_attachment_ref_ptr = &depth_attachment_ref;
            depth_attachment_ref.attachment = i;
        }
        else
        {
            VkAttachmentReference& color_attachment = color_attachment_refs[color_attachment_index++];
            color_attachment.attachment = i;
            color_attachment.layout = settings.descs[i].layout;
        }
    }

    VkSubpassDescription subpass_desc = {};
    subpass_desc.colorAttachmentCount = settings.count;
    if (depth_attachment_ref_ptr != nullptr)
        --subpass_desc.colorAttachmentCount;
    subpass_desc.pColorAttachments = color_attachment_refs;
    subpass_desc.pDepthStencilAttachment = depth_attachment_ref_ptr;
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::vector<VkSubpassDependency> dependencies(settings.dependencies_count * 2);
    for (uint32_t i = 0, j = 0; i < settings.dependencies_count; ++i, j += 2)
    {
        dependencies[j].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[j].dstSubpass = 0;
        dependencies[j].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[j].srcAccessMask = settings.dependencies[i].src_access;
        dependencies[j].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[j].dstAccessMask = settings.dependencies[i].dst_access;
        dependencies[j].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[j + 1] = dependencies[j];
        dependencies[j + 1].srcSubpass = 0;
        dependencies[j + 1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[j].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[j].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dependencies[j].srcAccessMask = settings.dependencies[i].dst_access;
        dependencies[j].dstAccessMask = settings.dependencies[i].src_access;
    }

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = settings.count;
    render_pass_create_info.pAttachments = attachment_descriptions;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_desc;
    render_pass_create_info.dependencyCount = dependencies.size();
    render_pass_create_info.pDependencies = !dependencies.empty() ? &dependencies[0] : nullptr;

    VK_CHECK(vkCreateRenderPass(gpu_iface.device->id(), &render_pass_create_info, context.allocation_callbacks, &id_));

    return VK_SUCCESS;
}

void RenderPass::destroy(VulkanContext& context)
{
    vkDestroyRenderPass(gpu_iface_.device->id(), id_, context.allocation_callbacks);
}

VkResult Framebuffer::init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings)
{
    gpu_iface_ = gpu_iface;
    settings_ = settings;

    VkImageView image_views[max_attachments];
    for (uint32_t i = 0; i < settings.count; ++i)
        image_views[i] = settings.attachments[i]->image_view_id();

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.attachmentCount = settings.count;
    framebuffer_create_info.height = settings.height;
    framebuffer_create_info.layers = 1;
    framebuffer_create_info.pAttachments = image_views;
    framebuffer_create_info.renderPass = *settings.render_pass;
    framebuffer_create_info.width = settings.width;
    VK_CHECK(vkCreateFramebuffer(*gpu_iface.device, &framebuffer_create_info, context.allocation_callbacks, &id_));

    return VK_SUCCESS;
}

void Framebuffer::destroy(VulkanContext& context)
{
    vkDestroyFramebuffer(*gpu_iface_.device, id_, context.allocation_callbacks);
}

VkResult Texture::init(VulkanContext& context, const GPUInterface& gpu_iface,
    const ImageView::Settings& image_settings, const SamplerSettings& sampler_settings,
    const uint8_t* data, uint32_t size)
{
    VK_CHECK(image_view_.init(context, gpu_iface, image_settings, VK_NULL_HANDLE, data, size));

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.addressModeU = sampler_settings.address_mode_u;
    sampler_create_info.addressModeV = sampler_settings.address_mode_v;
    sampler_create_info.addressModeW = sampler_settings.address_mode_w;
    sampler_create_info.magFilter = sampler_settings.mag_filter;
    sampler_create_info.minFilter = sampler_settings.min_filter;
    sampler_create_info.mipmapMode = sampler_settings.mipmap_mode;
    VK_CHECK(vkCreateSampler(*gpu_iface.device, &sampler_create_info, context.allocation_callbacks, &sampler_));

    descriptor_image_info_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptor_image_info_.imageView = image_view_.image_view_id();
    descriptor_image_info_.sampler = sampler_;

    return VK_SUCCESS;
}

void Texture::destroy(VulkanContext& context)
{
    vkDestroySampler(*image_view_.gpu_interface().device, sampler_, context.allocation_callbacks);
    image_view_.destroy(context);
}

VkResult CommandPool::init(VulkanContext& context, const GPUInterface& gpu_iface)
{
    gpu_iface_ = gpu_iface;

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = gpu_iface.device->physical_device()->graphics_queue_family_index();
    VK_CHECK(vkCreateCommandPool(*gpu_iface.device, &create_info, context.allocation_callbacks, &id_));

    return VK_SUCCESS;
}

void CommandPool::destroy(VulkanContext& context)
{
    vkDestroyCommandPool(*gpu_iface_.device, id_, context.allocation_callbacks);
}

VkResult CommandPool::create_command_buffers(VulkanContext& context, CommandBuffer* buffers, uint32_t count)
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.commandPool = id_;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = count;

    std::vector<VkCommandBuffer> tmp_buffers(count);
    VK_CHECK(vkAllocateCommandBuffers(*gpu_iface_.device, &allocate_info, &tmp_buffers[0]));
    for (uint32_t i = 0; i < count; ++i)
    {
        VK_CHECK(buffers[i].init(context, tmp_buffers[i]));
    }

    return VK_SUCCESS;
}

void CommandPool::destroy_command_buffers(VulkanContext& context, CommandBuffer* buffers, uint32_t count)
{
    std::vector<VkCommandBuffer> tmp_buffers(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        tmp_buffers[i] = buffers[i];
        buffers[i].destroy(context);
    }
    vkFreeCommandBuffers(*gpu_iface_.device, id_, count, &tmp_buffers[0]);
}

VkResult CommandBuffer::init(VulkanContext&, VkCommandBuffer id)
{
    id_ = id;
    return VK_SUCCESS;
}

void CommandBuffer::destroy(VulkanContext&)
{
}

CommandBuffer& CommandBuffer::begin()
{
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(id_, &begin_info));
    return *this;
}

void CommandBuffer::end()
{
    VK_CHECK(vkEndCommandBuffer(id_));
}

CommandBuffer& CommandBuffer::begin_render_pass_command(
    const Framebuffer* framebuffer, const vec4& color, float depth, uint32_t stencil,
    bool clear_color, bool clear_depth, bool clear_stencil)
{
    VkClearValue clear_values[2];
    VkClearColorValue clear_color_value;
    clear_color_value.float32[0] = color.x;
    clear_color_value.float32[1] = color.y;
    clear_color_value.float32[2] = color.z;
    clear_color_value.float32[3] = color.w;
    clear_values[0].color = clear_color_value;
    VkClearDepthStencilValue clear_depth_stencil_value;
    clear_depth_stencil_value.depth = depth;
    clear_depth_stencil_value.stencil = stencil;
    clear_values[1].depthStencil = clear_depth_stencil_value;

    VkRect2D rect;
    rect.offset.x = rect.offset.y = 0;
    rect.extent.width = framebuffer->width();
    rect.extent.height = framebuffer->height();

    uint32_t clear_value_count = clear_color + (clear_depth | clear_stencil);
    const VkClearValue* clear_value = clear_color ? &clear_values[0] : &clear_values[1];

    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.clearValueCount = clear_value_count;
    render_pass_begin_info.framebuffer = *framebuffer;
    render_pass_begin_info.pClearValues = clear_value;
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderArea = rect;
    render_pass_begin_info.renderPass = *framebuffer->render_pass();
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    vkCmdBeginRenderPass(id_, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    return *this;
}

CommandBuffer& CommandBuffer::end_render_pass_command()
{
    vkCmdEndRenderPass(id_);
    return *this;
}

CommandBuffer& CommandBuffer::set_viewport_command(const VkRect2D& rect)
{
    VkViewport viewport;
    viewport.x = static_cast<float>(rect.offset.x);
    viewport.y = static_cast<float>(rect.offset.y);
    viewport.width = static_cast<float>(rect.extent.width);
    viewport.height = static_cast<float>(rect.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(id_, 0, 1, &viewport);

    return *this;
}

CommandBuffer& CommandBuffer::set_scissor_command(const VkRect2D& rect)
{
    vkCmdSetScissor(id_, 0, 1, &rect);
    return *this;
}

CommandBuffer& CommandBuffer::copy_image_command(VkImage src, VkImage dst, VkImageLayout src_layout, VkImageLayout dst_layout,
    const VkImageCopy* regions, uint32_t regions_count)
{
    vkCmdCopyImage(id_, src, src_layout, dst, dst_layout, regions_count, regions);
    return *this;
}

CommandBuffer& CommandBuffer::copy_buffer(VkBuffer src, VkBuffer dst, const VkBufferCopy* regions, uint32_t regions_count)
{
    vkCmdCopyBuffer(id_, src, dst, regions_count, regions);
    return *this;
}

CommandBuffer& CommandBuffer::bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point)
{
    vkCmdBindPipeline(id_, bind_point, pipeline);
    return *this;
}

CommandBuffer& CommandBuffer::bind_descriptor_set(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout,
    const VkDescriptorSet* descriptor_sets, uint32_t descriptor_sets_count, uint32_t first)
{
    vkCmdBindDescriptorSets(id_, bind_point, pipeline_layout, first, descriptor_sets_count, descriptor_sets, 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::draw(const Mesh& mesh, size_t part_index)
{
    const MeshPart& part = mesh.parts()[part_index];
    const Buffer& vbuffer = mesh.vbuffer();
    const Buffer& ibuffer = mesh.ibuffer();
    VkBuffer vk_buffer = vbuffer;

    VkDeviceSize size[1] = {0};

    vkCmdBindVertexBuffers(id_, 0, 1, &vk_buffer, size);
    vkCmdBindIndexBuffer(id_, ibuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(id_, part.indices_count, 1, part.ibuffer_offset, part.vbuffer_offset, 0);
    return *this;
}

CommandBuffer& CommandBuffer::transfer_image_layout(VkImage image, VkImageLayout src_layout, VkImageLayout dst_layout, VkImageAspectFlags aspect_flags)
{
    VkImageMemoryBarrier image_memory_barrier = {};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = image;
    image_memory_barrier.oldLayout = src_layout;
    image_memory_barrier.newLayout = dst_layout;
    image_memory_barrier.subresourceRange.aspectMask = aspect_flags;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier(id_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

    return *this;
}

CommandBuffer& CommandBuffer::render_target_barrier(VkImage image, VkImageLayout layout, VkImageAspectFlags aspect_flags)
{
    VkImageMemoryBarrier image_memory_barrier = {};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = image;
    image_memory_barrier.oldLayout = layout;
    image_memory_barrier.newLayout = layout;
    image_memory_barrier.subresourceRange.aspectMask = aspect_flags;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier(id_, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

    return *this;
}

void GeometryLayout::vertex_input_info(VkPipelineVertexInputStateCreateInfo& info)
{
    // vertex input
    static const VkVertexInputAttributeDescription vi_attr_desc[4] =
    {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, // pos
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vec3) }, // nrm
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, 2 * sizeof(vec3) }, //tng
        { 3, 0, VK_FORMAT_R32G32_SFLOAT, 3 * sizeof(vec3) } // tex
    };

    static const VkVertexInputBindingDescription vi_binding_desc = { 0, sizeof(vk::GeometryLayout::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

    memset(&info, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pVertexAttributeDescriptions = vi_attr_desc;
    info.vertexAttributeDescriptionCount = 4;
    info.pVertexBindingDescriptions = &vi_binding_desc;
    info.vertexBindingDescriptionCount = 1;
}

void FullscreenLayout::vertex_input_info(VkPipelineVertexInputStateCreateInfo& info)
{
    // vertex input
    static const VkVertexInputAttributeDescription vi_attr_desc[4] =
    {
        { 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 }, // pos
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(vec4) } // tex
    };

    static const VkVertexInputBindingDescription vi_binding_desc = { 0, sizeof(vk::FullscreenLayout::Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

    memset(&info, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pVertexAttributeDescriptions = vi_attr_desc;
    info.vertexAttributeDescriptionCount = 2;
    info.pVertexBindingDescriptions = &vi_binding_desc;
    info.vertexBindingDescriptionCount = 1;
}

void init_fullscreen_viewport(VkViewport& viewport, const VulkanContext& context)
{
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.width = static_cast<float>(context.width);
    viewport.height = static_cast<float>(context.height);
    viewport.x = 0;
    viewport.y = 0;
}

VkResult Material::init(vk::VulkanContext& context, const vk::GPUInterface& gpu_iface)
{
    gpu_iface_ = gpu_iface;

    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pSetLayouts = &context.descriptor_set_layouts.material_layout;
    allocate_info.descriptorSetCount = 1;
    allocate_info.descriptorPool = context.descriptor_pools.main_descriptor_pool;
    VK_CHECK(vkAllocateDescriptorSets(*gpu_iface.device, &allocate_info, &descriptor_set_));

    return VK_SUCCESS;
}

void Material::destroy(VulkanContext& context)
{
    vkFreeDescriptorSets(*gpu_iface_.device, context.descriptor_pools.main_descriptor_pool, 1, &descriptor_set_);
}

void Material::build_descriptor_set()
{
    VkDescriptorImageInfo infos[max_texture_index];
    for (size_t i = 0; i < max_texture_index; ++i)
        infos[i] = textures_[i]->descriptor_image_info();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.descriptorCount = max_texture_index;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.dstBinding = 1;
    write_descriptor_set.dstSet = descriptor_set_;
    write_descriptor_set.pImageInfo = infos;
    vkUpdateDescriptorSets(*gpu_iface_.device, 1, &write_descriptor_set, 0, nullptr);
}

VkResult Mesh::create_cube(vk::VulkanContext& context, const vk::GPUInterface& gpu_iface)
{
    parts_.resize(1);
    uint16_t cube_indices[36] = { 0, 2, 1, 0, 3, 2, 7, 4, 6, 4, 5, 6, 8, 10, 9, 11, 10, 8, 12, 13, 14, 12, 14, 15, 16, 18, 17, 16, 19, 18, 20, 21, 22, 20, 22, 23 };
    vk::GeometryLayout::Vertex cube_vertices[24] = {
        // front
        { { -0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { -0.5f, +0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { +0.5f, +0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { +0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
        // back
        { { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { -0.5f, +0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { +0.5f, +0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { +0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
        // left
        { { -0.5f, -0.5f, -0.5f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { -0.5f, +0.5f, -0.5f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { -0.5f, +0.5f, +0.5f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { -0.5f, -0.5f, +0.5f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
        // right
        { { +0.5f, -0.5f, -0.5f },{ +1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { +0.5f, +0.5f, -0.5f },{ +1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { +0.5f, +0.5f, +0.5f },{ +1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { +0.5f, -0.5f, +0.5f },{ +1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
        // top
        { { -0.5f, +0.5f, +0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { -0.5f, +0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { +0.5f, +0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { +0.5f, +0.5f, +0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
        // bottom
        { { -0.5f, -0.5f, +0.5f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
        { { -0.5f, -0.5f, -0.5f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
        { { +0.5f, -0.5f, -0.5f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
        { { +0.5f, -0.5f, +0.5f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } }
    };

    uint32_t graphics_queue_family_index = gpu_iface.device->physical_device()->graphics_queue_family_index();

    vk::Buffer::Settings buffer_settings;
    buffer_settings.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_settings.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VK_CHECK(vbuffer_.init(context, gpu_iface, buffer_settings, reinterpret_cast<const uint8_t*>(cube_vertices), sizeof(cube_vertices)));

    buffer_settings.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VK_CHECK(ibuffer_.init(context, gpu_iface, buffer_settings, reinterpret_cast<const uint8_t*>(cube_indices), sizeof(cube_indices)));

    parts_[0].vbuffer_offset = 0;
    parts_[0].ibuffer_offset = 0;
    parts_[0].indices_count = array_size(cube_indices);
    parts_[0].material = nullptr;

    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pSetLayouts = &context.descriptor_set_layouts.mesh_layout;
    allocate_info.descriptorSetCount = 1;
    allocate_info.descriptorPool = context.descriptor_pools.main_descriptor_pool;
    VK_CHECK(vkAllocateDescriptorSets(*context.main_device, &allocate_info, &descriptor_set_));

    buffer_settings.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer_settings.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VK_CHECK(uniform_.init(context, gpu_iface, buffer_settings, reinterpret_cast<const uint8_t*>(&mat4x4::identity()), sizeof(mat4x4)));

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstSet = descriptor_set_;
    write_descriptor_set.pBufferInfo = &uniform_.descriptor_buffer_info();
    vkUpdateDescriptorSets(*gpu_iface.device, 1, &write_descriptor_set, 0, nullptr);

    return VK_SUCCESS;
}

VkResult Mesh::create_quad(vk::VulkanContext& context, const vk::GPUInterface& gpu_iface)
{
    parts_.resize(1);
    uint16_t indices[3] = { 0, 1, 2 };
    vk::FullscreenLayout::Vertex vertices[3] = {
        { { -1.0f, -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { -1.0f, 3.0f, 1.0f, 1.0f }, { 0.0f, 2.0f } },
        { { 3.0f, -1.0f, 1.0f, 1.0f }, { 2.0f, 0.0f } },
    };

    uint32_t graphics_queue_family_index = gpu_iface.device->physical_device()->graphics_queue_family_index();

    vk::Buffer::Settings buffer_settings;
    buffer_settings.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_settings.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VK_CHECK(vbuffer_.init(context, gpu_iface, buffer_settings, reinterpret_cast<const uint8_t*>(vertices), sizeof(vertices)));

    buffer_settings.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VK_CHECK(ibuffer_.init(context, gpu_iface, buffer_settings, reinterpret_cast<const uint8_t*>(indices), sizeof(indices)));

    parts_[0].vbuffer_offset = 0;
    parts_[0].ibuffer_offset = 0;
    parts_[0].indices_count = array_size(indices);
    parts_[0].material = nullptr;

    return VK_SUCCESS;
}

void Mesh::destroy(vk::VulkanContext& context)
{
    uniform_.destroy(context);
    if (descriptor_set_ != VK_NULL_HANDLE)
        VK_CHECK(vkFreeDescriptorSets(*context.main_device, context.descriptor_pools.main_descriptor_pool, 1, &descriptor_set_));
    ibuffer_.destroy(context);
    vbuffer_.destroy(context);
}

namespace
{
struct TGAHeader
{
    char header[12];
};

struct TGA
{
    char header[6];
};

const char uncompressed_tga_header[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const char uncompressed_tga_header_bw[12] = { 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

VkResult load_uncompressed_tga(ImageData& image, FILE* f)
{
    TGA tga;
    fread(tga.header, sizeof(tga.header), 1, f);
    image.width = (tga.header[1] << 8) | tga.header[0];
    image.height = (tga.header[3] << 8) | tga.header[2];
    // format
    uint8_t bpp = tga.header[4];
    if (bpp < 24) return VK_ERROR_INITIALIZATION_FAILED;
    size_t size = image.width * image.height * (bpp / 8);
    image.data.resize(size);
    fread(reinterpret_cast<char*>(&image.data[0]), size, 1, f);
    image.format = bpp == 32 ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8_SRGB;

    return VK_SUCCESS;
}
}

VkResult load_tga_image(ImageData& image, const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f)
        return VK_ERROR_INITIALIZATION_FAILED;
    TGAHeader header;
    fread(header.header, sizeof(header.header), 1, f);
    if (::memcmp(header.header, uncompressed_tga_header, 12) && ::memcmp(header.header, uncompressed_tga_header_bw, 12))
    {
        ASSERT(false, "Only uncompressed TGA images are supported now");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkResult res = load_uncompressed_tga(image, f);
    fclose(f);
    return res;
}

}}