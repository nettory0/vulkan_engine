#include "vk_base.h"
#include <cstring>
#include <vector>
#include <array>
#include <map>
#include <typeinfo>
#include  <stdexcept>
#include  "vk_struct_builders.h"

namespace vk_base
{

void CLogicalDeviceBase::destroy()
{
	if(auto deviceHandle = device())
	{
		// All VMA allocations must be freed by now - the allocator goes before the device.
		vk_init::vmaAllocatorRemove(device());
		vk_init::volkTable(deviceHandle).vkDestroyDevice(device(), nullptr);
		vk_init::volkTableRemove(device());
		releaseDevice();
	}
}

void CLogicalDeviceBase::waitIdle()
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkDeviceWaitIdle(device());
}

void CLogicalDeviceBase::create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo& createInfo)
{
	VkDevice newDevice;
	vk_init::checkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr, &newDevice), typeid(CLogicalDeviceBase).name(), {__func__});
	setDevice(newDevice);
	volkLoadDeviceTable(&vk_init::volkTableAdd(device()), device());
	vk_init::vmaAllocatorAdd(volkGetLoadedInstance(), physicalDevice, device());
}

CLogicalDeviceBase::~CLogicalDeviceBase()
{
	destroy();
}

void CInstanceBase::create(const VkInstanceCreateInfo& createInfo)
{

	//vkCreateInstance(&createInfo, nullptr, &m_handle);
	createVulkanObject(vkCreateInstance, &createInfo, {});
}

void CInstanceBase::destroy()
{
	destroyVulkanObject(vkDestroyInstance);
}

CInstanceBase::~CInstanceBase()
{
	destroy();
}

void CDebugMessenger::create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createVulkanObject(vkCreateDebugUtilsMessengerEXT, &createInfo, instance);
}

void CDebugMessenger::destroy(VkInstance instance)
{
	destroyVulkanObject(vkDestroyDebugUtilsMessengerEXT);
}

CDebugMessenger::~CDebugMessenger()
{
	destroy(device());
}

void destroySurface(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator)
{
	vkDestroySurfaceKHR(instance, surface, pAllocator);
}

void CDescriptorPool::create(const VkDevice& device, const VkDescriptorPoolCreateInfo& createInfo)// int size)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateDescriptorPool, &createInfo, device);
}

void CDescriptorPool::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyDescriptorPool);
}

CDescriptorPool::~CDescriptorPool()
{
	destroy();
}

void CSwapchainBase::create(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateSwapchainKHR, &createInfo, device);
}

void CSwapchainBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroySwapchainKHR);
}

std::vector<VkImage> CSwapchainBase::getSwapchainImages()
{
	if(auto deviceHandle = device())
	{
		uint32_t swapchainImageCount = 0;
		vk_init::volkTable(deviceHandle).vkGetSwapchainImagesKHR(device(), handle(), &swapchainImageCount, NULL);
		std::vector<VkImage> swapchainImageHandleList(swapchainImageCount);
		vk_init::volkTable(deviceHandle).vkGetSwapchainImagesKHR(device(), handle(), &swapchainImageCount, swapchainImageHandleList.data());
		return swapchainImageHandleList;
	}
	return {};
}

CSwapchainBase::~CSwapchainBase()
{
	destroy();
}

CBufferBase::CBufferBase(CBufferBase&& other) noexcept
{
	*this = std::move(other);
}

CBufferBase& CBufferBase::operator=(CBufferBase&& other) noexcept
{
	if(this == &other)
		return *this;
	destroy();
	m_handle = other.m_handle;
	other.m_handle = VK_NULL_HANDLE;
	setDevice(other.device());
	other.setDevice(VK_NULL_HANDLE);
	return *this;
}

VkMemoryRequirements CBufferBase::memoryRequirements()
{
	VkMemoryRequirements memoryRequirements{};
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkGetBufferMemoryRequirements(device(), handle(), &memoryRequirements);
	return memoryRequirements;
}

void CBufferBase::create(const VkDevice& device, const VkBufferCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateBuffer, &createInfo, device);
}

void CBufferBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyBuffer);
}

CBufferBase::~CBufferBase()
{
	destroy();
}

VkMemoryRequirements CImageBase::memoryRequirements()
{
	VkMemoryRequirements memoryRequirements{};
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkGetImageMemoryRequirements(device(), handle(), &memoryRequirements);
	return memoryRequirements;
}

void CImageBase::create(const VkDevice& device, const VkImageCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateImage, &createInfo, device);
}

void CImageBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyImage);
}

CImageBase::~CImageBase()
{
	destroy();
}

void CFramebufferBase::create(const VkDevice& device, const VkFramebufferCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateFramebuffer, &createInfo, device);
}

void CFramebufferBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyFramebuffer);
}

CFramebufferBase::~CFramebufferBase()
{
	destroy();
}

void CSampler::create(const VkDevice& device, const VkSamplerCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateSampler, &createInfo, device);
}

void CSampler::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroySampler);
}

CSampler::~CSampler()
{
	destroy();
}

CMemoryBuffer::CMemoryBuffer(CMemoryBuffer&& other) noexcept
{
	*this = std::move(other);
}
CMemoryBuffer& CMemoryBuffer::operator=(CMemoryBuffer&& other) noexcept
{
	if(this == &other)
		return *this;
	destroy();
	m_handle = other.m_handle;
	other.m_handle = VK_NULL_HANDLE;
	setDevice(other.device());
	other.setDevice(VK_NULL_HANDLE);
	return *this;
}

void CMemoryBuffer::destroy()
{
	if(auto deviceHandle = device())
	{
		if(handle())
			vmaFreeMemory(vk_init::vmaAllocator(deviceHandle), handle());
		m_handle = VK_NULL_HANDLE;
		releaseDevice();
	}
}

void CMemoryBuffer::createForBuffer(const VkDevice& device, VkBuffer buffer, const VmaAllocationCreateInfo& createInfo)
{
	setDevice(device);
	vk_init::checkResult(vmaAllocateMemoryForBuffer(vk_init::vmaAllocator(device), buffer, &createInfo, &m_handle, nullptr), typeid(CMemoryBuffer).name(), {__func__});
}

void CMemoryBuffer::createForImage(const VkDevice& device, VkImage image, const VmaAllocationCreateInfo& createInfo)
{
	setDevice(device);
	vk_init::checkResult(vmaAllocateMemoryForImage(vk_init::vmaAllocator(device), image, &createInfo, &m_handle, nullptr), typeid(CMemoryBuffer).name(), {__func__});
}

void CMemoryBuffer::bindBuffer(VkBuffer buffer)
{
	if(auto deviceHandle = device())
		vk_init::checkResult(vmaBindBufferMemory(vk_init::vmaAllocator(deviceHandle), handle(), buffer), typeid(CMemoryBuffer).name(), {__func__});
}

void CMemoryBuffer::bindImage(VkImage image)
{
	if(auto deviceHandle = device())
		vk_init::checkResult(vmaBindImageMemory(vk_init::vmaAllocator(deviceHandle), handle(), image), typeid(CMemoryBuffer).name(), {__func__});
}

VkDeviceMemory CMemoryBuffer::deviceMemory() const
{
	if(auto deviceHandle = device(); deviceHandle && handle())
	{
		VmaAllocationInfo info{};
		vmaGetAllocationInfo(vk_init::vmaAllocator(deviceHandle), handle(), &info);
		return info.deviceMemory;
	}
	return VK_NULL_HANDLE;
}

VkDeviceSize CMemoryBuffer::offset() const
{
	if(auto deviceHandle = device(); deviceHandle && handle())
	{
		VmaAllocationInfo info{};
		vmaGetAllocationInfo(vk_init::vmaAllocator(deviceHandle), handle(), &info);
		return info.offset;
	}
	return 0;
}

// vmaCopy* map (refcounted, no-op for persistently mapped allocations), copy and flush/invalidate
// non-coherent memory, and take the allocation's offset inside the VkDeviceMemory block into account.
void CMemoryBuffer::loadMemory(size_t bufferSize, const void* dataIn, size_t offset)
{
	if(auto deviceHandle = device())
	{
		if(dataIn == nullptr || bufferSize == 0 || !handle())
			return;
		vmaCopyMemoryToAllocation(vk_init::vmaAllocator(deviceHandle), dataIn, handle(), offset, bufferSize);
	}
}

void CMemoryBuffer::readMemory(size_t bufferSize, void* dataIn)
{
	if(auto deviceHandle = device())
	{
		if(dataIn == nullptr || bufferSize == 0 || !handle())
			return;
		vmaCopyAllocationToMemory(vk_init::vmaAllocator(deviceHandle), handle(), 0, dataIn, bufferSize);
	}
}

CMemoryBuffer::~CMemoryBuffer()
{
	destroy();
}

void CImageView::create(const VkDevice& device, const VkImageViewCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateImageView, &createInfo, device);
}

void CImageView::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyImageView);
}

CImageView::~CImageView()
{
	destroy();
}

void CFence::reset()
{
	if(auto deviceHandle = device())
		vk_init::checkResult(vk_init::volkTable(deviceHandle).vkResetFences(deviceHandle, 1, &handle()), {typeid(CFence).name()}, {__func__});
}

void CFence::create(const VkDevice& device, const VkFenceCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateFence, &createInfo, device);
}

void CFence::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyFence);
}

VkResult CFence::waitForFence()
{
	if(auto deviceHandle = device())
	{
		if(!isNull())
			return vk_init::volkTable(deviceHandle).vkWaitForFences(device(), 1, &handle(), VK_TRUE, UINT64_MAX);
	}
	return VK_SUCCESS;
}

CFence::~CFence()
{
	destroy();
}

void CQueue::getDeviceQueue(const VkDevice& device, uint32_t family)
{
	setDevice(device);
	vk_init::volkTable(device).vkGetDeviceQueue(device, family, 0, &handle());
}

void CQueue::submit(const VkSubmitInfo& submitInfo, VkFence fence)
{
	if(auto deviceHandle = device())
	{
		if(auto result = vk_init::volkTable(deviceHandle).vkQueueSubmit(handle(), 1, &submitInfo, fence); result != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void CQueue::waitIdle()
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkQueueWaitIdle(handle());
}

VkResult CQueue::present(const VkPresentInfoKHR& presentInfo)
{
	if(auto deviceHandle = device())
		return vk_init::volkTable(deviceHandle).vkQueuePresentKHR(handle(), &presentInfo);
	return VK_ERROR_UNKNOWN;
}

void CSemaphore::create(const VkDevice& device, const VkSemaphoreCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateSemaphore, &createInfo, device);
}

void CSemaphore::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroySemaphore);
}

CSemaphore::~CSemaphore()
{
	destroy();
}

void CPipelineLayoutBase::create(const VkDevice& device, const VkPipelineLayoutCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreatePipelineLayout, &createInfo, device);
}

void CPipelineLayoutBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyPipelineLayout);
}

CPipelineLayoutBase::~CPipelineLayoutBase()
{
	destroy();
}

void CCommandPoolBase::create(const VkDevice& device, const VkCommandPoolCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateCommandPool, &createInfo, device);
}

void CCommandPoolBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyCommandPool);
}

void CCommandPoolBase::allocateCommandBuffer(VkCommandBufferLevel level, uint32_t size, VkCommandBuffer* pCommandBuffers)
{
	if(auto deviceHandle = device())
	{
		VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = handle(), .level = level, .commandBufferCount = size};
		vk_init::volkTable(deviceHandle).vkAllocateCommandBuffers(device(), &allocInfo, pCommandBuffers);
	}
}

void CCommandPoolBase::freeCommandBuffers(uint32_t size, VkCommandBuffer* pCommand)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkFreeCommandBuffers(device(), handle(), size, pCommand);
}

CCommandPoolBase::~CCommandPoolBase()
{
	destroy();
}

void CPipelineBase::create(const VkDevice& device, const VkGraphicsPipelineCreateInfo& createInfo)
{
	setDevice(device);
	if(vk_init::volkTable(device).vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle()) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline!");
}

void CPipelineBase::createCompute(const VkDevice& device, const VkComputePipelineCreateInfo& createInfo)
{
	setDevice(device);
	if(vk_init::volkTable(device).vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle()))
		throw std::runtime_error("failed to create compute pipeline!");
}

void CPipelineBase::createRT(const VkDevice& device, const VkRayTracingPipelineCreateInfoKHR& rayTracingPipelineCreateInfo)
{
	setDevice(device);
	if(vk_init::volkTable(device).vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, NULL, &handle()) != VK_SUCCESS)
		throw std::runtime_error("failed to create ray tracing pipeline!");
}

void CPipelineBase::destroy()
{
	if(handle())
	{
		if(auto deviceHandle = device())
			vk_init::volkTable(deviceHandle).vkDestroyPipeline(device(), handle(), nullptr);
		handle() = {};
		releaseDevice();
	}
}

CPipelineBase::~CPipelineBase()
{
	destroy();
}

void CDescriptorSetBase::create(const VkDevice& device, const VkDescriptorSetAllocateInfo& allocInfo)
{
	setDevice(device);
	vk_init::checkResult(vk_init::volkTable(device).vkAllocateDescriptorSets(device, &allocInfo, &m_set), typeid(CDescriptorSetBase).name(), {__func__});
	m_pool = allocInfo.descriptorPool;
}

void  CDescriptorSetBase::destroy()
{
	if(m_set)
	{
		if(auto deviceHandle = device())
			vk_init::volkTable(deviceHandle).vkFreeDescriptorSets(device(), m_pool, 1, &m_set);
		m_set = VK_NULL_HANDLE;
	}
}

void CDescriptorSetBase::update(std::vector<VkWriteDescriptorSet>& aDescriptorWrites)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkUpdateDescriptorSets(device(), aDescriptorWrites.size(), aDescriptorWrites.data(), 0, nullptr);
}

VkDeviceSize CDescriptorSetBase::allocateDescriptorSetOffset()
{
	VkDeviceSize offset{0};// = m_currentOffset;
	//m_currentOffset += m_alignedSetSize;
	return offset;
}

CDescriptorSetBase::~CDescriptorSetBase()
{
	destroy();
}

void CShaderModule::create(const VkDevice& device, const VkShaderModuleCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateShaderModule, &createInfo, device);
}

void CShaderModule::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyShaderModule);
}

CShaderModule::~CShaderModule()
{
	destroy();
}

void CDescriptorSetLayoutBase::create(const VkDevice& device, const VkDescriptorSetLayoutCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateDescriptorSetLayout, &createInfo, device);
}

void CDescriptorSetLayoutBase::destroy()
{
	if(auto deviceHandle = device())
		destroyVulkanObject(vk_init::volkTable(deviceHandle).vkDestroyDescriptorSetLayout);
}

CDescriptorSetLayoutBase::~CDescriptorSetLayoutBase()
{
	destroy();
}

void CCommandBuffer::beginCommandBuffer(const VkCommandBufferBeginInfo& beginInfo)
{
	if(auto deviceHandle = device())
		vk_init::checkResult(vk_init::volkTable(deviceHandle).vkBeginCommandBuffer(handle(), &beginInfo), typeid(CCommandBuffer).name(), {__func__});
}

void CCommandBuffer::endCommandBuffer()
{
	if(auto deviceHandle = device())
		vk_init::checkResult(vk_init::volkTable(deviceHandle).vkEndCommandBuffer(handle()), typeid(CCommandBuffer).name(), {__func__});
}

void CCommandBuffer::blitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBlitImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdDispatch(handle(), groupCountX, groupCountY, groupCountZ);
}

void CCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& pRenderPassBegin, VkSubpassContents subpassContents)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBeginRenderPass(handle(), &pRenderPassBegin, subpassContents);
}

void CCommandBuffer::beginRendering(const VkRenderingInfo* pRenderingInfo)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBeginRendering(handle(), pRenderingInfo);
}

void CCommandBuffer::endRendering()
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdEndRendering(handle());
}
void CCommandBuffer::endRenderPass()
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdEndRenderPass(handle());
}

void CCommandBuffer::bindDescriptorSet(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBindDescriptorSets(handle(), pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void CCommandBuffer::bindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBindVertexBuffers(handle(), firstBinding, bindingCount, pBuffers, pOffsets);
}

void CCommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBindIndexBuffer(handle(), buffer, offset, indexType);
}

void CCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdDrawIndexed(handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CCommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
									 VkPipelineStageFlags dstStageMask,
									 VkDependencyFlags dependencyFlags,
									 uint32_t memoryBarrierCount,
									 const VkMemoryBarrier* pMemoryBarriers,
									 uint32_t bufferMemoryBarrierCount,
									 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
									 uint32_t imageMemoryBarrierCount,
									 const VkImageMemoryBarrier* pImageMemoryBarriers)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdPipelineBarrier(handle(), srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdDraw(handle(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void CCommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBindPipeline(handle(), pipelineBindPoint, pipeline);
}

void CCommandBuffer::copyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdCopyImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CCommandBuffer::traceRaysKHR(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdTraceRaysKHR(handle(), pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

void CCommandBuffer::buildAccelerationStructuresKHR(uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBuildAccelerationStructuresKHR(handle(), infoCount, pInfos, ppBuildRangeInfos);
}

void CCommandBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
{
	if(auto deviceHandle = device())
	{
		VkBufferCopy copyRegion{.srcOffset = srcOffset, . dstOffset = dstOffset, .size = size};
		vk_init::volkTable(deviceHandle).vkCmdCopyBuffer(handle(), srcBuffer, dstBuffer, 1, &copyRegion);
	}
}

void CCommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdCopyBufferToImage(handle(), srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void CCommandBuffer::pushConstant(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdPushConstants(handle(), layout, stageFlags, offset, size, pValues);
}

void CCommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdSetViewport(handle(), firstViewport, viewportCount, pViewports);
}

void CCommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdSetScissor(handle(), firstScissor, scissorCount, pScissors);
}

void CCommandBuffer::bindDescriptorBuffersEXT(uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT* pBindingInfos)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdBindDescriptorBuffersEXT(handle(), bufferCount, pBindingInfos);
}
void CCommandBuffer::setDescriptorBufferOffsetsEXT(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdSetDescriptorBufferOffsetsEXT(handle(), pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
}

void CCommandBuffer::pipelineBarrier2(const VkDependencyInfo* pDependencyInfo)
{
	if(auto deviceHandle = device())
		vk_init::volkTable(deviceHandle).vkCmdPipelineBarrier2(handle(), pDependencyInfo);
}


}
