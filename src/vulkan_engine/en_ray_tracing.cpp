#include "en_ray_tracing.h"
#include "en_animation_structs.h"
#include "en_render_system.h"

namespace en_vk
{

void CRayTracingSystem::shaderBindingTable(VkDevice device, VkPipeline pipelineRt, vk_structs::CPhysicalDevice& physicalDevice, uint32_t presentQueue, vk_structs::SSupportDetails details)
{
	physicalDeviceRayTracingPipelineProperties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR, .pNext = NULL};
	VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &physicalDeviceRayTracingPipelineProperties};
	physicalDevice.physicalDeviceProperties2(physicalDeviceProperties2);

	VkDeviceSize progSize = physicalDeviceRayTracingPipelineProperties.shaderGroupBaseAlignment;

	VkDeviceSize shaderBindingTableSize = progSize * 4;
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = NULL, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0};
	VkBufferCreateInfo shaderBindingTableBufferCreateInfo = vk_init::bufferCreateInfoSimple(nullptr, shaderBindingTableSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &presentQueue);

	m_bufferRT.create(device, details , shaderBindingTableBufferCreateInfo, &memoryAllocateFlagsInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


	std::vector<char> shaderHandleBuffer(shaderBindingTableSize);

	vk_init::volkTable(device).vkGetRayTracingShaderGroupHandlesKHR(device, pipelineRt, 0, 4, shaderBindingTableSize, shaderHandleBuffer.data());

	std::vector<char> tempBuffer(shaderBindingTableSize);
	char* pBeginBuffer = tempBuffer.data();

	for(uint32_t x = 0; x < 4; x++)
	{
		memcpy(pBeginBuffer, shaderHandleBuffer.data() + x * physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize, physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize);
		pBeginBuffer = reinterpret_cast<char*>(pBeginBuffer) + physicalDeviceRayTracingPipelineProperties.shaderGroupBaseAlignment;
	}
	m_bufferRT.load(tempBuffer.size(), tempBuffer.data(), 0);

	VkDeviceSize hitGroupOffset = 0u * progSize;
	VkDeviceSize rayGenOffset = 1u * progSize;
	VkDeviceSize missOffset = 2u * progSize;

	rchitShaderBindingTable = {.deviceAddress = m_bufferRT.adress() + hitGroupOffset, .stride = progSize, .size = progSize};
	rgenShaderBindingTable = {.deviceAddress = m_bufferRT.adress() + rayGenOffset, .stride = progSize, .size = progSize};
	rmissShaderBindingTable = {.deviceAddress = m_bufferRT.adress() + missOffset, .stride = progSize, .size = progSize * 2};
	const VkStridedDeviceAddressRegionKHR callableShaderBindingTable = {};

}

void CRayTracingSystem::clear(VkDevice device)
{
}

}