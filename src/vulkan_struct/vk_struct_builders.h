#pragma once
#include "vk_export.h"
#include "volk.h"
#include "vk_mem_alloc.h"


namespace vk_init
{

VULKAN_STRUCT VolkDeviceTable& volkTableAdd(void* device);
VULKAN_STRUCT void volkTableRemove(void*);
VULKAN_STRUCT const VolkDeviceTable& volkTable(void*);

// One VMA allocator per logical device, created/destroyed together with the device.
VULKAN_STRUCT VmaAllocator vmaAllocatorAdd(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
VULKAN_STRUCT void vmaAllocatorRemove(void* device);
VULKAN_STRUCT VmaAllocator vmaAllocator(void* device);

VULKAN_STRUCT void checkResult(VkResult result, const char* obj, const char* func);


VULKAN_STRUCT VkResult acquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

VULKAN_STRUCT VkBufferCreateInfo bufferCreateInfo(const void* pNext,
	VkBufferCreateFlags flags,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkSharingMode sharingMode,
	uint32_t queueFamilyIndexCount,
	const uint32_t* pQueueFamilyIndices);

VULKAN_STRUCT VkBufferCreateInfo bufferCreateInfoSimple(const void* pNext, VkDeviceSize size, VkBufferUsageFlags usage, const uint32_t* pQueueFamilyIndices);


VULKAN_STRUCT VkWriteDescriptorSet writeDescriptorSet(const void* pNext,
	VkDescriptorSet dstSet,
	uint32_t dstBinding,
	uint32_t dstArrayElement,
	uint32_t descriptorCount,
	VkDescriptorType descriptorType,
	const VkDescriptorImageInfo* pImageInfo,
	const VkDescriptorBufferInfo* pBufferInfo,
	const VkBufferView* pTexelBufferView);

VULKAN_STRUCT VkWriteDescriptorSet writeDescriptorSetBuffer(const void* pNext,
	VkDescriptorSet dstSet,
	uint32_t dstBinding,
	uint32_t dstArrayElement,
	uint32_t descriptorCount,
	VkDescriptorType descriptorType,
	const VkDescriptorBufferInfo* pBufferInfo);

VULKAN_STRUCT VkWriteDescriptorSet writeDescriptorSetImage(const void* pNext,
	VkDescriptorSet dstSet,
	uint32_t dstBinding,
	uint32_t dstArrayElement,
	VkDescriptorType descriptorType,
	const VkDescriptorImageInfo* pImageInfo);

VULKAN_STRUCT VkImageMemoryBarrier imageMemoryBarrier(
	const void* pNext,
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t srcQueueFamilyIndex,
	uint32_t dstQueueFamilyIndex,
	VkImage image,
	VkImageSubresourceRange subresourceRange);

VULKAN_STRUCT VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
	uint32_t binding,
	VkDescriptorType descriptorType,
	uint32_t descriptorCount,
	VkShaderStageFlags stageFlags,
	const VkSampler* pImmutableSamplers);

VULKAN_STRUCT VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
	const void* pNext,
	VkDescriptorSetLayoutCreateFlags flags,
	uint32_t bindingCount,
	const VkDescriptorSetLayoutBinding* pBindings);

VULKAN_STRUCT VkAttachmentDescription attachmentDescription(
	VkAttachmentDescriptionFlags flags,
	VkFormat format,
	VkSampleCountFlagBits samples,
	VkAttachmentLoadOp loadOp,
	VkAttachmentStoreOp storeOp,
	VkAttachmentLoadOp stencilLoadOp,
	VkAttachmentStoreOp stencilStoreOp,
	VkImageLayout initialLayout,
	VkImageLayout finalLayout);
VULKAN_STRUCT VkFramebufferCreateInfo framebufferCreateInfo(
	const void* pNext,
	VkFramebufferCreateFlags flags,
	VkRenderPass renderPass,
	uint32_t attachmentCount,
	const VkImageView* pAttachments,
	uint32_t width,
	uint32_t height,
	uint32_t layers);

VULKAN_STRUCT VkBufferDeviceAddressInfo bufferDeviceAddressInfo(const void* pNext, VkBuffer buffer);

VULKAN_STRUCT VkFenceCreateInfo fenceCreateInfo(const void* pNext, VkFenceCreateFlags flags);
VULKAN_STRUCT VkSemaphoreCreateInfo semaphoreCreateInfo(const void* pNext, VkSemaphoreCreateFlags flags);
VULKAN_STRUCT VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const void* pNext,
	VkPipelineLayoutCreateFlags flags,
	uint32_t setLayoutCount,
	const VkDescriptorSetLayout* pSetLayouts,
	uint32_t pushConstantRangeCount,
	const VkPushConstantRange* pPushConstantRanges);
VULKAN_STRUCT VkMemoryAllocateInfo memoryAllocateInfo(const void* pNext, VkDeviceSize allocationSize, uint32_t memoryTypeIndex);


VULKAN_STRUCT VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

VULKAN_STRUCT VkSwapchainCreateInfoKHR swapchainCreateInfoKHR(
	const void* pNext,
	VkSwapchainCreateFlagsKHR flags,
	VkSurfaceKHR surface,
	uint32_t minImageCount,
	VkFormat imageFormat,
	VkColorSpaceKHR imageColorSpace,
	VkExtent2D imageExtent,
	uint32_t imageArrayLayers,
	VkImageUsageFlags imageUsage,
	VkSharingMode imageSharingMode,
	uint32_t queueFamilyIndexCount,
	const uint32_t* pQueueFamilyIndices,
	VkSurfaceTransformFlagBitsKHR preTransform,
	VkCompositeAlphaFlagBitsKHR compositeAlpha,
	VkPresentModeKHR presentMode,
	VkBool32 clipped,
	VkSwapchainKHR oldSwapchain);

VULKAN_STRUCT VkImageCreateInfo imageCreateInfo(
	const void* pNext,
	VkImageCreateFlags flags,
	VkImageType imageType,
	VkFormat format,
	VkExtent3D extent,
	uint32_t mipLevels,
	uint32_t arrayLayers,
	VkSampleCountFlagBits samples,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkSharingMode sharingMode,
	uint32_t  queueFamilyIndexCount,
	const uint32_t* pQueueFamilyIndices,
	VkImageLayout initialLayout);

VULKAN_STRUCT VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const void* pNext, VkPipelineViewportStateCreateFlags flags, uint32_t viewportCount, const VkViewport* pViewports, uint32_t scissorCount, const VkRect2D* pScissors);

VULKAN_STRUCT VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
	const void* pNext,
	VkPipelineCreateFlags flags,
	uint32_t stageCount,
	const VkPipelineShaderStageCreateInfo* pStages,
	const VkPipelineVertexInputStateCreateInfo* pVertexInputState,
	const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
	const VkPipelineTessellationStateCreateInfo* pTessellationState,
	const VkPipelineViewportStateCreateInfo* pViewportState,
	const VkPipelineRasterizationStateCreateInfo* pRasterizationState,
	const VkPipelineMultisampleStateCreateInfo* pMultisampleState,
	const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
	const VkPipelineColorBlendStateCreateInfo* pColorBlendState,
	const VkPipelineDynamicStateCreateInfo* pDynamicState,
	VkPipelineLayout layout,
	VkRenderPass renderPass,
	uint32_t subpass,
	VkPipeline basePipelineHandle,
	int32_t  basePipelineIndex);

VULKAN_STRUCT VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
	VkBool32 blendEnable,
	VkBlendFactor srcColorBlendFactor,
	VkBlendFactor dstColorBlendFactor,
	VkBlendOp colorBlendOp,
	VkBlendFactor srcAlphaBlendFactor,
	VkBlendFactor dstAlphaBlendFactor,
	VkBlendOp alphaBlendOp,
	VkColorComponentFlags colorWriteMask);

VULKAN_STRUCT VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
	const void* pNext,
	VkPipelineColorBlendStateCreateFlags flags,
	VkBool32 logicOpEnable,
	VkLogicOp logicOp,
	uint32_t attachmentCount,
	const VkPipelineColorBlendAttachmentState* pAttachments,
	float blendConstants[4]);

VULKAN_STRUCT VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
	const void* pNext,
	VkPipelineVertexInputStateCreateFlags flags,
	uint32_t vertexBindingDescriptionCount,
	const VkVertexInputBindingDescription* pVertexBindingDescriptions,
	uint32_t vertexAttributeDescriptionCount,
	const VkVertexInputAttributeDescription* pVertexAttributeDescriptions);

VULKAN_STRUCT VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
	const void* pNext,
	VkPipelineDepthStencilStateCreateFlags flags,
	VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable,
	VkCompareOp depthCompareOp,
	VkBool32 depthBoundsTestEnable,
	VkBool32 stencilTestEnable,
	VkStencilOpState front,
	VkStencilOpState back,
	float minDepthBounds,
	float maxDepthBounds
);

VULKAN_STRUCT VkImageViewCreateInfo imageViewCreateInfo(
	const void* pNext,
	VkImageViewCreateFlags flags,
	VkImage image,
	VkImageViewType viewType,
	VkFormat format,
	VkComponentMapping components,
	VkImageSubresourceRange subresourceRange
);

VULKAN_STRUCT VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR(
	const void* pNext,
	VkRayTracingShaderGroupTypeKHR type,
	uint32_t generalShader,
	uint32_t closestHitShader,
	uint32_t anyHitShader,
	uint32_t intersectionShader,
	const void* pShaderGroupCaptureReplayHandle
);

VULKAN_STRUCT VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(
	const void* pNext,
	VkPipelineShaderStageCreateFlags flags,
	VkShaderStageFlagBits stage,
	VkShaderModule module,
	const char* pName,
	const VkSpecializationInfo* pSpecializationInfo);

VULKAN_STRUCT VkShaderModuleCreateInfo shaderModuleCreateInfo(const void* pNext, VkShaderModuleCreateFlags flags, size_t codeSize, const uint32_t* pCode);

VULKAN_STRUCT VkComputePipelineCreateInfo computePipelineCreateInfo(
	const void* pNext,
	VkPipelineCreateFlags flags,
	VkPipelineShaderStageCreateInfo stage,
	VkPipelineLayout layout,
	VkPipeline basePipelineHandle,
	int32_t basePipelineIndex);

VULKAN_STRUCT VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(const void* pNext,
	VkDescriptorPool descriptorPool,
	uint32_t descriptorSetCount,
	const VkDescriptorSetLayout* pSetLayouts);

VULKAN_STRUCT VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR(const void* pNext,
	VkGeometryTypeKHR geometryType,
	VkAccelerationStructureGeometryDataKHR geometry,
	VkGeometryFlagsKHR flags);

VULKAN_STRUCT VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR(const void* pNext,
	VkAccelerationStructureTypeKHR type,
	VkBuildAccelerationStructureFlagsKHR flags,
	VkBuildAccelerationStructureModeKHR mode,
	VkAccelerationStructureKHR srcAccelerationStructure,
	VkAccelerationStructureKHR dstAccelerationStructure,
	uint32_t geometryCount,
	const VkAccelerationStructureGeometryKHR* pGeometries,
	const VkAccelerationStructureGeometryKHR* const* ppGeometries,
	VkDeviceOrHostAddressKHR scratchData);

VULKAN_STRUCT VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const void* pNext,
	VkDescriptorPoolCreateFlags flags,
	uint32_t maxSets,
	uint32_t poolSizeCount,
	const VkDescriptorPoolSize* pPoolSizes);

VULKAN_STRUCT VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfoKHR(const void* pNext,
	VkAccelerationStructureCreateFlagsKHR createFlags,
	VkBuffer buffer,
	VkDeviceSize offset,
	VkDeviceSize size,
	VkAccelerationStructureTypeKHR type,
	VkDeviceAddress deviceAddress
);

VULKAN_STRUCT VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
	const void* pNext,
	VkPipelineInputAssemblyStateCreateFlags flags,
	VkPrimitiveTopology topology,
	VkBool32 primitiveRestartEnable
);

VULKAN_STRUCT VkVertexInputBindingDescription vertexInputBindingDescription(
	uint32_t binding,
	uint32_t stride,
	VkVertexInputRate inputRate
);

VULKAN_STRUCT VkViewport viewport(float x,
	float y,
	float width,
	float height,
	float minDepth,
	float maxDepth);

VULKAN_STRUCT VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
	const void* pNext,
	VkPipelineRasterizationStateCreateFlags flags,
	VkBool32 depthClampEnable,
	VkBool32 rasterizerDiscardEnable,
	VkPolygonMode polygonMode,
	VkCullModeFlags cullMode,
	VkFrontFace frontFace,
	VkBool32 depthBiasEnable,
	float depthBiasConstantFactor,
	float depthBiasClamp,
	float depthBiasSlopeFactor,
	float lineWidth
);

VULKAN_STRUCT VkSubmitInfo submitInfo(
	const void* pNext,
	uint32_t waitSemaphoreCount,
	const VkSemaphore* pWaitSemaphores,
	const VkPipelineStageFlags* pWaitDstStageMask,
	uint32_t commandBufferCount,
	const VkCommandBuffer* pCommandBuffers,
	uint32_t signalSemaphoreCount,
	const VkSemaphore* pSignalSemaphores
);

VULKAN_STRUCT VkPresentInfoKHR presentInfoKHR(
	const void* pNext,
	uint32_t waitSemaphoreCount,
	const VkSemaphore* pWaitSemaphores,
	uint32_t swapchainCount,
	const VkSwapchainKHR* pSwapchains,
	const uint32_t* pImageIndices,
	VkResult* pResults);

VULKAN_STRUCT VkCommandBufferBeginInfo commandBufferBeginInfo(
	const void* pNext,
	VkCommandBufferUsageFlags flags,
	const VkCommandBufferInheritanceInfo* pInheritanceInfo
);

VULKAN_STRUCT VkRenderPassBeginInfo renderPassBeginInfo(
	const void* pNext,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	VkRect2D renderArea,
	uint32_t clearValueCount,
	const VkClearValue* pClearValues
);

VULKAN_STRUCT VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR(
	const void* pNext,
	uint32_t accelerationStructureCount,
	const VkAccelerationStructureKHR* pAccelerationStructures
);

VULKAN_STRUCT VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR(
	void* pNext,
	VkDeviceSize accelerationStructureSize,
	VkDeviceSize updateScratchSize,
	VkDeviceSize buildScratchSize
	);

}



