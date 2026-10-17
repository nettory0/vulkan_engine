#include <stdexcept>
#include "en_gl_descriptor_manager.h"
#include "vk_struct_builders.h"
#include <glm/gtc/type_ptr.hpp>

namespace en_vk
{

vk_structs::CUniformBase* CResourceManager::uniform(uint32_t i) const
{

	return m_storage.uniform(i);
}

vk_structs::CTexture createTexture(const VkFormat& format, uint32_t present, vk_base::CQueue& queue, const vk_structs::SSupportDetails& details, VkDevice device, vk_structs::CCommandPool* pCommandPool)
{
	vk_structs::CTexture result;

	VkImageCreateInfo rayTraceImageCreateInfo = {
vk_init::imageCreateInfo(nullptr,
	0,
	VK_IMAGE_TYPE_2D,
	format,//m_details.m_swapChainInfo.m_formats[0].format,
	{.width = details.m_swapChainInfo.m_capabilities.currentExtent.width, .height = details.m_swapChainInfo.m_capabilities.currentExtent.height, .depth = 1},
	1,
	1,
	VK_SAMPLE_COUNT_1_BIT,
	VK_IMAGE_TILING_OPTIMAL,
	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	VK_SHARING_MODE_EXCLUSIVE,
	1,
	&present,//m_context.queueFamilyIndices().m_presentFamily.value(),
	VK_IMAGE_LAYOUT_UNDEFINED)};

	result.m_texture.create(device, details, rayTraceImageCreateInfo, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


	VkImageViewCreateInfo rayTraceImageViewCreateInfo = {
		vk_init::imageViewCreateInfo(
			nullptr,
			0,
			result.m_texture.handle(),
			VK_IMAGE_VIEW_TYPE_2D,
			format,
			{.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
			{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1})
	};

	result.m_texture.createImageView(device, rayTraceImageViewCreateInfo);

	// =========================================================================
	// Ray Trace Image Barrier
	// (VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_GENERAL)

	VkImageMemoryBarrier rayTraceGeneralMemoryBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL,
		.srcQueueFamilyIndex = present,
		.dstQueueFamilyIndex = present,
		.image = result.m_texture.handle(),
		.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1}};

	auto command = pCommandPool->beginSingleTimeCommands();
	command.pipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &rayTraceGeneralMemoryBarrier);
	pCommandPool->endSingleTimeCommands(command, queue);

	return result;
}

void CResourceFactory::init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice, vk_structs::CCommandPool* pCommandPool)
{
	m_device = device;
	m_details = details;
	m_physicalDevice = physicalDevice;
	m_pCommandPool = pCommandPool;
}

vk_structs::CBuffer CResourceFactory::createBufferResource(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlagBits, const uint32_t* pQueueFamilyIndices, VkMemoryPropertyFlags memoryPropertyFlags, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo) const
{
	VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, bufferSize, bufferUsageFlagBits, VK_SHARING_MODE_EXCLUSIVE, 0, pQueueFamilyIndices)};
	vk_structs::CBuffer result;
	result.create(m_device, m_details, bufferInfo, memoryAllocateFlagsInfo, memoryPropertyFlags);
	return  result;
}

vk_structs::CTexture CResourceFactory::createTextureResourceRt(const VkFormat& format, uint32_t present, vk_base::CQueue& queue) const
{
	vk_structs::CTexture texture = createTexture(format, present, queue, m_details, m_device, m_pCommandPool);
	return texture;
}

vk_structs::CTexture CResourceFactory::createTextureResource(const std::vector<unsigned char>& pixels, int texWidth, int texHeight, int texChannels, vk_base::CQueue& queue)
{
	vk_structs::CTexture texture;
	texture.createTextureImage(m_device, m_physicalDevice, pixels, texWidth, texHeight, texChannels, *m_pCommandPool, queue, m_details);
	return texture;
}

vk_structs::CBlas CResourceFactory::createBottomLevel(VkDevice device, const vk_structs::CBuffer& vertexBuffer, const vk_structs::CBuffer& indexBuffer, const SIndexVertexBuffers& geom, const VkMemoryAllocateFlagsInfo& memoryAllocateFlagsInfo, uint32_t present, const vk_structs::SSupportDetails& details, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue)
{
	vk_structs::CBlas blas;
	VkAccelerationStructureGeometryDataKHR bottomLevelAccelerationStructureGeometryData = {
		.triangles = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = NULL,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = {.deviceAddress = vertexBuffer.adress()},
			.vertexStride = sizeof(SVertexRT),
			.maxVertex = geom.m_maxVertex,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = {.deviceAddress = indexBuffer.adress()},
			.transformData = {.deviceAddress = 0}}};

	VkAccelerationStructureGeometryKHR bottomLevelAccelerationStructureGeometry = vk_init::accelerationStructureGeometryKHR(nullptr, VK_GEOMETRY_TYPE_TRIANGLES_KHR, bottomLevelAccelerationStructureGeometryData, VK_GEOMETRY_OPAQUE_BIT_KHR);


	VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo = vk_init::accelerationStructureBuildGeometryInfoKHR(nullptr,
																																					   VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
																																					   0,
																																					   VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
																																					   VK_NULL_HANDLE,
																																					   VK_NULL_HANDLE,
																																					   1,
																																					   &bottomLevelAccelerationStructureGeometry,
																																					   NULL,
																																					   {.deviceAddress = 0}
	);
	VkMemoryAllocateFlagsInfo allocFlags{};
	allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	VkAccelerationStructureBuildSizesInfoKHR  bottomLevelAccelerationStructureBuildSizesInfo{vk_init::accelerationStructureBuildSizesInfoKHR(nullptr, 0, 0, 0)};
	std::vector<uint32_t> bottomLevelMaxPrimitiveCountList = {geom.m_primitiveCount};


	vk_init::volkTable(device).vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomLevelAccelerationStructureBuildGeometryInfo, bottomLevelMaxPrimitiveCountList.data(), &bottomLevelAccelerationStructureBuildSizesInfo);

	auto usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo bottomLevelAccelerationStructureBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, usage, &present);

	blas.m_asBuffer.create(device, details, bottomLevelAccelerationStructureBufferCreateInfo, &allocFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkAccelerationStructureCreateInfoKHR bottomLevelAccelerationStructureCreateInfo = vk_init::accelerationStructureCreateInfoKHR(NULL, 0, blas.m_asBuffer.handle(), 0, bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, 0);


	vk_init::volkTable(device).vkCreateAccelerationStructureKHR(device, &bottomLevelAccelerationStructureCreateInfo, NULL, &blas.m_accelerationStructureHandle);

	// =========================================================================
	 // Build Bottom Level Acceleration Structure

	VkAccelerationStructureDeviceAddressInfoKHR bottomLevelAccelerationStructureDeviceAddressInfo = {
		.sType =
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.pNext = NULL,
		.accelerationStructure = blas.m_accelerationStructureHandle};

	blas.m_accelerationStructureDeviceAddress = vk_init::volkTable(device).vkGetAccelerationStructureDeviceAddressKHR(device, &bottomLevelAccelerationStructureDeviceAddressInfo);

	VkBufferCreateInfo bottomLevelAccelerationStructureScratchBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, bottomLevelAccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &present);
	vk_structs::CBuffer asScratchBuffer;
	asScratchBuffer.create(device, details, bottomLevelAccelerationStructureScratchBufferCreateInfo, &memoryAllocateFlagsInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


	bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = blas.m_accelerationStructureHandle;

	bottomLevelAccelerationStructureBuildGeometryInfo.scratchData = {.deviceAddress = asScratchBuffer.adress()};

	VkAccelerationStructureBuildRangeInfoKHR  bottomLevelAccelerationStructureBuildRangeInfo = geom.m_primitive;

	const VkAccelerationStructureBuildRangeInfoKHR* bottomLevelAccelerationStructureBuildRangeInfos = &bottomLevelAccelerationStructureBuildRangeInfo;

	vk_base::CCommandBuffer command{commandPool.beginSingleTimeCommands()};
	command.buildAccelerationStructuresKHR(1, &bottomLevelAccelerationStructureBuildGeometryInfo, &bottomLevelAccelerationStructureBuildRangeInfos);
	commandPool.endSingleTimeCommands(command, queue);
	asScratchBuffer.clean();
	return blas;
}

vk_structs::CTlas CResourceFactory::createTopLevel(VkDevice device, const std::vector<VkDeviceAddress>& aBottomLevelAccelerationStructureDeviceAddress, const VkMemoryAllocateFlagsInfo& memoryAllocateFlagsInfo, vk_base::CQueue& queue, uint32_t present, const vk_structs::SSupportDetails& details, vk_structs::CCommandPool& commandPool)
{
	vk_structs::CTlas tlas;
	glm::mat4 mat(1);
	//mat = glm::translate(mat, glm::vec3(1000, 100, 1000));
	//mat = glm::scale(mat, glm::vec3(0.6));

	std::vector<VkAccelerationStructureInstanceKHR> aBottomLevelAccelerationStructureInstance;

	for(auto& blas : aBottomLevelAccelerationStructureDeviceAddress)
	{
		VkAccelerationStructureInstanceKHR bottomLevelAccelerationStructureInstance{
			.instanceCustomIndex = 0,
			.mask = 0xFF,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = aBottomLevelAccelerationStructureDeviceAddress[0]
		};
		memcpy(bottomLevelAccelerationStructureInstance.transform.matrix, glm::value_ptr(mat), sizeof(float) * 3 * 4);
		bottomLevelAccelerationStructureInstance.transform.matrix[0][3] = 0;
		bottomLevelAccelerationStructureInstance.transform.matrix[1][3] = 0;
		bottomLevelAccelerationStructureInstance.transform.matrix[2][3] = 0;
		aBottomLevelAccelerationStructureInstance.push_back(bottomLevelAccelerationStructureInstance);
	}

	VkBufferCreateInfo bottomLevelGeometryInstanceBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &present);

	tlas.m_instanceBuffer.create(device, details, bottomLevelGeometryInstanceBufferCreateInfo, &memoryAllocateFlagsInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	tlas.m_instanceBuffer.load(aBottomLevelAccelerationStructureInstance.size() * sizeof(VkAccelerationStructureInstanceKHR), aBottomLevelAccelerationStructureInstance.data(), 0);


	const VkAccelerationStructureGeometryDataKHR topLevelAccelerationStructureGeometryData =
	{.instances = {
		.sType =
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
		.pNext = NULL,
		.arrayOfPointers = VK_FALSE,
		.data = {.deviceAddress =
		 tlas.m_instanceBuffer.adress()}}};

	const VkAccelerationStructureGeometryKHR topLevelAccelerationStructureGeometry = vk_init::accelerationStructureGeometryKHR(nullptr, VK_GEOMETRY_TYPE_INSTANCES_KHR, topLevelAccelerationStructureGeometryData, VK_GEOMETRY_OPAQUE_BIT_KHR);

	VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo = vk_init::accelerationStructureBuildGeometryInfoKHR(
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		0,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		1,
		&topLevelAccelerationStructureGeometry,
		NULL,
		{.deviceAddress = 0});


	VkAccelerationStructureBuildSizesInfoKHR topLevelAccelerationStructureBuildSizesInfo{vk_init::accelerationStructureBuildSizesInfoKHR(nullptr, 0, 0, 0)};

	std::vector<uint32_t> topLevelMaxPrimitiveCountList = {(uint32_t)aBottomLevelAccelerationStructureInstance.size()};

	vk_init::volkTable(device).vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &topLevelAccelerationStructureBuildGeometryInfo, topLevelMaxPrimitiveCountList.data(), &topLevelAccelerationStructureBuildSizesInfo);

	VkMemoryAllocateFlagsInfo flagsInfo{};
	flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkBufferCreateInfo topLevelAccelerationStructureBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &present);

	tlas.m_asBuffer.create(device, details, topLevelAccelerationStructureBufferCreateInfo, &flagsInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


	const VkAccelerationStructureCreateInfoKHR topLevelAccelerationStructureCreateInfo = vk_init::accelerationStructureCreateInfoKHR(
		NULL,
		0,
		tlas.m_asBuffer.handle(),
		0,
		topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		0);


	vk_init::volkTable(device).vkCreateAccelerationStructureKHR(device, &topLevelAccelerationStructureCreateInfo, NULL, &tlas.m_accelerationStructureHandle);

	const VkAccelerationStructureDeviceAddressInfoKHR topLevelAccelerationStructureDeviceAddressInfo = {
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.pNext = NULL,
		.accelerationStructure = tlas.m_accelerationStructureHandle};
	vk_structs::CBuffer asScratchBuffer;
	const VkDeviceAddress topLevelAccelerationStructureDeviceAddress = vk_init::volkTable(device).vkGetAccelerationStructureDeviceAddressKHR(device, &topLevelAccelerationStructureDeviceAddressInfo);
	const VkBufferCreateInfo topLevelAccelerationStructureScratchBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, topLevelAccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &present);
	asScratchBuffer.create(device, details, topLevelAccelerationStructureScratchBufferCreateInfo, &memoryAllocateFlagsInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	topLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = tlas.m_accelerationStructureHandle;
	topLevelAccelerationStructureBuildGeometryInfo.scratchData = {.deviceAddress = asScratchBuffer.adress()};
	const VkAccelerationStructureBuildRangeInfoKHR  topLevelAccelerationStructureBuildRangeInfo{.primitiveCount = 1, .primitiveOffset = 0, .firstVertex = 0, .transformOffset = 0};
	const VkAccelerationStructureBuildRangeInfoKHR* topLevelAccelerationStructureBuildRangeInfos = &topLevelAccelerationStructureBuildRangeInfo;


	auto command_1 = commandPool.beginSingleTimeCommands();
	command_1.buildAccelerationStructuresKHR(1, &topLevelAccelerationStructureBuildGeometryInfo, &topLevelAccelerationStructureBuildRangeInfos);
	commandPool.endSingleTimeCommands(command_1, queue);
	asScratchBuffer.clean();
	return tlas;
}

uint32_t CResourceManager::createBlasResource(VkDescriptorType descriptorType, int destinationBinding)
{
	auto resourceId = m_storage.addBlas(descriptorType, destinationBinding);
	auto* pNewAdded = static_cast<vk_structs::CUniformBlas*>(uniform(resourceId));
	if(!pNewAdded)
		throw std::runtime_error("Failed to cast to CUniformBlas");
	return resourceId;
}

void CResourceStorage::destroy()
{
	for(auto& [index, buffer] : m_mapResources.resource())
		buffer->destroy();
	m_mapResources.clear();
}

void CResourceManager::destroy()
{
	m_storage.destroy();
}

void CDescriptorManager::init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice)
{
	m_device = device;
	m_details = details;
	m_physicalDevice = physicalDevice;
}

void CDescriptorManager::addDescriptor(uint64_t idElement,
									   uint32_t frame,
									   uint32_t index,
									   VkDescriptorSetLayout descriptorSetLayout,
									   std::map<uint32_t, SUniform>&& binding)
{
	auto [it, inserted] = m_mapFrameDescriptor.try_emplace(SFrameKey{idElement, frame});

	if(!it->second.m_mapFrameDescriptorToBinding)
	{
		it->second.m_mapFrameDescriptorToBinding = std::make_unique<SFrameDescriptorBindings>();
	}

	std::vector<VkWriteDescriptorSet> aWriteDescriptorSetIn;

	for(auto& bind : binding)
		aWriteDescriptorSetIn.push_back(bind.second.m_uniform->writeDescriptorSet());

	it->second.m_mapFrameDescriptorToBinding->addDescriptor(index, descriptorSetLayout, std::move(binding), m_physicalDevice, m_device, aWriteDescriptorSetIn, m_details);
}

void CDescriptorManager::addDescriptor(uint64_t idElement,
									   uint32_t frame,
									   uint32_t index,
									   VkDescriptorSetLayout descriptorSetLayout,
									   std::map<uint32_t, SUniform>&& binding,
									   const std::vector < VkWriteDescriptorSet>& aWriteDescriptorSetIn)
{
	auto [it, inserted] = m_mapFrameDescriptor.try_emplace(SFrameKey{idElement, frame});

	if(!it->second.m_mapFrameDescriptorToBinding)
	{
		it->second.m_mapFrameDescriptorToBinding = std::make_unique<SFrameDescriptorBindings>();
	}

	it->second.m_mapFrameDescriptorToBinding->addDescriptor(index, descriptorSetLayout, std::move(binding), m_physicalDevice, m_device, aWriteDescriptorSetIn, m_details);
}

SEntityDescriptorSetsForBinding CDescriptorManager::getDescriptors(uint64_t idElement) const
{
	SEntityDescriptorSetsForBinding result;
	for(auto& [key, resource] : m_mapFrameDescriptor)
	{
		if(key.m_entity == idElement)
		{
			auto& res = result.m_mapDescriptor[key.m_frame];

			for(auto& [index, resDescriptor] : resource.m_mapFrameDescriptorToBinding->m_mapDescriptorToBinding)
			{
				res.push_back(resDescriptor->descriptorBufferBindingInfo());
			}
		}
	}

	return result;
}

SFrameDescriptorBindings* CDescriptorManager::getDescriptorsBinding(uint64_t idElement, uint32_t frame) const
{
	SFrameKey keyElem{idElement, frame};
	if(auto it = m_mapFrameDescriptor.find(keyElem); it != m_mapFrameDescriptor.end())
	{
		return it->second.m_mapFrameDescriptorToBinding.get();
	}
	return nullptr;
}

vk_structs::CUniformBase* CResourceStorage::uniform(uint32_t uniformId) const
{
	return m_mapResources.get(uniformId);
}

void CDescriptorManager::destroy()
{
	clearDescriptors();
	m_mapFrameDescriptor.clear();
}

void CDescriptorManager::clearDescriptors()
{
	for(auto& [id, resource] : m_mapFrameDescriptor)
	{
		for(auto& [idDescr, descriptor] : resource.m_mapFrameDescriptorToBinding->m_mapDescriptorToBinding)
		{
			descriptor->destroy();
		}
	}
}

uint32_t CResourceStorage::addBlas(VkDescriptorType descriptorType, int destinationBinding)
{
	return m_mapResources.add(std::make_unique<vk_structs::CUniformBlas>(descriptorType, destinationBinding));
}

VkDescriptorSetLayout CDescriptorLayoutCache::getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType type, uint32_t descriptorNum) const
{
	return m_mapPipelineLayoutSchema.at({type, descriptorNum}).m_layout;
}

void CDescriptorLayoutCache::setPipelineDescriptorLayouts(const std::map<en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo>& mapSchema)
{
	m_mapPipelineLayoutSchema = mapSchema;
}

VkDescriptorType CDescriptorLayoutCache::getDescriptorType(en_vk_utils::EPipelineLayoutType type, uint32_t descriptorNum, uint32_t binding) const
{
	return m_mapPipelineLayoutSchema.at({type, descriptorNum}).m_mapDescriptorType.at(binding);
}

void SFrameDescriptorBindings::addDescriptor(uint32_t index, 
											 VkDescriptorSetLayout descriptorSetLayout,
											 std::map<uint32_t, SUniform>&& binding,
											 VkPhysicalDevice physicalDevice,
											 VkDevice device,
											 const std::vector<VkWriteDescriptorSet>& aWriteDescriptorSetIn,
											 const vk_structs::SSupportDetails& details)
{
	m_mapDescriptorToBinding[index] = std::make_unique<vk_structs::CDescriptorBufferResource>();
	m_mapDescriptorToBinding[index]->create(physicalDevice, device, descriptorSetLayout, aWriteDescriptorSetIn, details);
	
	
	for(auto& [resourceIndex, uniform] : binding)
	{
		m_resourceIndex[{index, resourceIndex}] = std::move(uniform);
	}
	
	
}

}