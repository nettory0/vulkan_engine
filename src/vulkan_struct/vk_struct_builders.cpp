#include "vk_struct_builders.h"
#include <map>
#include <stdexcept>
namespace vk_init
{

static inline std::map<void*, VolkDeviceTable> vkFunctionsTable;


VolkDeviceTable& volkTableAdd(void* device)
{
	return vkFunctionsTable[device];
}

void volkTableRemove(void* device)
{
	vkFunctionsTable.erase(device);
}

const VolkDeviceTable& volkTable(void* device)
{
	auto it = vkFunctionsTable.find(device);
	if(it != vkFunctionsTable.end())
		return it->second;

	// Not registered yet (e.g. device created outside CLogicalDeviceBase::create) -
	// load and cache a real table instead of handing back one full of null function pointers.
	VolkDeviceTable& table = vkFunctionsTable[device];
	volkLoadDeviceTable(&table, static_cast<VkDevice>(device));
	return table;
}


VkResult acquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
	return volkTable(device).vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}


void checkResult(VkResult result, const char* obj, const char* func)
{
	if(result != VK_SUCCESS)
	{
		printf("%s::%s vk error %d\n", obj, func, result);
		throw std::runtime_error("Failed to create Vulkan object!");
	}
}

VkBufferCreateInfo bufferCreateInfo(const void* pNext,
	VkBufferCreateFlags flags,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkSharingMode sharingMode,
	uint32_t queueFamilyIndexCount,
	const uint32_t* pQueueFamilyIndices)
{
	return VkBufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.size = size,
		.usage = usage,
		.sharingMode = sharingMode,
		.queueFamilyIndexCount = queueFamilyIndexCount,
		.pQueueFamilyIndices = pQueueFamilyIndices
	};
}
VkBufferCreateInfo bufferCreateInfoSimple(const void* pNext, VkDeviceSize size, VkBufferUsageFlags usage, const uint32_t* pQueueFamilyIndices)
{
	return bufferCreateInfo(pNext, 0, size, usage, VK_SHARING_MODE_EXCLUSIVE, 1, pQueueFamilyIndices);
}
VkWriteDescriptorSet writeDescriptorSet(const void* pNext,
	VkDescriptorSet dstSet,
	uint32_t dstBinding,
	uint32_t dstArrayElement,
	uint32_t descriptorCount,
	VkDescriptorType descriptorType,
	const VkDescriptorImageInfo* pImageInfo,
	const VkDescriptorBufferInfo* pBufferInfo,
	const VkBufferView* pTexelBufferView)
{
	return VkWriteDescriptorSet
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = pNext,
		.dstSet = dstSet,
		.dstBinding = dstBinding,
		.dstArrayElement = dstArrayElement,
		.descriptorCount = descriptorCount,
		.descriptorType = descriptorType,
		.pImageInfo = pImageInfo,
		.pBufferInfo = pBufferInfo,
		.pTexelBufferView = pTexelBufferView
	};
}
VkWriteDescriptorSet writeDescriptorSetBuffer(const void* pNext, VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorBufferInfo* pBufferInfo)
{
	return writeDescriptorSet(pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, pBufferInfo, nullptr);
}
VkWriteDescriptorSet writeDescriptorSetImage(const void* pNext, VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo)
{
	return writeDescriptorSet(pNext, dstSet, dstBinding, dstArrayElement, 1, descriptorType, pImageInfo, nullptr, nullptr);
}
VkImageMemoryBarrier imageMemoryBarrier(const void* pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImage image, VkImageSubresourceRange subresourceRange)
{
	return VkImageMemoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = pNext,
		.srcAccessMask = srcAccessMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = srcQueueFamilyIndex,
		.dstQueueFamilyIndex = dstQueueFamilyIndex,
		.image = image,
		.subresourceRange = subresourceRange
	};
}
VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding,
	VkDescriptorType descriptorType,
	uint32_t descriptorCount,
	VkShaderStageFlags stageFlags,
	const VkSampler* pImmutableSamplers)
{
	return VkDescriptorSetLayoutBinding
	{
		.binding = binding,
		.descriptorType = descriptorType,
		.descriptorCount = descriptorCount,
		.stageFlags = stageFlags,
		.pImmutableSamplers = pImmutableSamplers
	};
}
VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const void* pNext, VkDescriptorSetLayoutCreateFlags flags, uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings)
{
	return VkDescriptorSetLayoutCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.bindingCount = bindingCount,
		.pBindings = pBindings
	};
}
VkAttachmentDescription attachmentDescription(VkAttachmentDescriptionFlags flags, VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
	return VkAttachmentDescription
	{
		.flags = flags,
		.format = format,
		.samples = samples,
		.loadOp = loadOp,
		.storeOp = storeOp,
		.stencilLoadOp = stencilLoadOp,
		.stencilStoreOp = stencilStoreOp,
		.initialLayout = initialLayout,
		.finalLayout = finalLayout
	};
}
VkFramebufferCreateInfo framebufferCreateInfo(const void* pNext, VkFramebufferCreateFlags flags, VkRenderPass renderPass, uint32_t attachmentCount, const VkImageView* pAttachments, uint32_t width, uint32_t height, uint32_t layers)
{
	return VkFramebufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.renderPass = renderPass,
		.attachmentCount = attachmentCount,
		.pAttachments = pAttachments,
		.width = width,
		.height = height,
		.layers = layers
	};
}
VkBufferDeviceAddressInfo bufferDeviceAddressInfo(const void* pNext, VkBuffer buffer)
{
	return VkBufferDeviceAddressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = pNext, .buffer = buffer};
}
VkFenceCreateInfo fenceCreateInfo(const void* pNext, VkFenceCreateFlags flags)
{
	return VkFenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = pNext, .flags = flags};
}

VkSemaphoreCreateInfo semaphoreCreateInfo(const void* pNext, VkSemaphoreCreateFlags flags)
{
	return VkSemaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = pNext, .flags = flags};
}
VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const void* pNext, VkPipelineLayoutCreateFlags flags, uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts, uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges)
{
	return VkPipelineLayoutCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.setLayoutCount = setLayoutCount,
		.pSetLayouts = pSetLayouts,
		.pushConstantRangeCount = pushConstantRangeCount,
		.pPushConstantRanges = pPushConstantRanges
	};
}
VkMemoryAllocateInfo memoryAllocateInfo(const void* pNext, VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
{
	return VkMemoryAllocateInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .pNext = pNext, .allocationSize = allocationSize, .memoryTypeIndex = memoryTypeIndex};
}
VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
{
	return VkVertexInputAttributeDescription{.location = location, .binding = binding, .format = format, .offset = offset};
}
VkSwapchainCreateInfoKHR swapchainCreateInfoKHR(
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
	VkSwapchainKHR oldSwapchain)
{
	return VkSwapchainCreateInfoKHR{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = pNext,
		.flags = flags,
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = imageFormat,
		.imageColorSpace = imageColorSpace,
		.imageExtent = imageExtent,
		.imageArrayLayers = imageArrayLayers,
		.imageUsage = imageUsage,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndexCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.preTransform = preTransform,
		.compositeAlpha = compositeAlpha,
		.presentMode = presentMode,
		.clipped = clipped,
		.oldSwapchain = oldSwapchain};
}
VkImageCreateInfo imageCreateInfo(const void* pNext,
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
	uint32_t queueFamilyIndexCount,
	const uint32_t* pQueueFamilyIndices,
	VkImageLayout initialLayout)
{
	return VkImageCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.imageType = imageType,
		.format = format,
		.extent = extent,
		.mipLevels = mipLevels,
		.arrayLayers = arrayLayers,
		.samples = samples,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = sharingMode,
		.queueFamilyIndexCount = queueFamilyIndexCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.initialLayout = initialLayout
	};
}

VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const void* pNext, VkPipelineViewportStateCreateFlags flags, uint32_t viewportCount, const VkViewport* pViewports, uint32_t scissorCount, const VkRect2D* pScissors)
{
	return VkPipelineViewportStateCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = pNext, .flags = flags, .viewportCount = viewportCount, .pViewports = pViewports, .scissorCount = scissorCount, .pScissors = pScissors};
}
VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(const void* pNext, VkPipelineCreateFlags flags, uint32_t stageCount, const VkPipelineShaderStageCreateInfo* pStages, const VkPipelineVertexInputStateCreateInfo* pVertexInputState, const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState, const VkPipelineTessellationStateCreateInfo* pTessellationState, const VkPipelineViewportStateCreateInfo* pViewportState, const VkPipelineRasterizationStateCreateInfo* pRasterizationState, const VkPipelineMultisampleStateCreateInfo* pMultisampleState, const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState, const VkPipelineColorBlendStateCreateInfo* pColorBlendState, const VkPipelineDynamicStateCreateInfo* pDynamicState, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass, VkPipeline basePipelineHandle, int32_t basePipelineIndex)
{
	return VkGraphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.stageCount = stageCount,
		.pStages = pStages,
		.pVertexInputState = pVertexInputState,
		.pInputAssemblyState = pInputAssemblyState,
		.pTessellationState = pTessellationState,
		.pViewportState = pViewportState,
		.pRasterizationState = pRasterizationState,
		.pMultisampleState = pMultisampleState,
		.pDepthStencilState = pDepthStencilState,
		.pColorBlendState = pColorBlendState,
		.pDynamicState = pDynamicState,
		.layout = layout,
		.renderPass = renderPass,
		.subpass = subpass,
		.basePipelineHandle = basePipelineHandle,
		.basePipelineIndex = basePipelineIndex
	};
}

VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkBool32 blendEnable, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp, VkColorComponentFlags colorWriteMask)
{
	return VkPipelineColorBlendAttachmentState{
		.blendEnable = blendEnable,
		.srcColorBlendFactor = srcColorBlendFactor,
		.dstColorBlendFactor = dstColorBlendFactor,
		.colorBlendOp = colorBlendOp,
		.srcAlphaBlendFactor = srcAlphaBlendFactor,
		.dstAlphaBlendFactor = dstAlphaBlendFactor,
		.alphaBlendOp = alphaBlendOp,
		.colorWriteMask = colorWriteMask
	};
}

VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(const void* pNext, VkPipelineColorBlendStateCreateFlags flags, VkBool32 logicOpEnable, VkLogicOp logicOp, uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments, float blendConstants[4])
{
	return VkPipelineColorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.logicOpEnable = logicOpEnable,
		.logicOp = logicOp,
		.attachmentCount = attachmentCount,
		.pAttachments = pAttachments,
		.blendConstants = {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]}
	};
}

VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(const void* pNext, VkPipelineVertexInputStateCreateFlags flags, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription* pVertexAttributeDescriptions)
{
	return VkPipelineVertexInputStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.vertexBindingDescriptionCount = vertexBindingDescriptionCount,
		.pVertexBindingDescriptions = pVertexBindingDescriptions,
		.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount,
		.pVertexAttributeDescriptions = pVertexAttributeDescriptions
	};
}

VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(const void* pNext, VkPipelineDepthStencilStateCreateFlags flags,
	VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp,
	VkBool32 depthBoundsTestEnable, VkBool32 stencilTestEnable, VkStencilOpState front, VkStencilOpState back, float minDepthBounds, float maxDepthBounds)
{
	return VkPipelineDepthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.depthTestEnable = depthTestEnable,
		.depthWriteEnable = depthWriteEnable,
		.depthCompareOp = depthCompareOp,
		.depthBoundsTestEnable = depthBoundsTestEnable,
		.stencilTestEnable = stencilTestEnable,
		.front = front,
		.back = back,
		.minDepthBounds = minDepthBounds,
		.maxDepthBounds = maxDepthBounds
	};
}

VkImageViewCreateInfo imageViewCreateInfo(const void* pNext, VkImageViewCreateFlags flags, VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange)
{
	return VkImageViewCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.image = image,
		.viewType = viewType,
		.format = format,
		.components = components,
		.subresourceRange = subresourceRange};
}

VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfoKHR(const void* pNext, VkRayTracingShaderGroupTypeKHR type, uint32_t generalShader, uint32_t closestHitShader, uint32_t anyHitShader, uint32_t intersectionShader, const void* pShaderGroupCaptureReplayHandle)
{
	return VkRayTracingShaderGroupCreateInfoKHR{.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		.pNext = pNext,
		.type = type,
		.generalShader = generalShader,
		.closestHitShader = closestHitShader,
		.anyHitShader = anyHitShader,
		.intersectionShader = intersectionShader,
		.pShaderGroupCaptureReplayHandle = pShaderGroupCaptureReplayHandle};
}

VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(const void* pNext, VkPipelineShaderStageCreateFlags flags, VkShaderStageFlagBits stage, VkShaderModule module, const char* pName, const VkSpecializationInfo* pSpecializationInfo)
{
	return VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .flags = flags, .stage = stage, .module = module, .pName = pName, .pSpecializationInfo = pSpecializationInfo};
}

VkShaderModuleCreateInfo shaderModuleCreateInfo(const void* pNext, VkShaderModuleCreateFlags flags, size_t codeSize, const uint32_t* pCode)
{
	return VkShaderModuleCreateInfo{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pNext = pNext, .flags = flags, .codeSize = codeSize, .pCode = pCode};
}

VkComputePipelineCreateInfo computePipelineCreateInfo(const void* pNext, VkPipelineCreateFlags flags, VkPipelineShaderStageCreateInfo stage, VkPipelineLayout layout, VkPipeline basePipelineHandle, int32_t basePipelineIndex)
{
	return VkComputePipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.stage = stage,
		.layout = layout,
		.basePipelineHandle = basePipelineHandle,
		.basePipelineIndex = basePipelineIndex
	};
}

VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(const void* pNext, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts)
{
	return VkDescriptorSetAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = pNext,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = descriptorSetCount,
		.pSetLayouts = pSetLayouts
	};
}

VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR(const void* pNext, VkGeometryTypeKHR geometryType, VkAccelerationStructureGeometryDataKHR geometry, VkGeometryFlagsKHR flags)
{
	return VkAccelerationStructureGeometryKHR{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = pNext,
		.geometryType = geometryType,
		.geometry = geometry,
		.flags = flags
	};
}

VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR(const void* pNext,
	VkAccelerationStructureTypeKHR type,
	VkBuildAccelerationStructureFlagsKHR flags,
	VkBuildAccelerationStructureModeKHR mode,
	VkAccelerationStructureKHR srcAccelerationStructure,
	VkAccelerationStructureKHR dstAccelerationStructure,
	uint32_t geometryCount,
	const VkAccelerationStructureGeometryKHR* pGeometries,
	const VkAccelerationStructureGeometryKHR* const* ppGeometries,
	VkDeviceOrHostAddressKHR scratchData)
{
	return VkAccelerationStructureBuildGeometryInfoKHR{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.pNext = pNext,
		.type = type,
		.flags = flags,
		.mode = mode,
		.srcAccelerationStructure = srcAccelerationStructure,
		.dstAccelerationStructure = dstAccelerationStructure,
		.geometryCount = geometryCount,
		.pGeometries = pGeometries,
		.ppGeometries = ppGeometries,
		.scratchData = scratchData};
}

VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const void* pNext, VkDescriptorPoolCreateFlags flags, uint32_t maxSets, uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes)
{
	return VkDescriptorPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.maxSets = maxSets,
		.poolSizeCount = poolSizeCount,
		.pPoolSizes = pPoolSizes
	};
}

VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfoKHR(const void* pNext,
	VkAccelerationStructureCreateFlagsKHR createFlags,
	VkBuffer buffer,
	VkDeviceSize offset,
	VkDeviceSize size,
	VkAccelerationStructureTypeKHR type,
	VkDeviceAddress deviceAddress)
{
	return VkAccelerationStructureCreateInfoKHR{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = pNext,
		.createFlags = createFlags,
		.buffer = buffer,
		.offset = offset,
		.size = size,
		.type = type,
		.deviceAddress = deviceAddress
	};
}

VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(const void* pNext, VkPipelineInputAssemblyStateCreateFlags flags, VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable)
{
	return VkPipelineInputAssemblyStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = pNext,
		.flags = flags,
		.topology = topology,
		.primitiveRestartEnable = primitiveRestartEnable
	};
}

VkVertexInputBindingDescription vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	return VkVertexInputBindingDescription{.binding = binding, .stride = stride, .inputRate = inputRate};
}

VkViewport viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	return VkViewport{.x = x, .y = y, .width = width, .height = height, .minDepth = minDepth, .maxDepth = maxDepth};
}

VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(const void* pNext,
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
	float lineWidth)
{
	return VkPipelineRasterizationStateCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.flags = flags,
		.depthClampEnable = depthClampEnable,
		.rasterizerDiscardEnable = rasterizerDiscardEnable,
		.polygonMode = polygonMode,
		.cullMode = cullMode,
		.frontFace = frontFace,
		.depthBiasEnable = depthBiasEnable,
		.depthBiasConstantFactor = depthBiasConstantFactor,
		.depthBiasClamp = depthBiasClamp,
		.depthBiasSlopeFactor = depthBiasSlopeFactor,
		.lineWidth = lineWidth
	};
}

VkSubmitInfo submitInfo(const void* pNext, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, const VkPipelineStageFlags* pWaitDstStageMask, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers, uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores)
{
	return VkSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = pNext,
		.waitSemaphoreCount = waitSemaphoreCount,
		.pWaitSemaphores = pWaitSemaphores,
		.pWaitDstStageMask = pWaitDstStageMask,
		.commandBufferCount = commandBufferCount,
		.pCommandBuffers = pCommandBuffers,
		.signalSemaphoreCount = signalSemaphoreCount,
		.pSignalSemaphores = pSignalSemaphores
	};
}

VkPresentInfoKHR presentInfoKHR(
	const void* pNext,
	uint32_t waitSemaphoreCount,
	const VkSemaphore* pWaitSemaphores,
	uint32_t swapchainCount,
	const VkSwapchainKHR* pSwapchains,
	const uint32_t* pImageIndices,
	VkResult* pResults)
{
	return VkPresentInfoKHR
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = pNext,
		.waitSemaphoreCount = waitSemaphoreCount,
		.pWaitSemaphores = pWaitSemaphores,
		.swapchainCount = swapchainCount,
		.pSwapchains = pSwapchains,
		.pImageIndices = pImageIndices,
		.pResults = pResults
	};
}

VkCommandBufferBeginInfo commandBufferBeginInfo(const void* pNext, VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
	return VkCommandBufferBeginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = pNext, .flags = flags, .pInheritanceInfo = pInheritanceInfo};
}

VkRenderPassBeginInfo renderPassBeginInfo(const void* pNext, VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, uint32_t clearValueCount, const VkClearValue* pClearValues)
{
	return VkRenderPassBeginInfo{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .pNext = pNext, .renderPass = renderPass, .framebuffer = framebuffer, .renderArea = renderArea, .clearValueCount = clearValueCount, .pClearValues = pClearValues};
}

VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR(const void* pNext, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures)
{
	return VkWriteDescriptorSetAccelerationStructureKHR{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, .pNext = pNext, .accelerationStructureCount = accelerationStructureCount, .pAccelerationStructures = pAccelerationStructures};
}

VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR(void* pNext, VkDeviceSize accelerationStructureSize, VkDeviceSize updateScratchSize, VkDeviceSize buildScratchSize)
{
	return VkAccelerationStructureBuildSizesInfoKHR{
	.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	.pNext = pNext,
	.accelerationStructureSize = accelerationStructureSize, 
	.updateScratchSize = updateScratchSize, 
	.buildScratchSize = buildScratchSize
	};
}

}

