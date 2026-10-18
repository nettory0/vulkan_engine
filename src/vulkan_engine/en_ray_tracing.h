#pragma once
#include "vk_objects.h"
#include "en_gl_utils.h"

namespace en_vk
{

class CVulkanContext;

class CRayTracingSystem
{
public:
	CRayTracingSystem(en_vk::CVulkanContext& context): m_context{context}, m_bufferRT()
	{
	};
	~CRayTracingSystem() = default;

	void shaderBindingTable(VkDevice device, VkPipeline pipelineRt, vk_structs::CPhysicalDevice& physicalDevice, uint32_t presentQueue, vk_structs::SSupportDetails details);

	void clear(VkDevice device);

	VkStridedDeviceAddressRegionKHR rchitShaderBindingTable;
	VkStridedDeviceAddressRegionKHR rgenShaderBindingTable;
	VkStridedDeviceAddressRegionKHR rmissShaderBindingTable;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR physicalDeviceRayTracingPipelineProperties;
	vk_structs::CBuffer m_bufferRT;
	std::vector<en_vk_utils::SMaterial> m_materialList;

	en_vk::CVulkanContext& m_context;
};
}