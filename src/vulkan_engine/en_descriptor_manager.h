#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include "vk_objects.h"
#include "en_graphics_pipeline.h"
#include "en_common_utils.h"
#include "en_utils.h"

namespace en_model
{

struct STexture;
}

namespace en_vk
{

struct SEntityDescriptorSets
{
	en_vk_utils::EPipelineLayoutType m_pipelineLayoutType;
	uint32_t m_meshId{0};
	uint32_t m_textureId{0};
	size_t m_bufferSize{0};
	size_t m_bufferSize2{0};
};

struct SEntityDescriptorSetsForBinding
{
	std::map<int, std::vector<VkDescriptorBufferBindingInfoEXT>> m_mapDescriptor;
};

struct SUniform
{
	std::unique_ptr<vk_structs::CUniformBase> m_uniform;
};

struct SDescriptorResourceIndex
{
	uint32_t m_descriptorIndex{0};
	uint32_t m_resourceIndex{0};
	bool operator==(const SDescriptorResourceIndex& other) const noexcept
	{
		return m_descriptorIndex == other.m_descriptorIndex && m_resourceIndex == other.m_resourceIndex;
	}

	bool operator<(const SDescriptorResourceIndex& other) const noexcept
	{
		if(m_descriptorIndex != other.m_descriptorIndex)
			return m_descriptorIndex < other.m_descriptorIndex;
		return m_resourceIndex < other.m_resourceIndex;
	}
};

struct SFrameDescriptorBindings
{
	std::map< SDescriptorResourceIndex, SUniform> m_resourceIndex;
	std::map<uint32_t, std::unique_ptr<vk_structs::CDescriptorBufferResource>> m_mapDescriptorToBinding;
	void addDescriptor(uint32_t index, VkDescriptorSetLayout descriptorSetLayout, 
					   std::map<uint32_t, SUniform>&& binding,
					   VkPhysicalDevice physicalDevice,
					   VkDevice device,
					   const std::vector<VkWriteDescriptorSet>& writes,
					   const vk_structs::SSupportDetails& details);
};

struct SFrameSetDescriptorBindings
{
	std::unique_ptr<SFrameDescriptorBindings> m_mapFrameDescriptorToBinding;
};

struct SFrameKey
{
	uint64_t m_entity{0};
	uint32_t m_frame{0};
	bool operator==(const SFrameKey& other) const noexcept
	{
		return m_entity == other.m_entity && m_frame == other.m_frame;
	}
};

struct SFrameKeyHash
{
	size_t operator()(const SFrameKey& frameKey) const noexcept
	{
		size_t seed = std::hash<uint64_t>{}(frameKey.m_entity);
		seed ^= std::hash<uint32_t>{}(frameKey.m_frame) + 0x9e3779b9 + (seed << 6)+ (seed >> 2);
		return seed;
	}
};

class CResourceStorage
{
public:
	vk_structs::CUniformBase* uniform(uint32_t i) const;
	void destroy();
	uint32_t addBlas(VkDescriptorType descriptorType, int destinationBinding);

private:
	en_util::CResourceKeeper<vk_structs::CUniformBase> m_mapResources;
};

struct SIndexVertexBuffers
{
	VkAccelerationStructureBuildRangeInfoKHR  m_primitive;
	uint32_t m_primitiveCount{0};
	uint32_t m_maxVertex{0};
};

struct SVertexRT
{
	glm::vec3 pos;
	glm::vec3 normal;
};

class CResourceFactory
{
public:
	void init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice, vk_structs::CCommandPool* pCommandPool);
	vk_structs::CBuffer createBufferResource(VkDeviceSize bufferSize,
								  VkBufferUsageFlags,
								  const uint32_t* pQueueFamilyIndices,
								  VkMemoryPropertyFlags memoryPropertyFlags,
								  const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo) const;
	vk_structs::CTexture createTextureResourceRt(const VkFormat& format, uint32_t present, vk_base::CQueue& queue) const;
	vk_structs::CTexture createTextureResource(const std::vector<unsigned char>& pixels,
								   int texWidth, int texHeight,
								   int texChannels,
								   vk_base::CQueue& queue);
	vk_structs::CBlas createBottomLevel(VkDevice device, const vk_structs::CBuffer& vertexBuffer,
										const vk_structs::CBuffer& indexBuffer, const SIndexVertexBuffers& geom, const VkMemoryAllocateFlagsInfo& memoryAllocateFlagsInfo, uint32_t present, const vk_structs::SSupportDetails& details, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue);
	vk_structs::CTlas createTopLevel(VkDevice device, const std::vector<VkDeviceAddress>& aBottomLevelAccelerationStructureDeviceAddress,
									 const VkMemoryAllocateFlagsInfo& memoryAllocateFlagsInfo, vk_base::CQueue& queue, uint32_t present, const vk_structs::SSupportDetails& details, vk_structs::CCommandPool& commandPool);
private:
	VkDevice m_device{VK_NULL_HANDLE};
	vk_structs::SSupportDetails m_details;
	VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
	vk_structs::CCommandPool* m_pCommandPool;

};
class CResourceManager
{
public:
	uint32_t createBlasResource(VkDescriptorType descriptorType, int destinationBinding);

	void destroy();
	vk_structs::CUniformBase* uniform(uint32_t i) const;

private:
	CResourceStorage m_storage;
};

class CDescriptorLayoutCache
{
public:
	void setPipelineDescriptorLayouts(const std::map<en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo>& mapSchema);
	VkDescriptorType getDescriptorType(en_vk_utils::EPipelineLayoutType, uint32_t descriptorNum, uint32_t binding) const;
	VkDescriptorSetLayout getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType, uint32_t num) const;
private:
	std::map<en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo> m_mapPipelineLayoutSchema;
};

class CDescriptorManager
{
public:
	void init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice);
	void addDescriptor(uint64_t idElement, 
					   uint32_t frame, 
					   uint32_t index,
					   VkDescriptorSetLayout descriptorSetLayout, 
					   std::map<uint32_t, SUniform>&& binding,
					   const std::vector < VkWriteDescriptorSet>& aWriteDescriptorSet);
	void addDescriptor(uint64_t idElement,
					   uint32_t frame,
					   uint32_t index,
					   VkDescriptorSetLayout descriptorSetLayout,
					   std::map<uint32_t, SUniform>&& binding);
	
	SEntityDescriptorSetsForBinding getDescriptors(uint64_t idElement) const;
	SFrameDescriptorBindings* getDescriptorsBinding(uint64_t idElement, uint32_t frame) const;
	void destroy();
	void clearDescriptors();
	CDescriptorLayoutCache& descriptorLayout(){return m_pipelineLayoutSchema;}

private:
	CDescriptorLayoutCache m_pipelineLayoutSchema;
	std::unordered_map<SFrameKey, SFrameSetDescriptorBindings, SFrameKeyHash> m_mapFrameDescriptor;
	VkDevice m_device{VK_NULL_HANDLE};
	vk_structs::SSupportDetails m_details;
	VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};

};
}

struct SDescriptorInit
{
	uint64_t m_id;
	en_vk_utils::EPipelineLayoutType m_layout;
	size_t m_uniformBufferSize{0};
	size_t m_uniformBufferSize2{0};
	uint32_t m_meshId{0};
	uint32_t m_textureId{0};
};

struct SRay
{
	glm::vec4 origin{0};
	glm::vec4 direction{0};
	glm::uvec4 totalTriangles{0};
	glm::mat4 view;
	glm::mat4 proj;
};

struct SHit {
	uint32_t tBits{0};
	uint32_t meshIndex{0};
	uint32_t triangleIndex{0};
	uint32_t idObject{0};
	glm::vec4 pt1;
	glm::vec4 pt2;
	glm::vec4 pt3;
};

struct SMeshDesc
{
	uint32_t vertexBufferIndex;
	uint32_t indexBufferIndex;
	uint32_t indexCount;
	uint32_t triangleOffset;
	glm::uvec4 objectId;
	glm::mat4 model;
	glm::mat4 modelInv;
};

struct SComputeInit
{
	std::vector<VkDescriptorBufferInfo> aBufferInfoVx;
	std::vector<VkDescriptorBufferInfo> aBufferInfoIndex;
	std::vector<SMeshDesc> aMeshInfo;
	uint64_t m_id{0};
};
