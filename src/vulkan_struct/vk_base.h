#pragma once
#include <vector>
#include "vk_export.h"
#include "vk_object_template.h"

namespace vk_base
{

class VULKAN_STRUCT CInstanceBase: public CVulkanObject<VkInstance, std::monostate, VkInstanceCreateInfo>
{
public:
	CInstanceBase() = default;
	CInstanceBase(const CInstanceBase&) = delete;
	CInstanceBase& operator=(const CInstanceBase&) = delete;
	CInstanceBase(CInstanceBase&&) noexcept = default;
	CInstanceBase& operator=(CInstanceBase&&) noexcept = default;
	void create(const VkInstanceCreateInfo& createInfo);
	void destroy();
	~CInstanceBase();
};

class VULKAN_STRUCT CLogicalDeviceBase: public vk_base::CVkDeviceBase<VkDevice>
{
public:
	CLogicalDeviceBase() = default;
	CLogicalDeviceBase(const CLogicalDeviceBase&) = delete;
	CLogicalDeviceBase& operator=(const CLogicalDeviceBase&) = delete;
	CLogicalDeviceBase(CLogicalDeviceBase&&) noexcept = default;
	CLogicalDeviceBase& operator=(CLogicalDeviceBase&&) noexcept = default;
	void destroy();
	void waitIdle();
	void create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo& createInfo);
	~CLogicalDeviceBase();
};

class VULKAN_STRUCT CDebugMessenger: public CVulkanObject<VkDebugUtilsMessengerEXT, VkInstance, VkDebugUtilsMessengerCreateInfoEXT>
{
public:
	CDebugMessenger() = default;
	CDebugMessenger(const CDebugMessenger&) = delete;
	CDebugMessenger& operator=(const CDebugMessenger&) = delete;
	CDebugMessenger(CDebugMessenger&&) noexcept = default;
	CDebugMessenger& operator=(CDebugMessenger&&) noexcept = default;
	void create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void destroy(VkInstance instance);
	~CDebugMessenger();
};
void VULKAN_STRUCT destroySurface(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);

class VULKAN_STRUCT CDescriptorPool: public CVulkanObject<VkDescriptorPool, VkDevice, VkDescriptorPoolCreateInfo>
{

public:
	CDescriptorPool() = default;
	CDescriptorPool(const CDescriptorPool&) = delete;
	CDescriptorPool& operator=(const CDescriptorPool&) = delete;
	CDescriptorPool(CDescriptorPool&&) noexcept = default;
	CDescriptorPool& operator=(CDescriptorPool&&) noexcept = default;
	void create(const VkDevice& device, const VkDescriptorPoolCreateInfo& poolInfo);// int size);
	void destroy();
	~CDescriptorPool();
};

class VULKAN_STRUCT CSwapchainBase: public CVulkanObject<VkSwapchainKHR, VkDevice, VkSwapchainCreateInfoKHR>
{
public:
	CSwapchainBase() = default;
	CSwapchainBase(const CSwapchainBase&) = delete;
	CSwapchainBase& operator=(const CSwapchainBase&) = delete;
	CSwapchainBase(CSwapchainBase&&) noexcept = default;
	CSwapchainBase& operator=(CSwapchainBase&&) noexcept = default;
	void create(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo);
	void destroy();
	std::vector<VkImage> getSwapchainImages();
	~CSwapchainBase();
};

class VULKAN_STRUCT CBufferBase: public CVulkanObject<VkBuffer, VkDevice, VkBufferCreateInfo>
{
public:
	CBufferBase() = default;
	CBufferBase(const CBufferBase&) = delete;
	CBufferBase& operator=(const CBufferBase&) = delete;
	CBufferBase(CBufferBase&&) noexcept;
	CBufferBase& operator=(CBufferBase&&) noexcept;
	VkMemoryRequirements memoryRequirements();
	void create(const VkDevice& device, const VkBufferCreateInfo& info);
	void destroy();
	~CBufferBase();
};

class VULKAN_STRUCT CImageBase: public CVulkanObject<VkImage, VkDevice, VkImageCreateInfo>
{
public:
	CImageBase() = default;
	CImageBase(const CImageBase&) = delete;
	CImageBase& operator=(const CImageBase&) = delete;
	CImageBase(CImageBase&&) noexcept = default;
	CImageBase& operator=(CImageBase&&) noexcept = default;
	VkMemoryRequirements memoryRequirements();
	void create(const VkDevice& device, const VkImageCreateInfo& rayTraceImageCreateInfo);
	void destroy();
	~CImageBase();
};

class VULKAN_STRUCT CFramebufferBase: public CVulkanObject<VkFramebuffer, VkDevice, VkFramebufferCreateInfo>
{
public:
	CFramebufferBase() = default;
	CFramebufferBase(const CFramebufferBase&) = delete;
	CFramebufferBase& operator=(const CFramebufferBase&) = delete;
	CFramebufferBase(CFramebufferBase&&) noexcept = default;
	CFramebufferBase& operator=(CFramebufferBase&&) noexcept = default;
	void create(const VkDevice& device, const VkFramebufferCreateInfo& framebufferInfo);
	void destroy();
	~CFramebufferBase();
};

class VULKAN_STRUCT CSampler: public CVulkanObject<VkSampler, VkDevice, VkSamplerCreateInfo>
{
public:
	CSampler() = default;
	CSampler(const CSampler&) = delete;
	CSampler& operator=(const CSampler&) = delete;
	CSampler(CSampler&&) noexcept = default;
	CSampler& operator=(CSampler&&) noexcept = default;
	void create(const VkDevice& device, const VkSamplerCreateInfo& samplerInfo);
	void destroy();
	~CSampler();
};

// Memory block suballocated by the device's VmaAllocator (see vk_init::vmaAllocator).
class VULKAN_STRUCT CMemoryBuffer: public CVulkanObject<VmaAllocation, VkDevice, VmaAllocationCreateInfo>
{
public:
	CMemoryBuffer() = default;
	CMemoryBuffer(const CMemoryBuffer&) = delete;
	CMemoryBuffer& operator=(const CMemoryBuffer&) = delete;
	CMemoryBuffer(CMemoryBuffer&&) noexcept;
	CMemoryBuffer& operator=(CMemoryBuffer&&) noexcept;
	void createForBuffer(const VkDevice& device, VkBuffer buffer, const VmaAllocationCreateInfo& allocInfo);
	void createForImage(const VkDevice& device, VkImage image, const VmaAllocationCreateInfo& allocInfo);
	void bindBuffer(VkBuffer buffer);
	void bindImage(VkImage image);
	VkDeviceMemory deviceMemory() const;
	VkDeviceSize offset() const;
	void destroy();
	void loadMemory(size_t bufferSize, const void* dataIn, size_t offset);
	void readMemory(size_t bufferSize, void* dataIn);
	~CMemoryBuffer();
};

class VULKAN_STRUCT CImageView: public CVulkanObject<VkImageView, VkDevice, VkImageViewCreateInfo>
{
public:
	CImageView() = default;
	CImageView(const CImageView&) = delete;
	CImageView& operator=(const CImageView&) = delete;
	CImageView(CImageView&&) noexcept = default;
	CImageView& operator=(CImageView&&) noexcept = default;
	void create(const VkDevice& device, const VkImageViewCreateInfo& viewInfo);
	void destroy();
	~CImageView();
};

class VULKAN_STRUCT CFence: public CVulkanObject<VkFence, VkDevice, VkFenceCreateInfo>
{
public:
	CFence() = default;
	CFence(const CFence&) = delete;
	CFence& operator=(const CFence&) = delete;
	CFence(CFence&&) noexcept = default;
	CFence& operator=(CFence&&) noexcept = default;
	void reset();
	void create(const VkDevice& device, const VkFenceCreateInfo& fenceInfo);
	void destroy();
	VkResult waitForFence();

	bool isNull()
	{
		return handle() == VK_NULL_HANDLE;
	}
	~CFence();
};

class VULKAN_STRUCT CCommandPoolBase: public CVulkanObject<VkCommandPool, VkDevice, VkCommandPoolCreateInfo>
{
public:
	CCommandPoolBase() = default;
	CCommandPoolBase(const CCommandPoolBase&) = delete;
	CCommandPoolBase& operator=(const CCommandPoolBase&) = delete;
	CCommandPoolBase(CCommandPoolBase&&) noexcept = default;
	CCommandPoolBase& operator=(CCommandPoolBase&&) noexcept = default;
	void create(const VkDevice& device, const VkCommandPoolCreateInfo& poolInfo);
	void destroy();
	void allocateCommandBuffer(VkCommandBufferLevel level, uint32_t size, VkCommandBuffer* pCommandBuffers);
	void freeCommandBuffers(uint32_t size, VkCommandBuffer* pCommand);
	~CCommandPoolBase();
};

class VULKAN_STRUCT CPipelineLayoutBase: public CVulkanObject<VkPipelineLayout, VkDevice, VkPipelineLayoutCreateInfo>
{
public:
	CPipelineLayoutBase() = default;
	CPipelineLayoutBase(const CPipelineLayoutBase&) = delete;
	CPipelineLayoutBase& operator=(const CPipelineLayoutBase&) = delete;
	CPipelineLayoutBase(CPipelineLayoutBase&&) noexcept = default;
	CPipelineLayoutBase& operator=(CPipelineLayoutBase&&) noexcept = default;
	void create(const VkDevice& device, const VkPipelineLayoutCreateInfo& pipelineLayoutInfo);
	void destroy();
	~CPipelineLayoutBase();
};

class VULKAN_STRUCT CPipelineBase: public   CVulkanObject<VkPipeline, VkDevice, std::monostate>
{
public:
	CPipelineBase() = default;
	CPipelineBase(const CPipelineBase&) = delete;
	CPipelineBase& operator=(const CPipelineBase&) = delete;
	CPipelineBase(CPipelineBase&&) noexcept = default;
	CPipelineBase& operator=(CPipelineBase&&) noexcept = default;
	void create(const VkDevice& device, const VkGraphicsPipelineCreateInfo& createInfo);
	void createCompute(const VkDevice& device, const VkComputePipelineCreateInfo& createInfo);
	void createRT(const VkDevice& device, const VkRayTracingPipelineCreateInfoKHR& rayTracingPipelineCreateInfo);
	void destroy();
	~CPipelineBase();
};

class VULKAN_STRUCT CShaderModule: public CVulkanObject<VkShaderModule, VkDevice, VkShaderModuleCreateInfo>
{
public:

	CShaderModule(const CShaderModule&) = delete;
	CShaderModule& operator=(const CShaderModule&) = delete;
	CShaderModule(CShaderModule&&) noexcept = default;
	CShaderModule& operator=(CShaderModule&&) noexcept = default;
	CShaderModule() = default;
	void create(const VkDevice& device, const VkShaderModuleCreateInfo& createInfo);
	void destroy();
	~CShaderModule();
};

class VULKAN_STRUCT CDescriptorSetLayoutBase: public CVulkanObject<VkDescriptorSetLayout, VkDevice, VkDescriptorSetLayoutCreateInfo>
{
public:
	CDescriptorSetLayoutBase() = default;
	CDescriptorSetLayoutBase(const CDescriptorSetLayoutBase&) = delete;
	CDescriptorSetLayoutBase& operator=(const CDescriptorSetLayoutBase&) = delete;
	CDescriptorSetLayoutBase(CDescriptorSetLayoutBase&&) noexcept = default;
	CDescriptorSetLayoutBase& operator=(CDescriptorSetLayoutBase&&) noexcept = default;
	void create(const VkDevice& device, const VkDescriptorSetLayoutCreateInfo& layoutInfo);
	void destroy();
	~CDescriptorSetLayoutBase();
};

class VULKAN_STRUCT CSemaphore: public CVulkanObject<VkSemaphore, VkDevice, VkSemaphoreCreateInfo>
{
public:
	CSemaphore() = default;
	CSemaphore(const CSemaphore&) = delete;
	CSemaphore& operator=(const CSemaphore&) = delete;
	CSemaphore(CSemaphore&&) noexcept = default;
	CSemaphore& operator=(CSemaphore&&) noexcept = default;
	void create(const VkDevice& device, const VkSemaphoreCreateInfo& semaphoreInfo);
	void destroy();
	~CSemaphore();
};

// VkQueue is not created/destroyed by the application (it is retrieved from and
// owned by the VkDevice), so this wrapper intentionally has no destructor.
class VULKAN_STRUCT CQueue: public CVulkanObject<VkQueue, VkDevice, VkQueue>
{
public:
	void getDeviceQueue(const VkDevice& device, uint32_t family);
	void submit(const VkSubmitInfo& submitInfo, VkFence fence);
	void waitIdle();
	VkResult present(const VkPresentInfoKHR& presentInfo);
};

class VULKAN_STRUCT CDescriptorSetBase: public  CVkDeviceBase<VkDevice>
{
public:
	CDescriptorSetBase() = default;
	CDescriptorSetBase(const CDescriptorSetBase&) = delete;
	CDescriptorSetBase& operator=(const CDescriptorSetBase&) = delete;
	CDescriptorSetBase(CDescriptorSetBase&&) noexcept = default;
	CDescriptorSetBase& operator=(CDescriptorSetBase&&) noexcept = default;

	VkDescriptorSet m_set{VK_NULL_HANDLE};

	VkDescriptorSetLayout m_layout{VK_NULL_HANDLE};
	VkBuffer m_descriptorBuffer{VK_NULL_HANDLE};
	void* m_mappedMemory{nullptr};
	VkDeviceAddress m_descriptorBufferAddress{};


	VkDeviceSize m_setSize{};
	VkDeviceSize m_alignedSetSize{};

public:
	void create(const VkDevice& device, const VkDescriptorSetAllocateInfo&);
	void destroy();
	void update(std::vector<VkWriteDescriptorSet>& descriptorWrites);
	VkDeviceSize allocateDescriptorSetOffset();
	~CDescriptorSetBase();

private:
	VkDescriptorPool m_pool{VK_NULL_HANDLE};
};

// Command buffers are allocated from and freed/destroyed by their owning
// CCommandPoolBase (there is no vkDestroyCommandBuffer) - this wrapper is
// intentionally not self-destroying; its pool owns its lifetime.
class VULKAN_STRUCT CCommandBuffer: public CVulkanObject<VkCommandBuffer, VkDevice,  std::monostate>
{
public:
	CCommandBuffer() = default;

	CCommandBuffer(const VkDevice& devise)
	{
		setDevice(devise);
	}
	void beginCommandBuffer(const VkCommandBufferBeginInfo& beginInfo);
	void endCommandBuffer();
	void blitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
	void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void beginRenderPass(const VkRenderPassBeginInfo& pRenderPassBegin, VkSubpassContents subpassContents);
	void endRenderPass();
	void beginRendering(const VkRenderingInfo* pRenderingInfo);
	void endRendering();
	void bindDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
		VkPipelineLayout layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const VkDescriptorSet* pDescriptorSets,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets);
	void bindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
	void bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
	void pipelineBarrier(VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkDependencyFlags dependencyFlags,
		uint32_t memoryBarrierCount,
		const VkMemoryBarrier* pMemoryBarriers,
		uint32_t bufferMemoryBarrierCount,
		const VkBufferMemoryBarrier* pBufferMemoryBarriers,
		uint32_t imageMemoryBarrierCount,
		const VkImageMemoryBarrier* pImageMemoryBarriers);
	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
	void copyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
	void traceRaysKHR(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
		uint32_t width,
		uint32_t height,
		uint32_t depth);
	void buildAccelerationStructuresKHR(uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size);
	void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
	void pushConstant(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
	void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
	void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
	void bindDescriptorBuffersEXT(uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT* pBindingInfos);
	void setDescriptorBufferOffsetsEXT(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets);
	void pipelineBarrier2(const VkDependencyInfo* pDependencyInfo);
};

}
