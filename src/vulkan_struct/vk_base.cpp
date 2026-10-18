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
	if(device())
	{
		vk_init::volkTable(device()).vkDestroyDevice(device(), nullptr);
		vk_init::volkTableRemove(device());
		releaseDevice();
	}
}

void CLogicalDeviceBase::waitIdle()
{
	if(device())
		vk_init::volkTable(device()).vkDeviceWaitIdle(device());
}

void CLogicalDeviceBase::create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo& createInfo)
{
	VkDevice newDevice;
	vk_init::checkResult(vkCreateDevice(physicalDevice, &createInfo, nullptr, &newDevice), typeid(CLogicalDeviceBase).name(), {__func__});
	setDevice(newDevice);
	volkLoadDeviceTable(&vk_init::volkTableAdd(device()), device());
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
	 //destroy(VkInstance) ignores its parameter internally (the instance is
	// already stored via CVkDeviceBase<VkInstance>), so this is safe.
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyDescriptorPool);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroySwapchainKHR);
}

std::vector<VkImage> CSwapchainBase::getSwapchainImages()
{
	uint32_t swapchainImageCount = 0;
	vk_init::volkTable(device()).vkGetSwapchainImagesKHR(device(), handle(), &swapchainImageCount, NULL);
	std::vector<VkImage> swapchainImageHandleList(swapchainImageCount);
	vk_init::volkTable(device()).vkGetSwapchainImagesKHR(device(), handle(), &swapchainImageCount, swapchainImageHandleList.data());
	return swapchainImageHandleList;
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
	VkMemoryRequirements memoryRequirements;
	vk_init::volkTable(device()).vkGetBufferMemoryRequirements(device(), handle(), &memoryRequirements);
	return memoryRequirements;
}

void CBufferBase::create(const VkDevice& device, const VkBufferCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateBuffer, &createInfo, device);
}

void CBufferBase::destroy()
{
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyBuffer);
}

CBufferBase::~CBufferBase()
{
	destroy();
}

VkMemoryRequirements CImageBase::memoryRequirements()
{
	VkMemoryRequirements memoryRequirements;
	vk_init::volkTable(device()).vkGetImageMemoryRequirements(device(), handle(), &memoryRequirements);
	return memoryRequirements;
}

void CImageBase::create(const VkDevice& device, const VkImageCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateImage, &createInfo, device);
}

void CImageBase::destroy()
{
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyImage);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyFramebuffer);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroySampler);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkFreeMemory);
}

void CMemoryBuffer::create(const VkDevice& device, const VkMemoryAllocateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkAllocateMemory, &createInfo, device);
}

void CMemoryBuffer::loadMemory(size_t bufferSize, const void* dataIn, size_t offset)
{
	auto& vk = vk_init::volkTable(device());
	void* data{nullptr};
	VkResult res = vk.vkMapMemory(device(), handle(), offset, bufferSize, 0, &data);
	if (res != VK_SUCCESS)
	{
		return;
	}
	if(dataIn == nullptr)
	{
		vk.vkUnmapMemory(device(), handle());
		return;
	}
	if (bufferSize == 0)
	{
		vk.vkUnmapMemory(device(), handle());
		return;
	}
	memcpy(data, dataIn, bufferSize);
	vk.vkUnmapMemory(device(), handle());
}

void CMemoryBuffer::readMemory(size_t bufferSize, void* dataIn)
{
	auto& vk = vk_init::volkTable(device());
	void* data{nullptr};
	VkResult res = vk.vkMapMemory(device(), handle(), 0, bufferSize, 0, &data);
	if (res != VK_SUCCESS)
	{
		return;
	}
	if(dataIn == nullptr || bufferSize == 0)
	{
		vk.vkUnmapMemory(device(), handle());
		return;
	}
	memcpy(dataIn, data, bufferSize);
	vk.vkUnmapMemory(device(), handle());
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyImageView);
}

CImageView::~CImageView()
{
	destroy();
}

void CFence::reset()
{
	vk_init::checkResult(vk_init::volkTable(device()).vkResetFences(device(), 1, &handle()), {typeid(CFence).name()}, {__func__});
}

void CFence::create(const VkDevice& device, const VkFenceCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateFence, &createInfo, device);
}

void CFence::destroy()
{
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyFence);
}

VkResult CFence::waitForFence()
{
	if(!isNull())
		return vk_init::volkTable(device()).vkWaitForFences(device(), 1, &handle(), VK_TRUE, UINT64_MAX);
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
	if(auto result = vk_init::volkTable(device()).vkQueueSubmit(handle(), 1, &submitInfo, fence); result != VK_SUCCESS)
	{
		printf("%d\n", result);
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void CQueue::waitIdle()
{
	vk_init::volkTable(device()).vkQueueWaitIdle(handle());
}

VkResult CQueue::present(const VkPresentInfoKHR& presentInfo)
{
	return vk_init::volkTable(device()).vkQueuePresentKHR(handle(), &presentInfo);
}

void CSemaphore::create(const VkDevice& device, const VkSemaphoreCreateInfo& createInfo)
{
	createVulkanObject(vk_init::volkTable(device).vkCreateSemaphore, &createInfo, device);
}

void CSemaphore::destroy()
{
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroySemaphore);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyPipelineLayout);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyCommandPool);
}

void CCommandPoolBase::allocateCommandBuffer(VkCommandBufferLevel level, uint32_t size, VkCommandBuffer* pCommandBuffers)
{
	VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = handle(), .level = level, .commandBufferCount = size};
	vk_init::volkTable(device()).vkAllocateCommandBuffers(device(), &allocInfo, pCommandBuffers);
}

void CCommandPoolBase::freeCommandBuffers(uint32_t size, VkCommandBuffer* pCommand)
{
	vk_init::volkTable(device()).vkFreeCommandBuffers(device(), handle(), size, pCommand);
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
		vk_init::volkTable(device()).vkDestroyPipeline(device(), handle(), nullptr);
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
		vk_init::volkTable(device()).vkFreeDescriptorSets(device(), m_pool, 1, &m_set);
		m_set = VK_NULL_HANDLE;
	}
}

void CDescriptorSetBase::update(std::vector<VkWriteDescriptorSet>& aDescriptorWrites)
{
	vk_init::volkTable(device()).vkUpdateDescriptorSets(device(), aDescriptorWrites.size(), aDescriptorWrites.data(), 0, nullptr);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyShaderModule);
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
	destroyVulkanObject(vk_init::volkTable(device()).vkDestroyDescriptorSetLayout);
}

CDescriptorSetLayoutBase::~CDescriptorSetLayoutBase()
{
	destroy();
}

void CCommandBuffer::beginCommandBuffer(const VkCommandBufferBeginInfo& beginInfo)
{
	vk_init::checkResult(vk_init::volkTable(device()).vkBeginCommandBuffer(handle(), &beginInfo), typeid(CCommandBuffer).name(), {__func__});
}

void CCommandBuffer::endCommandBuffer()
{
	vk_init::checkResult(vk_init::volkTable(device()).vkEndCommandBuffer(handle()), typeid(CCommandBuffer).name(), {__func__});
}

void CCommandBuffer::blitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
	vk_init::volkTable(device()).vkCmdBlitImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vk_init::volkTable(device()).vkCmdDispatch(handle(), groupCountX, groupCountY, groupCountZ);
}

void CCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& pRenderPassBegin, VkSubpassContents subpassContents)
{
	vk_init::volkTable(device()).vkCmdBeginRenderPass(handle(), &pRenderPassBegin, subpassContents);
}

void CCommandBuffer::beginRendering(const VkRenderingInfo* pRenderingInfo)
{
	vk_init::volkTable(device()).vkCmdBeginRendering(handle(), pRenderingInfo);
}

void CCommandBuffer::endRendering()
{
	vk_init::volkTable(device()).vkCmdEndRendering(handle());
}
void CCommandBuffer::endRenderPass()
{
	vk_init::volkTable(device()).vkCmdEndRenderPass(handle());
}

void CCommandBuffer::bindDescriptorSet(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	vk_init::volkTable(device()).vkCmdBindDescriptorSets(handle(), pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void CCommandBuffer::bindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	vk_init::volkTable(device()).vkCmdBindVertexBuffers(handle(), firstBinding, bindingCount, pBuffers, pOffsets);
}

void CCommandBuffer::bindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vk_init::volkTable(device()).vkCmdBindIndexBuffer(handle(), buffer, offset, indexType);
}

void CCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	vk_init::volkTable(device()).vkCmdDrawIndexed(handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
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
	vk_init::volkTable(device()).vkCmdPipelineBarrier(handle(), srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CCommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vk_init::volkTable(device()).vkCmdDraw(handle(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void CCommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	vk_init::volkTable(device()).vkCmdBindPipeline(handle(), pipelineBindPoint, pipeline);
}

void CCommandBuffer::copyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
	vk_init::volkTable(device()).vkCmdCopyImage(handle(), srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CCommandBuffer::traceRaysKHR(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
{
	vk_init::volkTable(device()).vkCmdTraceRaysKHR(handle(), pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

void CCommandBuffer::buildAccelerationStructuresKHR(uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
	vk_init::volkTable(device()).vkCmdBuildAccelerationStructuresKHR(handle(), infoCount, pInfos, ppBuildRangeInfos);
}

void CCommandBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size)
{
	VkBufferCopy copyRegion{.srcOffset = srcOffset, . dstOffset = dstOffset, .size = size};
	vk_init::volkTable(device()).vkCmdCopyBuffer(handle(), srcBuffer, dstBuffer, 1, &copyRegion);
}

void CCommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	vk_init::volkTable(device()).vkCmdCopyBufferToImage(handle(), srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void CCommandBuffer::pushConstant(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
	vk_init::volkTable(device()).vkCmdPushConstants(handle(), layout, stageFlags, offset, size, pValues);
}

void CCommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	vk_init::volkTable(device()).vkCmdSetViewport(handle(), firstViewport, viewportCount, pViewports);
}

void CCommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	vk_init::volkTable(device()).vkCmdSetScissor(handle(), firstScissor, scissorCount, pScissors);
}

void CCommandBuffer::bindDescriptorBuffersEXT(uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT* pBindingInfos)
{
	vk_init::volkTable(device()).vkCmdBindDescriptorBuffersEXT(handle(), bufferCount, pBindingInfos);
}
void CCommandBuffer::setDescriptorBufferOffsetsEXT(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets)
{
	vk_init::volkTable(device()).vkCmdSetDescriptorBufferOffsetsEXT(handle(), pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
}

void CCommandBuffer::pipelineBarrier2(const VkDependencyInfo* pDependencyInfo)
{
	vk_init::volkTable(device()).vkCmdPipelineBarrier2(handle(), pDependencyInfo);
}


}
