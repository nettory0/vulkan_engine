#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "vk_base.h"
#include "vk_struct_builders.h"

class CGlWindow;
struct SSwapChain;
class CSceneObject;

namespace vk_structs
{

struct VULKAN_STRUCT SSwapchainSupportInfo
{
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;
};

struct VULKAN_STRUCT SSwapchainConfig
{
    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;
};

struct VULKAN_STRUCT SSupportDetails
{
    SSwapchainSupportInfo m_swapChainInfo;
    SSwapchainConfig m_swapChainConfig;

    VkFormat m_supportedFormat;
    VkPhysicalDeviceMemoryProperties m_memProperties;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    void init(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    void findSupportedFormat(VkPhysicalDevice device);
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const;
};

struct VULKAN_STRUCT SDescriptorSet
{
public:

    void create(const VkDevice& device, VkDescriptorSetAllocateInfo& allocInfo, std::vector<VkWriteDescriptorSet>& aDescriptorWrites);
    void destroy();
    VkDescriptorSet handle() {return m_descriptorSet.m_set;};
private:
    vk_base::CDescriptorSetBase m_descriptorSet;
};

bool VULKAN_STRUCT checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
bool VULKAN_STRUCT isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);

struct VULKAN_STRUCT SQueueFamilyIndices
{
    std::optional<uint32_t> m_graphicsFamily;
    std::optional<uint32_t> m_presentFamily;
    std::optional<uint32_t> m_computeFamily;
    void findQueueFamilies(VkPhysicalDevice device, const VkSurfaceKHR& surface);
    bool isComplete();
};

class VULKAN_STRUCT CBuffer
{
public:
    CBuffer() = default;
    CBuffer(const CBuffer&) = delete;
    CBuffer& operator=(const CBuffer&) = delete;
    CBuffer(CBuffer&&) noexcept;
    CBuffer& operator=(CBuffer&&) noexcept;
    ~CBuffer();
    void create(const VkDevice& device, const SSupportDetails& details, const VkBufferCreateInfo& bufferInfo, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo, VkMemoryPropertyFlags properties);
    void clean();
    void bindBufferMemory(const VkDevice& device);
    void load(size_t size, const void* pData, size_t offset);
    void read(size_t size, void* pData);
    VkDeviceAddress adress() const;
    VkDevice device()
    {
        return m_device;
    };
    VkBuffer& handle()
    {
        return m_buffer.handle();
    };
    const VkBuffer& handle() const
    {
        return m_buffer.handle();
    };
    // Memory is suballocated by VMA: the buffer lives at memoryOffset() inside memory().
    VkDeviceMemory memory() const
    {
        return m_bufferMemory.deviceMemory();
    };
    VkDeviceSize memoryOffset() const
    {
        return m_bufferMemory.offset();
    };
    VkDeviceSize size() const
    {
        return m_bufferSize;
    };
private:
    vk_base::CBufferBase m_buffer;
    vk_base::CMemoryBuffer m_bufferMemory;
    VkDeviceAddress m_adress{0};
    VkDevice m_device{VK_NULL_HANDLE};
    VkDeviceSize m_bufferSize{0};
};

class VULKAN_STRUCT CPhysicalDevice
{
public:
    void init(VkPhysicalDevice device);
    const VkPhysicalDeviceProperties& physicalDeviceProperties();
    void physicalDeviceProperties2(VkPhysicalDeviceProperties2& physicalDeviceProperties);
    static std::vector<VkPhysicalDevice> pickPhysicalDevices(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions);
    const VkPhysicalDevice& handle() const;
    VkSampleCountFlagBits getMaxUsableSampleCount();
private:
    VkPhysicalDevice m_handle{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
};

class VULKAN_STRUCT CLogicalDevice
{
public:

    CLogicalDevice() = default;

    CLogicalDevice(const CLogicalDevice&) = delete;
    CLogicalDevice& operator=(const CLogicalDevice&) = delete;

    CLogicalDevice(CLogicalDevice&&) noexcept = default;
    CLogicalDevice& operator=(CLogicalDevice&&) noexcept = default;

    const VkDevice& handle();
    void destroy();
    void waitIdle();
    void create(
        VkPhysicalDevice physicalDevice,
        const std::vector<const char*>& validationLayers,
        const SQueueFamilyIndices& indices,
        const std::vector<const char*>& deviceExtensions,
        bool enableRayTracing);
public:
    vk_base::CLogicalDeviceBase m_device;
};

class VULKAN_STRUCT CDescriptorSetLayout
{
public:
    CDescriptorSetLayout(const CDescriptorSetLayout&) = delete;
    CDescriptorSetLayout& operator=(const CDescriptorSetLayout&) = delete;
    CDescriptorSetLayout() = default;

    std::vector <VkDescriptorSetLayout> getAll();
    void create(const VkDevice& device, const std::vector<VkDescriptorSetLayoutCreateInfo>& info);
    void destroy();
    const std::vector<vk_base::CDescriptorSetLayoutBase>& setLayout() const
    {
        return m_aSetLayout;
    }
private:
    std::vector<vk_base::CDescriptorSetLayoutBase> m_aSetLayout;
};

class VULKAN_STRUCT CPipelineLayout
{
public:
    CPipelineLayout(const CPipelineLayout&) = delete;
    CPipelineLayout& operator=(const CPipelineLayout&) = delete;
    CPipelineLayout() = default;
    void create(const VkDevice& device, const std::vector<VkDescriptorSetLayoutCreateInfo>& info, const std::vector<VkPushConstantRange>& aPushConstantRange);
    void destroy();
    VkPipelineLayout layout() const;
    const CDescriptorSetLayout& descriptorSet() const;
private:
    vk_base::CPipelineLayoutBase m_layout;
    CDescriptorSetLayout m_descriptorSetLayout;
};

struct VULKAN_STRUCT SSwapchainImageView
{
    void destroy();
    vk_base::CImageView m_imageView;
    VkImage m_swapChainImage;
};

class VULKAN_STRUCT CSemaphoreFence
{
public:
    CSemaphoreFence(const CSemaphoreFence&) = delete;
    CSemaphoreFence& operator=(const CSemaphoreFence&) = delete;
    CSemaphoreFence(CSemaphoreFence&&) noexcept = default;
    CSemaphoreFence& operator=(CSemaphoreFence&&) noexcept = default;
    CSemaphoreFence() = default;
    void destroy();
    void create(const VkDevice& device);
    VkResult waitForFence();
    const VkSemaphore& availableSemaphore() const;
    const VkSemaphore& renderfinishedSemaphore() const;
    const VkFence& fence() const;
    void resetFence();
private:
    vk_base::CSemaphore m_imageAvailableSemaphores;
    vk_base::CSemaphore m_renderFinishedSemaphores;
    vk_base::CFence m_fence;
};

class VULKAN_STRUCT CSemaphoreFence2
{
public:
    CSemaphoreFence2(const CSemaphoreFence2&) = delete;
    CSemaphoreFence2& operator=(const CSemaphoreFence2&) = delete;
    CSemaphoreFence2(CSemaphoreFence2&&) noexcept = default;
    CSemaphoreFence2& operator=(CSemaphoreFence2&&) noexcept = default;
    CSemaphoreFence2() = default;
    void destroy();
    void create(const VkDevice& device);
    VkResult waitForFence();
    const VkSemaphore& availableSemaphore() const;
    const VkFence& fence() const;
    void resetFence();
private:
    vk_base::CSemaphore m_imageAvailableSemaphores;
    vk_base::CFence m_fence;
};

class VULKAN_STRUCT CCommandPool
{
public:
    void create(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t  queueFamilyIndices, const int size);
    void free();
    void destroy();
    void createCommandBuffers(const int size);
    vk_base::CCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk_base::CCommandBuffer& commandBuffer, vk_base::CQueue& queue);
    void freeSingleTimeCommand(vk_base::CCommandBuffer& commandBuffer);
public:
    vk_base::CCommandPoolBase m_pool;
    std::vector<vk_base::CCommandBuffer> m_commandBuffers;

};

class VULKAN_STRUCT CImage
{
public:
    void createResources(const VkDevice& device, const SSupportDetails& details, VkSampleCountFlagBits msaaSamples, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags, VkFormat format);
    void clean();
    void bind(const VkDevice& device);
    void createImage(const VkDevice& device, const SSupportDetails& details, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void create(const VkDevice& device, const SSupportDetails& details, const VkImageCreateInfo& rayTraceImageCreateInfo, const void* memoryAllocateFlagsInfo, VkMemoryPropertyFlags properties);

    void createImageView(const VkDevice& device, VkImageViewCreateInfo& viewInfo);
    const VkImage& handle() const;
    VkDeviceMemory memory() const;
    const VkImageView& view() const;
    static VkImageViewCreateInfo defaultcreateCreateInfo(VkImage image, const VkDevice& device, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
private:
    vk_base::CImageBase m_image;
    vk_base::CMemoryBuffer m_imageMemory;
    vk_base::CImageView m_imageView;
};

class VULKAN_STRUCT CBlas
{
public:
    VkAccelerationStructureKHR m_accelerationStructureHandle{VK_NULL_HANDLE};
    VkDeviceAddress m_accelerationStructureDeviceAddress;

    CBuffer m_asBuffer;

    void clean();
};

class VULKAN_STRUCT CTlas
{
public:
    VkAccelerationStructureKHR m_accelerationStructureHandle{VK_NULL_HANDLE};
    VkDeviceAddress m_accelerationStructureDeviceAddress;

    CBuffer m_instanceBuffer;
    CBuffer m_asBuffer;

    void clean();
};
void VULKAN_STRUCT transitionImageLayout(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    uint32_t mipLevels,
    VkImageAspectFlags aspectMask,
    CCommandPool& commandPool,
    vk_base::CQueue& queue);

class VULKAN_STRUCT CTexture
{
public:
    CTexture(CTexture&& other) = default;
    CTexture() = default;
    CTexture& operator=(CTexture&& other) = default;
    ~CTexture();
    void createTextureImage(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const std::vector<unsigned char>& pixels, int texWidth, int texHeight, int texChannels, CCommandPool& commandPool, vk_base::CQueue& queue, const SSupportDetails& details);
    void transitionImageLayout(const VkDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, CCommandPool& commandPool, vk_base::CQueue& queue);
    void clean();
    void copyBufferToImage(const VkDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, CCommandPool& commandPool, vk_base::CQueue& queue);
    void generateMipmaps(const VkDevice& device, const VkPhysicalDevice& physicalDevice, CCommandPool& commandPool, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, vk_base::CQueue& queue);
public:
    CImage m_texture;
    vk_base::CSampler m_textureSampler;
};

class VULKAN_STRUCT CUniformBase
{
public:
    CUniformBase(VkDescriptorType descriptorType, int destinationBinding);
    virtual ~CUniformBase() = default;
    virtual void destroy();
    virtual VkWriteDescriptorSet writeDescriptorSet() = 0;
protected:
    VkDescriptorType m_descriptorType;
    int m_destinationBinding;
};

class VULKAN_STRUCT CUniformBuffer: public CUniformBase
{
public:
    CUniformBuffer(VkDescriptorType descriptorType, int destinationBinding);
    CUniformBuffer& operator=(CUniformBuffer&& other) = default;
    ~CUniformBuffer();
    void destroy() override;
    void init(uint32_t bufferId, const CBuffer& resourceBuffer);
    VkWriteDescriptorSet writeDescriptorSet() override;
public:
    VkDescriptorBufferInfo m_descriptorInfo;
    uint32_t m_bufferId;
};

class VULKAN_STRUCT CUniformTexture: public CUniformBase
{
public:
    CUniformTexture(VkDescriptorType descriptorType, int destinationBinding);
    CUniformTexture& operator=(CUniformTexture&& other) = default;
    ~CUniformTexture();
    void destroy() override;
    void init(uint32_t bufferId, VkImageLayout imageLayout, const CTexture& resourceTexture);
    VkWriteDescriptorSet writeDescriptorSet() override;

public:
    uint32_t m_textureId;
    VkDescriptorImageInfo m_descriptorInfo;
};

class VULKAN_STRUCT CUniformTlas: public CUniformBase
{
public:
    CUniformTlas(VkDescriptorType descriptorType, int destinationBinding);
    void destroy() override;
    VkWriteDescriptorSet writeDescriptorSet() override;
    ~CUniformTlas();
public:
    CTlas m_tlas;
private:
    VkWriteDescriptorSetAccelerationStructureKHR m_asDescriptorInfo;
};

class VULKAN_STRUCT CUniformBlas: public CUniformBase
{
public:
    CUniformBlas(VkDescriptorType descriptorType, int destinationBinding);
    void destroy() override;
    VkWriteDescriptorSet writeDescriptorSet() override;
    ~CUniformBlas();
public:
    CBlas m_blas;
};

class VULKAN_STRUCT CDescriptorBufferResource
{
public:
    void create(VkPhysicalDevice physicalDevice, VkDevice device, VkDescriptorSetLayout layout, const std::vector<VkWriteDescriptorSet>& writes, const SSupportDetails& details);
    void destroy();
    VkDescriptorBufferBindingInfoEXT descriptorBufferBindingInfo()
    {
        return m_bindingInfo;
    }
private:
    CBuffer m_buffer;
    VkDescriptorBufferBindingInfoEXT m_bindingInfo;
};

struct VULKAN_STRUCT SRenderTargets
{
    std::vector<SSwapchainImageView> m_framebuffersImageViews;
    CImage m_color;
    CImage m_depth;
    void create(const VkDevice& device, VkSampleCountFlagBits msaaSamples, const std::vector<VkImage>& images, const SSupportDetails& details);
    void createRT(const VkDevice& device, VkSampleCountFlagBits msaaSamples, const std::vector<VkImage>& images, const SSupportDetails& details);
    void destroy();
};

struct SDescriptorSetLayoutInfo
{
    VkDescriptorSetLayout m_layout;
    std::map<uint32_t, VkDescriptorType> m_mapDescriptorType;
};

class VULKAN_STRUCT CPipelineLayoutBuilder
{
public:
    void addPushConstant(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);
    void setLayoutCreateInfo();
    void addDescriptorSetLayoutBinding(int index,
        uint32_t binding,
        VkDescriptorType descriptorType,
        uint32_t descriptorCount,
        VkShaderStageFlags stageFlags,
        const VkSampler* pImmutableSamplers);
    void addDescriptorSetLayoutBinding(int index, VkDescriptorSetLayoutBinding descriptorSetLayoutBinding);
public:
    std::vector<VkDescriptorSetLayoutCreateInfo> m_aLayoutInfo;
    std::vector<VkPushConstantRange> m_aPushConstant;
    std::map<int, std::vector<VkDescriptorSetLayoutBinding>> m_aBinding;
    std::map<uint32_t, SDescriptorSetLayoutInfo> m_mapDescriptorType;
};
class VULKAN_STRUCT CPipelineBuilder
{
public:
    CPipelineBuilder(const CPipelineBuilder&) = delete;
    CPipelineBuilder& operator=(const CPipelineBuilder&) = delete;
    CPipelineBuilder(CPipelineBuilder&&) noexcept = default;
    CPipelineBuilder& operator=(CPipelineBuilder&&) noexcept = default;
    CPipelineBuilder(VkDevice m_device);
    ~CPipelineBuilder();
    void set(const SSupportDetails& details, VkSampleCountFlagBits msaa, VkPrimitiveTopology topoType, bool bBlending);
    VkComputePipelineCreateInfo initPipelineCreateInfoCompute();
    VkGraphicsPipelineCreateInfo initPipelineCreateInfo(VkPipelineRenderingCreateInfo* renderingCreateInfo);
    VkRayTracingPipelineCreateInfoKHR initPipelineCreateInfoRT();
    void setImputAssambly(VkPipelineInputAssemblyStateCreateFlags flags, VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable);
    void addAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);
    void addAttributeDescription(VkVertexInputAttributeDescription vertexInputAttributeDescription);
    void setBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);
    void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
    void setDepthStencilState(VkPipelineDepthStencilStateCreateFlags flags,
        VkBool32 depthTestEnable,
        VkBool32 depthWriteEnable,
        VkCompareOp depthCompareOp,
        VkBool32 depthBoundsTestEnable,
        VkBool32 stencilTestEnable,
        VkStencilOpState front,
        VkStencilOpState back,
        float minDepthBounds,
        float maxDepthBounds);
    void setViewportState(float x, float y, float width, float height, float minDepth, float maxDepth, VkOffset2D offset, VkExtent2D extent);
    void setColorBlending(bool bOn);
    void setMultisampling(VkSampleCountFlagBits msaa);
    void addShaderStageCreateInfo(VkPipelineShaderStageCreateFlags flags, VkShaderStageFlagBits stage, std::string module, const char* pName, const VkSpecializationInfo* pSpecializationInfo);
    void setLayout(VkPipelineLayout layout);
    void setRayTracingShaderGroupCreateInfoList(std::vector<VkRayTracingShaderGroupCreateInfoKHR>& group);
    void setRenderPass(VkRenderPass renderpass);

private:
    VkViewport m_viewport;
    VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
    VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
    VkPipelineRasterizationStateCreateInfo m_rasterizer;
    VkVertexInputBindingDescription m_bindingDescription;
    std::vector<VkVertexInputAttributeDescription> m_aAttributeDescription;
    VkPipelineDepthStencilStateCreateInfo m_depthStencil;
    VkPipelineViewportStateCreateInfo m_viewportState;
    VkRect2D m_scissor;
    VkPipelineMultisampleStateCreateInfo m_multisampling;
    VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo m_colorBlending;
    float f[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    VkRenderPass m_renderPass;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
    VkPipelineLayout m_layout;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_rayTracingShaderGroupCreateInfoList;
    std::vector<std::unique_ptr<vk_base::CShaderModule>> m_aShaderStages;
    VkDevice m_device;

};

struct VULKAN_STRUCT SPipeline
{
public:
    SPipeline(const SPipeline&) = delete;
    SPipeline& operator=(const SPipeline&) = delete;
    SPipeline() = default;

    void destroy();
    void create(const VkDevice& device, const VkGraphicsPipelineCreateInfo& config);
    void createCompute(const VkDevice& device, const VkComputePipelineCreateInfo& config);
    void createRayTracing(const VkDevice& device, const VkRayTracingPipelineCreateInfoKHR& rayTracingPipelineCreateInfo);
public:
    vk_base::CPipelineBase m_pipeline;
};

struct VULKAN_STRUCT SFrameSyncPool
{
public:
    SFrameSyncPool(const SFrameSyncPool&) = delete;
    SFrameSyncPool& operator=(const SFrameSyncPool&) = delete;

    SFrameSyncPool() = default;

    void createRenderFences(const VkDevice& device, int size);
    void cleanRenderFences();
    void create(const VkDevice& device, int sizeRenderFences);
    void destroy();
public:
    std::vector<CSemaphoreFence> m_imageRenderFences;
};

struct VULKAN_STRUCT SFrameSyncPool2
{
public:
    SFrameSyncPool2(const SFrameSyncPool2&) = delete;
    SFrameSyncPool2& operator=(const SFrameSyncPool2&) = delete;

    SFrameSyncPool2() = default;

    void createRenderFences(const VkDevice& device, int size);
    void cleanRenderFences();
    void create(const VkDevice& device, int sizeRenderFences);
    void destroy();

public:
    const int m_framesInFlight{2};
    std::vector<vk_base::CSemaphore> m_renderFinishedSemaphores;
    std::vector<CSemaphoreFence2> m_imageRenderFences;
};


struct VULKAN_STRUCT SSwapchain
{
public:
    void createSwapChain(const VkDevice& device, const VkSurfaceKHR& surface, const SSupportDetails& swapChainSupport, const SQueueFamilyIndices& indices, bool enableTransferDst = false);
    void create(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    void destroy();
public:
    vk_base::CSwapchainBase m_swapchain;
};

struct VULKAN_STRUCT SInstance
{
public:
    void create(const std::vector<const char*>& aExtantion, const std::vector<const char*>& validationLayers, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
    void destroy();
    void setupDebugMessenger(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
    void destroyDebugMessenger();
    static void initVolkExternal();
    static void initVolkLoadInstanceOnlyExternal(VkInstance instance);
public:
    vk_base::CInstanceBase m_instance;
    vk_base::CDebugMessenger m_debugMessenger;
};

}