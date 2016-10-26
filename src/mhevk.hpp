#ifndef __MHEVK_HPP__
#define __MHEVK_HPP__

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

#define ASSERT(condition, text) if (!(condition)) {VERIFY_PRINT(text);}

#define VULKAN_VERIFY(res, text) VERIFY(res == VK_SUCCESS, text, res)
#define VK_CHECK(x) if (x != VK_SUCCESS) VERIFY_PRINT("");
#define VK_VERIFY(x) {VkResult res = x; if (res != VK_SUCCESS) {VERIFY_PRINT(""); return res;}}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

template <class T, size_t N>
constexpr uint32_t array_size(T(&)[N])
{
    return N;
}

namespace mhe {

const float pi = 3.14f;

template <class T>
T deg_to_rad(T d)
{
    return d * (T)180 / (T)pi;
}

template <class T>
class vector2
{
public:
    T x;
    T y;

    vector2() : x(0), y(0) {}
    vector2(T newx, T newy) : x(newx), y(newy) {}
};

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

    static matrix4x4 rotation_around_y(T angle)
    {
        T s = sin(angle);
        T c = cos(angle);

        static matrix4x4 m;
        m.set(c, 0, s, 0,
            0, 1, 0, 0,
            -s, 0, c, 0,
            0, 0, 0, 1);
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
        m.set((T)2 / (r - l), 0, 0, 0,
            0, (T)2 / (t - b), 0, 0,
            -(r + l) / (r - l), -(t + b) / (t - b), -f / (n - f), 1,
            0, 0, n * f / (n - f), 0);
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

typedef vector2<float> vec2;
typedef vector3<float> vec3;
typedef vector4<float> vec4;
typedef matrix4x4<float> mat4x4;

inline bool read_entire_file(std::vector<uint8_t>& data, const char* filename, const char* mode)
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

namespace vk {

struct VulkanContext;
class Swapchain;
class CommandBuffer;
class Device;
class Mesh;

#ifdef _WIN32
struct PlatformData
{
    HMODULE hinstance;
    HWND hwnd;
};
#endif

// extend errors enum
const VkResult VK_MHE_ERROR_DATA_PROCESSING = static_cast<VkResult>(-20000001);
const VkFlags vk_all_color_components = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

static const uint32_t max_attachments = 4;

const uint32_t invalid_index = std::numeric_limits<uint32_t>::max();

uint32_t get_memory_type_index(const VkMemoryRequirements& requirements, VkFlags properties, const VkPhysicalDeviceMemoryProperties& memory_properties);

struct GPUInterface
{
    Device* device;
};

struct ApplicationInfo
{
    VkStructureType    sType;
    const void*        pNext;
    const char*        pApplicationName;
    uint32_t           applicationVersion;
    const char*        pEngineName;
    uint32_t           engineVersion;
    uint32_t           apiVersion;

    ApplicationInfo(const char* applicationName, uint32_t version, const char* engineName, uint32_t engineVersion) :
        sType(VK_STRUCTURE_TYPE_APPLICATION_INFO),
        pNext(nullptr),
        apiVersion(VK_API_VERSION_1_0),
        pApplicationName(applicationName), applicationVersion(version),
        pEngineName(engineName), engineVersion(engineVersion)
    {}

    operator const VkApplicationInfo&() const
    {
        return *reinterpret_cast<const VkApplicationInfo*>(this);
    }

    const VkApplicationInfo* c_struct() const
    {
        return reinterpret_cast<const VkApplicationInfo*>(this);
    }
};
static_assert(sizeof(ApplicationInfo) == sizeof(VkApplicationInfo), "Size of the C++ structure does not match the size of the C structure");

struct InstanceCreateInfo
{
    VkStructureType             sType;
    const void*                 pNext;
    VkInstanceCreateFlags       flags;
    const VkApplicationInfo*    pApplicationInfo;
    uint32_t                    enabledLayerCount;
    const char* const*          ppEnabledLayerNames;
    uint32_t                    enabledExtensionCount;
    const char* const*          ppEnabledExtensionNames;

    InstanceCreateInfo(const ApplicationInfo* appinfo, uint32_t enabledLayerCount, const char* const* enabledLayerNames,
        uint32_t enabledExtensionCount, const char* const* enabledExtensionNames) :
        sType(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO),
        pNext(nullptr), flags(0),
        pApplicationInfo(appinfo->c_struct()),
        enabledLayerCount(enabledLayerCount), ppEnabledLayerNames(enabledLayerNames),
        enabledExtensionCount(enabledExtensionCount), ppEnabledExtensionNames(enabledExtensionNames)
    {}

    operator const VkInstanceCreateInfo&() const
    {
        return *reinterpret_cast<const VkInstanceCreateInfo*>(this);
    }

    const VkInstanceCreateInfo* c_struct() const
    {
        return reinterpret_cast<const VkInstanceCreateInfo*>(this);
    }
};
static_assert(sizeof(InstanceCreateInfo) == sizeof(VkInstanceCreateInfo), "Size of the C++ structure does not match the size of the C structure");

struct DeviceQueueCreateInfo
{
    VkStructureType             sType;
    const void*                 pNext;
    VkDeviceQueueCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
    uint32_t                    queueCount;
    const float*                pQueuePriorities;

    DeviceQueueCreateInfo(uint32_t queueFamilyIndex, uint32_t queueCount, const float* queuePriorities) :
        sType(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO),
        pNext(nullptr),
        flags(0),
        queueFamilyIndex(queueFamilyIndex),
        queueCount(queueCount), pQueuePriorities(queuePriorities)
    {}

    const VkDeviceQueueCreateInfo *c_struct() const
    {
        return reinterpret_cast<const VkDeviceQueueCreateInfo*>(this);
    }
};
static_assert(sizeof(DeviceQueueCreateInfo) == sizeof(VkDeviceQueueCreateInfo), "Size of the C++ structure does not match the size of the C structure");

struct DeviceCreateInfo
{
    VkStructureType                    sType;
    const void*                        pNext;
    VkDeviceCreateFlags                flags;
    uint32_t                           queueCreateInfoCount;
    const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
    uint32_t                           enabledLayerCount;
    const char* const*                 ppEnabledLayerNames;
    uint32_t                           enabledExtensionCount;
    const char* const*                 ppEnabledExtensionNames;
    const VkPhysicalDeviceFeatures*    pEnabledFeatures;

    DeviceCreateInfo(uint32_t queueCreateInfoCount, DeviceQueueCreateInfo* queueCreateInfos,
        uint32_t enabledLayerCount, const char* const* enabledLayerNames,
        uint32_t enabledExtensionCount, const char* const* enabledExtensionNames,
        const VkPhysicalDeviceFeatures* enabledFeatures) :
        sType(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO),
        pNext(nullptr),
        flags(0),
        queueCreateInfoCount(queueCreateInfoCount), pQueueCreateInfos(reinterpret_cast<VkDeviceQueueCreateInfo*>(queueCreateInfos)),
        enabledLayerCount(enabledLayerCount), ppEnabledLayerNames(enabledLayerNames),
        enabledExtensionCount(enabledExtensionCount), ppEnabledExtensionNames(enabledExtensionNames),
        pEnabledFeatures(enabledFeatures)
    {}

    const VkDeviceCreateInfo* c_struct() const
    {
        return reinterpret_cast<const VkDeviceCreateInfo*>(this);
    }
};
static_assert(sizeof(DeviceCreateInfo) == sizeof(VkDeviceCreateInfo), "Size of the C++ structure does not match the size of the C structure");

struct ImageCreateInfo
{
    VkStructureType          sType;
    const void*              pNext;
    VkImageCreateFlags       flags;
    VkImageType              imageType;
    VkFormat                 format;
    VkExtent3D               extent;
    uint32_t                 mipLevels;
    uint32_t                 arrayLayers;
    VkSampleCountFlagBits    samples;
    VkImageTiling            tiling;
    VkImageUsageFlags        usage;
    VkSharingMode            sharingMode;
    uint32_t                 queueFamilyIndexCount;
    const uint32_t*          pQueueFamilyIndices;
    VkImageLayout            initialLayout;

    ImageCreateInfo(VkImageType imageType, VkFormat format, const VkExtent3D& extent,
        uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
        VkSharingMode sharingMode) :
        sType(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO),
        pNext(nullptr),
        flags(0),
        imageType(imageType), format(format),
        extent(extent),
        mipLevels(mipLevels), arrayLayers(arrayLayers),
        samples(samples),
        tiling(VK_IMAGE_TILING_OPTIMAL),
        usage(usage), sharingMode(sharingMode),
        queueFamilyIndexCount(0), pQueueFamilyIndices(nullptr),
        initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
    {}

    const VkImageCreateInfo* c_struct() const
    {
        return reinterpret_cast<const VkImageCreateInfo*>(this);
    }
};
static_assert(sizeof(ImageCreateInfo) == sizeof(VkImageCreateInfo), "Size of the C++ structure does not match the size of the C structure");

struct MemoryAllocateInfo
{
    VkStructureType    sType;
    const void*        pNext;
    VkDeviceSize       allocationSize;
    uint32_t           memoryTypeIndex;

    MemoryAllocateInfo(VkDeviceSize size, uint32_t typeIndex) :
        sType(VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO),
        pNext(nullptr),
        allocationSize(size),
        memoryTypeIndex(typeIndex)
    {}

    const VkMemoryAllocateInfo* c_struct() const
    {
        return reinterpret_cast<const VkMemoryAllocateInfo*>(this);
    }
};
static_assert(sizeof(MemoryAllocateInfo) == sizeof(VkMemoryAllocateInfo), "Size of the C++ structure does not match the size of the C structure");

struct ExtensionFunctions
{
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
};

class PhysicalDevice
{
public:
    PhysicalDevice() :
        id_(VK_NULL_HANDLE),
        graphics_queue_family_index_(invalid_index),
        present_queue_family_index_(invalid_index)
    {}

    VkResult init(VulkanContext& context, VkPhysicalDevice id);
    void destroy(VulkanContext& context);

    operator VkPhysicalDevice()
    {
        return id_;
    }

    VkPhysicalDevice id() const
    {
        return id_;
    }

    uint32_t graphics_queue_family_index() const
    {
        return graphics_queue_family_index_;
    }

    const std::vector<VkPresentModeKHR>& present_modes() const
    {
        return present_modes_;
    }

    const VkSurfaceCapabilitiesKHR& surface_capabilities() const
    {
        return surface_capabilities_;
    }

    const std::vector<VkSurfaceFormatKHR>& surface_formats() const
    {
        return surface_formats_;
    }

    uint32_t get_memory_type_index(const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags flags) const;

    const std::vector<const char*>& enabled_debug_layers() const
    {
        return enabled_device_debug_layers_extensions_;
    }
private:
    VkResult check_properties(VulkanContext& context);

    VkPhysicalDevice id_;
    VkPhysicalDeviceProperties properties_;
    VkPhysicalDeviceMemoryProperties memory_properties_;
    std::vector<VkQueueFamilyProperties> queue_properties_;
    std::vector<const char*> enabled_device_debug_layers_extensions_;
    uint32_t graphics_queue_family_index_;
    uint32_t present_queue_family_index_;
    std::vector<VkPresentModeKHR> present_modes_;
    VkSurfaceCapabilitiesKHR surface_capabilities_;
    std::vector<VkSurfaceFormatKHR> surface_formats_;
};

class Queue
{
public:
    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface, VkQueue id);
    void destroy(VulkanContext& context);

    operator VkQueue()
    {
        return id_;
    }

    VkResult submit(const CommandBuffer* command_buffers, uint32_t count,
        const VkSemaphore* wait_semaphores = nullptr, uint32_t wait_semaphores_count = 0,
        const VkSemaphore* signal_semaphores = nullptr, uint32_t signal_semaphores_count = 0);
    VkResult present(const Swapchain* swapchain);
    VkResult wait_idle();

    VkSemaphore present_semaphore() const
    {
        return present_semaphore_;
    }
private:
    VkQueue id_;
    VkSemaphore present_semaphore_;
    VkFence submit_fence_;
    GPUInterface gpu_iface_;
};

class Device
{
public:
    Device() :
        id_(VK_NULL_HANDLE)
    {}

    VkResult init(VulkanContext& context, PhysicalDevice* physical_device);
    void destroy(VulkanContext& context);

    PhysicalDevice* physical_device()
    {
        return physical_device_;
    }

    VkDevice id() const
    {
        return id_;
    }

    uint32_t get_memory_type_index(const VkMemoryRequirements& memory_requirements, VkMemoryPropertyFlags flags) const;

    operator VkDevice()
    {
        return id_;
    }

    Queue& graphics_queue()
    {
        return graphics_queue_;
    }
private:
    PhysicalDevice* physical_device_;
    VkDevice id_;
    Queue graphics_queue_;
    std::vector<const char*> device_enabled_extensions_;
};

struct ImageData
{
    uint32_t width;
    uint32_t height;
    VkFormat format;
    std::vector<uint8_t> data;
};

VkResult load_tga_image(ImageData& image_data, const char* filename);

class ImageView
{
public:
    struct Settings
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mip_levels;
        uint32_t array_layers;
        VkFormat format;
        VkImageUsageFlags usage;
        VkImageAspectFlags aspect_mask;

        Settings() :
            depth(1),
            mip_levels(1), array_layers(1), format(VK_FORMAT_R8G8B8A8_UNORM),
            usage(VK_IMAGE_USAGE_SAMPLED_BIT),
            aspect_mask(VK_IMAGE_ASPECT_COLOR_BIT)
        {}
    };

    ImageView() :
        image_(VK_NULL_HANDLE),
        imageview_(VK_NULL_HANDLE),
        memory_(VK_NULL_HANDLE)
    {}

    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings, VkImage image, const uint8_t* data, uint32_t size);
    void destroy(VulkanContext& context);

    VkImageView image_view_id() const
    {
        return imageview_;
    }

    VkImage image() const
    {
        return image_;
    }

    const GPUInterface& gpu_interface() const
    {
        return gpu_iface_;
    }

    VkFormat format() const
    {
        return settings_.format;
    }
private:
    VkImage image_;
    VkImageView imageview_;
    VkDeviceMemory memory_;
    Settings settings_;
    GPUInterface gpu_iface_;
};

class Buffer
{
public:
    struct Settings
    {
        uint32_t* queue_family_indices;
        uint32_t queue_family_indices_count;
        VkSharingMode sharing_mode;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memory_properties;

        Settings() :
            queue_family_indices(nullptr),
            queue_family_indices_count(0),
            sharing_mode(VK_SHARING_MODE_EXCLUSIVE),
            memory_properties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {}
    };

    Buffer() :
        buffer_(VK_NULL_HANDLE),
        memory_(VK_NULL_HANDLE)
    {}

    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings, const uint8_t* data, uint32_t size);
    void destroy(VulkanContext& context);
    VkResult update(VulkanContext& context, const uint8_t* data, uint32_t size);

    const VkDescriptorBufferInfo& descriptor_buffer_info() const
    {
        return desc_buffer_info_;
    }

    operator VkBuffer() const
    {
        return buffer_;
    }
private:
    VkBuffer buffer_;
    VkDescriptorBufferInfo desc_buffer_info_;
    VkDeviceMemory memory_;
    GPUInterface gpu_iface_;
    Settings settings_;
};

class RenderPass
{
public:
    struct Settings
    {
        struct AttachmentDesc
        {
            VkFormat format;
            VkImageLayout layout;
        };

        struct DependencyDesc
        {
            RenderPass* render_pass;
            VkAccessFlags src_access;
            VkAccessFlags dst_access;
        };

        AttachmentDesc* descs;
        uint32_t count;

        DependencyDesc* dependencies;
        uint32_t dependencies_count;

        Settings() :
            dependencies_count(0)
        {}
    };

    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings);
    void destroy(VulkanContext& context);

    operator VkRenderPass()
    {
        return id_;
    }
private:
    VkRenderPass id_;
    GPUInterface gpu_iface_;
};

class Framebuffer
{
public:
    struct Settings
    {
        const ImageView** attachments;
        uint32_t count;
        RenderPass* render_pass;
        uint32_t width;
        uint32_t height;
    };

    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface, const Settings& settings);
    void destroy(VulkanContext& context);

    uint32_t width() const
    {
        return settings_.width;
    }

    uint32_t height() const
    {
        return settings_.height;
    }

    RenderPass* render_pass() const
    {
        return settings_.render_pass;
    }

    operator VkFramebuffer() const
    {
        return id_;
    }
private:
    GPUInterface gpu_iface_;
    Settings settings_;
    VkFramebuffer id_;
};

class Swapchain
{
public:
    struct Settings
    {
        VkFormat format;
        VkSharingMode image_sharing_mode;

        Settings() :
            format(VK_FORMAT_B8G8R8A8_UNORM),
            image_sharing_mode(VK_SHARING_MODE_EXCLUSIVE)
        {}
    };

    Swapchain() :
        id_(VK_NULL_HANDLE),
        device_(nullptr),
        current_buffer_(0)
    {}

    VkResult init(VulkanContext& context, Device* physical_device, const Settings& settings);
    void destroy(VulkanContext& context);

    const std::vector<ImageView>& color_images() const
    {
        return color_images_;
    }

    VkResult acquire_next_image();
    uint32_t current_buffer() const
    {
        return current_buffer_;
    }

    operator VkSwapchainKHR() const
    {
        return id_;
    }

    VkSemaphore next_image_semaphore() const
    {
        return next_image_semaphore_;
    }

    VkResult create_framebuffers(VulkanContext& context, RenderPass* render_pass);

    const std::vector<Framebuffer>& framebuffers() const
    {
        return framebuffers_;
    }

    const Framebuffer& current_framebuffer() const
    {
        return framebuffers_[current_buffer_];
    }
private:
    VkResult init_images(VulkanContext& context);

    VkSwapchainKHR id_;
    Device* device_;
    Settings settings_;
    std::vector<ImageView> color_images_;
    std::vector<Framebuffer> framebuffers_;
    uint32_t current_buffer_;
    VkSemaphore next_image_semaphore_;
};

VkSamplerCreateInfo SamplerCreateInfo(VkFilter mag_filter, VkFilter min_filter, VkSamplerMipmapMode mimap_mode,
    VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v, VkSamplerAddressMode address_mode_w);
VkSemaphoreCreateInfo SemaphoreCreateInfo();

// image + view + sampler
class Texture
{
public:
    struct SamplerSettings
    {
        VkSamplerAddressMode address_mode_u;
        VkSamplerAddressMode address_mode_v;
        VkSamplerAddressMode address_mode_w;
        VkFilter mag_filter;
        VkFilter min_filter;
        VkSamplerMipmapMode mipmap_mode;

        SamplerSettings() :
            address_mode_u(VK_SAMPLER_ADDRESS_MODE_REPEAT),
            address_mode_v(VK_SAMPLER_ADDRESS_MODE_REPEAT),
            address_mode_w(VK_SAMPLER_ADDRESS_MODE_REPEAT),
            mag_filter(VK_FILTER_LINEAR),
            min_filter(VK_FILTER_LINEAR),
            mipmap_mode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
        {}
    };

    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface,
        const ImageView::Settings& image_settings, const SamplerSettings& sampler_Settings,
        const uint8_t* data, uint32_t size);
    void destroy(VulkanContext& context);

    const VkDescriptorImageInfo& descriptor_image_info() const
    {
        return descriptor_image_info_;
    }
private:
    ImageView image_view_;
    VkSampler sampler_;
    VkDescriptorImageInfo descriptor_image_info_;
};

class Command
{
public:
    virtual ~Command() {}
};

class CommandBuffer
{
public:
    VkResult init(VulkanContext& context, VkCommandBuffer id);
    void destroy(VulkanContext& context);

    operator VkCommandBuffer() const
    {
        return id_;
    }

    CommandBuffer& begin();
    void end();

    CommandBuffer& begin_render_pass_command(const Framebuffer* framebuffer, const vec4& color, float depth, uint32_t stencil,
        bool clear_color, bool clear_depth, bool clear_stencil);
    CommandBuffer& end_render_pass_command();
    CommandBuffer& set_viewport_command(const VkRect2D& rect);
    CommandBuffer& set_scissor_command(const VkRect2D& rect);
    CommandBuffer& copy_image_command(VkImage src, VkImage dst, VkImageLayout src_layout, VkImageLayout dst_layout,
        const VkImageCopy* regions, uint32_t regions_count);
    CommandBuffer& copy_buffer(VkBuffer src, VkBuffer dst, const VkBufferCopy* regions, uint32_t regions_count);
    CommandBuffer& bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point);
    CommandBuffer& bind_descriptor_set(VkPipelineBindPoint bind_point, VkPipelineLayout pipeline_layout,
        const VkDescriptorSet* descriptor_sets, uint32_t descriptor_sets_count, uint32_t first);
    CommandBuffer& draw(const Mesh& mesh, size_t part_index);
    CommandBuffer& transfer_image_layout(VkImage image, VkImageLayout src_layout, VkImageLayout dst_layout, VkImageAspectFlags aspect_flags);
    CommandBuffer& render_target_barrier(VkImage image, VkImageLayout layout, VkImageAspectFlags aspect_flags);
private:
    VkCommandBuffer id_;
};

class CommandPool
{
public:
    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface);
    void destroy(VulkanContext& context);

    operator VkCommandPool()
    {
        return id_;
    }

    VkResult create_command_buffers(VulkanContext& context, CommandBuffer* buffers, uint32_t count);
    void destroy_command_buffers(VulkanContext& context, CommandBuffer* buffers, uint32_t count);
private:
    VkCommandPool id_;
    GPUInterface gpu_iface_;
};

struct RenderPasses
{
    RenderPass main_render_pass;
};

struct CommandPools
{
    CommandPool main_graphics_command_pool;
    CommandPool resource_uploading_command_pool;
};

struct DescriptorPools
{
    VkDescriptorPool main_descriptor_pool;
};

struct DesciptorSetLayouts
{
    VkDescriptorSetLayout mesh_layout;
    VkDescriptorSetLayout material_layout;
    VkDescriptorSetLayout posteffect_layout;
};

struct VulkanContext
{
    std::vector<const char*> instance_debug_layers_extensions;
    std::vector<const char*> enabled_instance_debug_layers_extensions;

    std::vector<const char*> device_debug_layers_extensions;

    VkDebugReportCallbackEXT debug_report_callback;

    ExtensionFunctions extension_functions;

    PlatformData platform_data;
    std::vector<VkLayerProperties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    std::vector<const char*> enabled_extensions;

    VkSurfaceKHR surface;

    VkAllocationCallbacks* allocation_callbacks;

    uint32_t width;
    uint32_t height;

    VkInstance instance;
    std::vector<PhysicalDevice> gpus;
    PhysicalDevice* main_gpu;
    std::vector<Device> devices;
    Device* main_device;
    Swapchain main_swapchain;

    GPUInterface default_gpu_interface;

    ImageView main_depth_stencil_image_view;

    RenderPasses render_passes;
    CommandPools command_pools;
    DescriptorPools descriptor_pools;

    DesciptorSetLayouts descriptor_set_layouts;

    VkPipelineCache main_pipeline_cache;

    VulkanContext() :
        allocation_callbacks(nullptr)
    {}
};

class GeometryLayout
{
public:
    struct Vertex
    {
        vec3 pos;
        vec3 nrm;
        vec3 tng;
        vec2 tex;
    };

    static uint32_t stride()
    {
        return sizeof(Vertex);
    }

    static void vertex_input_info(VkPipelineVertexInputStateCreateInfo& info);
};

class FullscreenLayout
{
public:
    struct Vertex
    {
        vec4 pos;
        vec2 tex;
    };

    static uint32_t stride()
    {
        return sizeof(Vertex);
    }

    static void vertex_input_info(VkPipelineVertexInputStateCreateInfo& info);
};

class Material
{
    enum
    {
        albedo_texture = 0,
        max_texture_index
    };
public:
    VkResult init(VulkanContext& context, const GPUInterface& gpu_iface);
    void destroy(VulkanContext& context);

    void set_albedo(Texture* texture)
    {
        textures_[albedo_texture] = texture;
        build_descriptor_set();
    }

    VkDescriptorSet descriptor_set() const
    {
        return descriptor_set_;
    }
private:
    void build_descriptor_set();

    Texture* textures_[max_texture_index];
    Buffer uniform_;
    VkDescriptorSet descriptor_set_;
    GPUInterface gpu_iface_;
};

struct MeshPart
{
    uint32_t ibuffer_offset;
    uint32_t vbuffer_offset;
    uint32_t indices_count;
    Material* material;
};

class Mesh
{
public:
    Mesh() :
        descriptor_set_(VK_NULL_HANDLE)
    {}

    VkResult create_cube(vk::VulkanContext& context, const vk::GPUInterface& gpu_iface);
    VkResult create_quad(vk::VulkanContext& context, const vk::GPUInterface& gpu_iface);
    void destroy(vk::VulkanContext& context);

    const std::vector<MeshPart>& parts() const
    {
        return parts_;
    }

    const vk::Buffer& vbuffer() const
    {
        return vbuffer_;
    }

    const vk::Buffer& ibuffer() const
    {
        return ibuffer_;
    }

    VkDescriptorSet descriptor_set() const
    {
        return descriptor_set_;
    }

    void set_material(size_t index, Material* material)
    {
        parts_[index].material = material;
    }
private:
    std::vector<MeshPart> parts_;
    vk::Buffer vbuffer_;
    vk::Buffer ibuffer_;
    vk::Buffer uniform_;
    VkDescriptorSet descriptor_set_;
};

VkResult init_vulkan_context(VulkanContext& context, const char* appname, uint32_t width, uint32_t height, bool enable_default_debug_layers);
void destroy_vulkan_context(VulkanContext& context);

bool app_message_loop(VulkanContext& context);

void init_fullscreen_viewport(VkViewport& viewport, const VulkanContext& context);

}
}

#endif
