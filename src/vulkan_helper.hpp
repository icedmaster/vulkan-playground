#ifndef __VULKAN_HELPER_HPP__
#define __VULKAN_HELPER_HPP__

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

#include <vector>
#include <cstdio>
#include <cassert>

#ifdef _DEBUG
#define VERIFY_PRINT(text) {printf("%s", text); assert(0);}
#else
#define VERIFY_PRINT(text) {printf("%s", text);}
#endif

#define VERIFY(condition, text, result) if (!(condition)) {VERIFY_PRINT(text); return result;}

#define VULKAN_VERIFY(res, text) VERIFY(res == VK_SUCCESS, text, res)

#define ASSERT(condition, text) if (!(condition)) {VERIFY_PRINT(text);}

namespace mhe {

struct VulkanContext;

#ifdef _WIN32
struct PlatformData
{
    HMODULE hinstance;
    HWND hwnd;
};
#endif

uint32_t get_memory_type_index(const VkMemoryRequirements& requirements, const VkPhysicalDeviceMemoryProperties& memory_properties);

void init_defaults(VkApplicationInfo& appinfo);
void init_defaults(VkInstanceCreateInfo& create_info);
void init_defaults(VkDeviceQueueCreateInfo& device_queue_create_info);
void init_defaults(VkDeviceCreateInfo& device_create_info);
void init_defaults(VkImageCreateInfo& image_create_info);
void init_defaults(VkImageViewCreateInfo& image_view_create_info);
void init_defaults(VkSwapchainCreateInfoKHR& swapchain_create_info);
void init_defaults(VkAttachmentDescription& attachment_description);
void init_defaults(VkSubpassDescription& subpass_desc);
void init_defaults(VkRenderPassCreateInfo& render_pass_create_info);
void init_defaults(VkFramebufferCreateInfo& framebuffer_create_info);
void init_defaults(VkCommandPoolCreateInfo& command_pool_create_info);
void init_defaults(VkCommandBufferAllocateInfo& cb_allocate_info);
void init_defaults(VkCommandBufferBeginInfo& begin_info);
void init_defaults(VkSubmitInfo& submit_info);
void init_defaults(VkPresentInfoKHR& present_info);

VkResult create_depth_image_view(VkImageView& imageview, VkImage& image, uint32_t width, uint32_t height, const VulkanContext& vulkan_context);
void destroy_image_view(VkImageView& image_view, VkImage& vk_image, const VulkanContext& vulkan_context);

struct VulkanContext
{
    PlatformData platform_data;
    std::vector<VkLayerProperties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    std::vector<const char*> enabled_extensions;
    std::vector<const char*> device_enabled_extensions;
    std::vector<VkQueueFamilyProperties> queue_properties;
    std::vector<VkPresentModeKHR> present_modes;

    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceMemoryProperties gpu_memory_properties;

    VkSurfaceCapabilitiesKHR surface_capabilities;

    VkAllocationCallbacks* allocation_callbacks;

    uint32_t width;
    uint32_t height;

    VkInstance instance;
    VkPhysicalDevice main_gpu;
    VkSurfaceKHR surface;
    VkDevice device;
    VkQueue graphics_queue;
    VkSwapchainKHR swapchain;

    // main framebuffer
    VkImage depth_image;
    VkImageView depth_image_view;
    std::vector<VkImage> color_images;
    std::vector<VkImageView> color_imageviews;
    VkRenderPass main_render_pass;
    VkFramebuffer main_framebuffer;

    // main command buffer pool
    VkCommandPool main_command_pool;

    uint32_t graphics_queue_family_index;
    uint32_t present_queue;

    VulkanContext() :
        allocation_callbacks(nullptr)
    {}
};

VkResult init_vulkan_context(VulkanContext& context, const char* appname, uint32_t width, uint32_t height);
void destroy_vulkan_context(VulkanContext& context);

bool app_message_loop(VulkanContext& context);

}

#endif

