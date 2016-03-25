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
#define VERIFY_PRINT(text) {printf("%s %d %s\n", __FUNCTION__, __LINE__, text); assert(0);}
#else
#define VERIFY_PRINT(text) {printf("%s", text);}
#endif

#define VERIFY(condition, text, result) if (!(condition)) {VERIFY_PRINT(text); return result;}

#define VULKAN_VERIFY(res, text) VERIFY(res == VK_SUCCESS, text, res)

#define ASSERT(condition, text) if (!(condition)) {VERIFY_PRINT(text);}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

namespace mhe {

template <class T>
class vector3
{
public:
    T x;
    T y;
    T z;

    vector3() : x(0), y(0), z(0) {}
    vector3(T newx, T newy, T newz) : x(newx), y(newy), z(newz) {}
};

template <class T>
class vector4
{
public:
    union
    {
        struct
        {
            T x, y, z, w;
        };
        
        struct
        {
            T c[4];
        };
    };

    vector4() : x(0), y(0), z(0), w(0) {}
    vector4(T newx, T newy, T newz, T neww) : x(newx), y(newy), z(newz), w(neww) {}
};

template <class T>
class matrix4x4
{
public:
    matrix4x4()
    {
        load_identity();
    }

    void load_identity()
    {
        row0_.x = 1;
        row1_.y = 1;
        row2_.z = 1;
        row3_.w = 1;
    }

    void set_uniform_scale(T s)
    {
        row0_.x = s;
        row1_.y = s;
        row2_.z = s;
    }

    static matrix4x4 identity()
    {
        static matrix4x4 m;
        return m;
    }

    static matrix4x4 scaling(T s)
    {
        static matrix4x4 m;
        m.set_uniform_scale(s);
        return m;
    }
private:
    vector4<T> row0_, row1_, row2_, row3_;
};

typedef vector3<float> vec3;
typedef vector4<float> vec4;
typedef matrix4x4<float> mat4x4;

struct VulkanContext;

#ifdef _WIN32
struct PlatformData
{
    HMODULE hinstance;
    HWND hwnd;
};
#endif

const VkFlags vk_all_color_components = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

uint32_t get_memory_type_index(const VkMemoryRequirements& requirements, VkFlags properties, const VkPhysicalDeviceMemoryProperties& memory_properties);

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
void init_defaults(VkSemaphoreCreateInfo& create_info);
void init_defaults(VkGraphicsPipelineCreateInfo& create_info);
void init_defaults(VkPipelineVertexInputStateCreateInfo& create_info);
void init_defaults(VkPipelineInputAssemblyStateCreateInfo& create_info);
void init_defaults(VkPipelineRasterizationStateCreateInfo& create_info);
void init_defaults(VkPipelineDepthStencilStateCreateInfo& create_info);
void init_defaults(VkPipelineColorBlendStateCreateInfo& create_info);
void init_defaults(VkPipelineColorBlendAttachmentState& state);
void init_defaults(VkPipelineMultisampleStateCreateInfo& create_info);
void init_defaults(VkPipelineDynamicStateCreateInfo& create_info);
void init_defaults(VkPipelineLayoutCreateInfo& create_info);
void init_defaults(VkDescriptorSetLayoutCreateInfo& create_info);
void init_defaults(VkShaderModuleCreateInfo& create_info);
void init_defaults(VkPipelineShaderStageCreateInfo& create_info);
void init_defaults(VkPipelineShaderStageCreateInfo& create_info);
void init_defaults(VkPipelineCacheCreateInfo& create_info);
void init_defaults(VkPipelineViewportStateCreateInfo& create_info);
void init_defaults(VkBufferCreateInfo& create_info);
void init_defaults(VkMemoryAllocateInfo& allocate_info);
void init_defaults(VkDescriptorSetAllocateInfo& allocate_info);
void init_defaults(VkDescriptorPoolCreateInfo& create_info);
void init_defaults(VkWriteDescriptorSet& write_descriptor_set);

VkResult create_depth_image_view(VkImageView& imageview, VkImage& image, uint32_t width, uint32_t height, const VulkanContext& vulkan_context);
void destroy_image_view(VkImageView& image_view, VkImage& vk_image, const VulkanContext& vulkan_context);

VkResult create_shader_module(VkShaderModule& shader, const VulkanContext& context, const uint8_t* text, uint32_t size);
void destroy_shader_module(VkShaderModule& shader, const VulkanContext& context);

bool read_whole_file(std::vector<uint8_t>& data, const char* filename, const char* mode = "rb");

VkResult load_shader_module(VkShaderModule& shader, const VulkanContext& context, const char* filename);

struct Buffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

VkResult create_static_buffer(Buffer& buffer, const VulkanContext& context, const uint8_t* data, uint32_t size, VkBufferUsageFlags usage);
void destroy_buffer(Buffer& buffer, const VulkanContext& context);
VkResult create_dynamic_buffer(Buffer& buffer, const VulkanContext& context, const uint8_t* data, uint32_t size, VkBufferUsageFlags usage);

struct ExtensionFunctions
{
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
};

struct VulkanContext
{
    std::vector<const char*> instance_debug_layers_extensions;
    std::vector<const char*> enabled_instance_debug_layers_extensions;

    std::vector<const char*> device_debug_layers_extensions;
    std::vector<const char*> enabled_device_debug_layers_extensions;

    VkDebugReportCallbackEXT debug_report_callback;

    ExtensionFunctions extension_functions;

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

    VkSemaphore present_semaphore;

    VkPipelineCache pipeline_cache;

    // main command buffer pool
    VkCommandPool main_command_pool;

    uint32_t graphics_queue_family_index;
    uint32_t present_queue;

    VulkanContext() :
        allocation_callbacks(nullptr)
    {}
};

VkResult init_vulkan_context(VulkanContext& context, const char* appname, uint32_t width, uint32_t height, bool enable_default_debug_layers);
void destroy_vulkan_context(VulkanContext& context);

bool app_message_loop(VulkanContext& context);

}

#endif

