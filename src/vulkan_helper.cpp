#include "vulkan_helper.hpp"

namespace mhe {

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
    RECT wr = { 0, 0, width, height };
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

uint32_t get_memory_type_index(const VkMemoryRequirements& requirements, const VkPhysicalDeviceMemoryProperties& memory_properties)
{
    for (uint32_t i = 0; i < 32; ++i)
    {
        if (requirements.memoryTypeBits & (1 << i))
        {
            if ((memory_properties.memoryTypes[i].propertyFlags & requirements.memoryTypeBits) == 0)
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
    image_create_info.arrayLayers = 1;
    image_create_info.extent = { 1, 1, 1 };
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
    image_view_create_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
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

VkResult create_depth_image_view(VkImageView& imageview, VkImage& image, uint32_t width, uint32_t height, const VulkanContext& vulkan_context)
{
    VkImageCreateInfo depth_image_create_info;
    init_defaults(depth_image_create_info);
    depth_image_create_info.extent = { width, height, 1 };
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
    depth_image_allocate_info.memoryTypeIndex = get_memory_type_index(depth_image_memory_requirements, vulkan_context.gpu_memory_properties);
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

VkResult init_vulkan_context(VulkanContext& context, const char* appname, uint32_t width, uint32_t height)
{
    context.width = width;
    context.height = height;
#ifdef _WIN32
    context.platform_data.hinstance = GetModuleHandle(nullptr);
#endif

    // check validation layers
    uint32_t instance_layer_count;
    VkResult res = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
    VULKAN_VERIFY(res, "vkEnumerateInstanceLayerProperties failed");
    if (instance_layer_count > 0)
    {
        context.instance_layer_properties.resize(instance_layer_count);
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, &context.instance_layer_properties[0]);
        VULKAN_VERIFY(res, "vkEnumerateInstanceLayerProperties failed");
        // TODO: enable validation layers
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

    res = vkCreateInstance(&create_info, nullptr, &context.instance);
    VULKAN_VERIFY(res, "vkCreateInstance failed");

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
        // TODO: enable validation layers
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

    return VK_SUCCESS;
}

void destroy_vulkan_context(VulkanContext& context)
{
    vkDestroyCommandPool(context.device, context.main_command_pool, context.allocation_callbacks);

    vkDestroyFramebuffer(context.device, context.main_framebuffer, context.allocation_callbacks);
    vkDestroyRenderPass(context.device, context.main_render_pass, context.allocation_callbacks);

    for (size_t i = 0, size = context.color_imageviews.size(); i < size; ++i)
        vkDestroyImageView(context.device, context.color_imageviews[i], context.allocation_callbacks);

    vkDestroySwapchainKHR(context.device, context.swapchain, context.allocation_callbacks);

    destroy_image_view(context.depth_image_view, context.depth_image, context);

    vkDestroyDevice(context.device, context.allocation_callbacks);

    vkDestroySurfaceKHR(context.instance, context.surface, context.allocation_callbacks);

    vkDestroyInstance(context.instance, context.allocation_callbacks);
}

bool app_message_loop(VulkanContext& context)
{
#ifdef _WIN32
    return wndprocess(context.platform_data.hwnd);
#endif
}

}