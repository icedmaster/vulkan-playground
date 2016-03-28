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

const float pi = 3.14f;

template <class T>
T deg_to_rad(T d)
{
    return d * (T)180 / (T)pi;
}

template <class T>
class vector3
{
public:
    T x;
    T y;
    T z;

    vector3() : x(0), y(0), z(0) {}
    vector3(T newx, T newy, T newz) : x(newx), y(newy), z(newz) {}

    void normalize()
    {
        T m = magnitude();
        x /= m;
        y /= m;
        z /= m;
    }

    T magnitude() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    vector3 operator- (const vector3& other) const
    {
        return vector3(x - other.x, y - other.y, z - other.z);
    }

    static vector3 up()
    {
        static vector3 v(0, 1, 0);
        return v;
    }
};

template <class T>
T dot(const vector3<T>& v1, const vector3<T>& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <class T>
vector3<T> cross(const vector3<T>& v1, const vector3<T>& v2)
{
    return vector3<T>(v1.y * v2.z - v2.y * v1.z,
                      v2.x * v1.z - v1.x * v2.z,
                      v1.x * v2.y - v2.x * v1.y);
}

template <class T>
vector3<T> operator- (const vector3<T>& v)
{
    return vector3<T>(-v.x, -v.y, -v.z);
}

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

    void set(T newx, T newy, T newz, T neww)
    {
        x = newx;
        y = newy;
        z = newz;
        w = neww;
    }

    static vector4 zero()
    {
        static vector4 v;
        return v;
    }
};

template <class T>
T dot(const vector4<T>& v1, const vector4<T>& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

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
        set(1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
    }

    void set(T a00, T a01, T a02, T a03,
             T a10, T a11, T a12, T a13,
             T a20, T a21, T a22, T a23,
             T a30, T a31, T a32, T a33)
    {
        m_[0][0] = a00; m_[0][1] = a01; m_[0][2] = a02, m_[0][3] = a03;
        m_[1][0] = a10; m_[1][1] = a11; m_[1][2] = a12, m_[1][3] = a13;
        m_[2][0] = a20; m_[2][1] = a21; m_[2][2] = a22, m_[2][3] = a23;
        m_[3][0] = a30; m_[3][1] = a31; m_[3][2] = a32, m_[3][3] = a33;
    }

    T* data()
    {
        return reinterpret_cast<T*>(m_);
    }

    const T* data() const
    {
        return reinterpret_cast<const T*>(m_);
    }

    T operator() (size_t i, size_t j) const
    {
        ASSERT(i < 4 && j < 4, "Invalid index");
        return m_[i][j];
    }

    matrix4x4& operator= (const matrix4x4& other)
    {
        if (this == &other)
            return *this;
        memcpy(m_, other.data(), sizeof(T) * 16);
        return *this;
    }

    matrix4x4 operator* (const matrix4x4& other) const
    {
#define MUL(i, j) (m_[i][0] * other(0, j) + m_[i][1] * other(1, j) + m_[i][2] * other(2, j) + m_[i][3] * other(3, j))
        matrix4x4 res;
        res.set(MUL(0, 0), MUL(0, 1), MUL(0, 2), MUL(0, 3),
                MUL(1, 0), MUL(1, 1), MUL(1, 2), MUL(1, 3),
                MUL(2, 0), MUL(2, 1), MUL(2, 2), MUL(2, 3),
                MUL(3, 0), MUL(3, 1), MUL(3, 2), MUL(3, 3));
        return res;
    }

    vector4<T> row(size_t index) const
    {
        ASSERT(index < 4, "Invalid index");
        return vector4<T>(m_[index][0], m_[index][1], m_[index][2], m_[index][3]);
    }

    void transpose()
    {
        T t = m_[0][1];
        m_[0][1] = m_[1][0];
        m_[1][0] = t;

        t = m_[0][2];
        m_[0][2] = m_[2][0];
        m_[2][0] = t;

        t = m_[0][3];
        m_[0][3] = m_[3][0];
        m_[3][0] = t;

        t = m_[1][2];
        m_[1][2] = m_[2][1];
        m_[2][1] = t;

        t = m_[1][3];
        m_[1][3] = m_[3][1];
        m_[3][1] = t;

        t = m_[2][3];
        m_[2][3] = m_[3][2];
        m_[3][2] = t;
    }

    matrix4x4 transposed() const
    {
        matrix4x4 res = *this;
        res.transpose();
        return res;
    }

    static matrix4x4 identity()
    {
        static matrix4x4 m;
        return m;
    }

    static matrix4x4 scaling(T s)
    {
        static matrix4x4 m;
        m.set(s, 0, 0, 0,
              0, s, 0, 0,
              0, 0, s, 0,
              0, 0, 0, 1);
        return m;
    }

    static matrix4x4 translation(const vector3<T>& v)
    {
        static matrix4x4 m;
        m.set(0, 0, 0, 0,
              0, 0, 0, 0,
              0, 0, 0, 0,
              v.x, v.y, v.z, 1);
        return m;
    }

    static matrix4x4 look_at(const vector3<T>& pos, const vector3<T>& dir, const vector3<T>& up)
    {
        static matrix4x4 m;
        vector3<T> f = dir - pos;
        f.normalize();
        vector3<T> s = cross(f, up);
        s.normalize();
        vector3<T> u = cross(s, f);
        u.normalize();
        m.set(s.x, -u.x, f.x, 0,
              s.y, -u.y, f.y, 0,
              s.z, -u.z, f.z, 0,
              -dot(pos, s), -dot(pos, u), -dot(pos, f), 1);
        return m;
    }

    static matrix4x4 frustum(T l, T r, T t, T b, T n, T f)
    {
        static matrix4x4 m;
        // suppose that we have depth mapped to [0..1] interval
        m.set((T)2 / (r - l),      0,                   0,               0,
              0,                   (T)2 / (t - b),      0,               0,
              -(r + l) / (r - l),  -(t + b) / (t - b),  -f / (n - f),    1,
              0,                   0,                   n * f / (n - f), 0);
        return m;
    }

    static matrix4x4 perspective(T fov, T aspect, T znear, T zfar)
    {
        T fov_tan = (T)tan(fov * 0.5f);
        T right = fov_tan * aspect * znear;
        T left = -right;
        T top = fov_tan * znear;
        T bottom = -top;
        return frustum(left, right, top, bottom, znear, zfar);
    }
private:
    T m_[4][4];
};

template <class T>
matrix4x4<T> transpose(const matrix4x4<T>& m)
{
    return m.transposed();
}

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

