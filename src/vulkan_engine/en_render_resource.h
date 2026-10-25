#pragma once
#include "vk_base.h"
#include "en_graphics_pipeline.h"
#include "en_context.h"
#include "en_descriptor_manager.h"
#include "en_draw_object.h"
#include "en_common_utils.h"

namespace en_model
{
struct SVertexData;
}

class CGpuUploadSystem;

namespace en_vk
{

struct SDraw
{
	uint32_t m_vertexIndexBuffer{0};
	size_t m_sizeVertices{0};
	size_t m_sizeIndexes{0};
	size_t m_structSize{0};
};

class CBufferManager
{
public:
	~CBufferManager();
	uint32_t createBuffer(std::unique_ptr<vk_structs::CBuffer> pResource);
	uint32_t createBuffer();
	vk_structs::CBuffer* get(uint32_t id);
	void destroy(uint32_t id);
	void clear();
private:
	en_util::CResourceKeeper<vk_structs::CBuffer> m_aBuffer;
};


class CRenderResources
{
	friend class CGpuUploadSystem;
public:
	const SDraw* getMesh(uint32_t meshId);
	const vk_structs::CTexture* getTextureUniform(uint32_t textureId) const;
	void clear();
	vk_structs::CBuffer* getBuffer(uint32_t bufferId);
	vk_structs::CTexture* getTexture(uint32_t textureId);
	void init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice, vk_structs::CCommandPool* pCommandPool);
	void updateBuffer(uint32_t bufferId, size_t bufferSize, const void* dataIn, size_t offset);
	void readBuffer(uint32_t bufferId, size_t bufferSize, void* dataIn);
	uint32_t addBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlagBits, const uint32_t* pQueueFamilyIndices, VkMemoryPropertyFlags memoryPropertyFlags, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo);
	uint32_t getUniformBuffer(uint64_t idElement, int frame, uint32_t numLayout, int dstBinding);
	vk_structs::CUniformBase* getUniform(uint64_t idElement, uint32_t frame, uint32_t numLayout, int dstBinding);

public:
	CResourceManager m_uniformManager;
	CBufferManager m_bufferManager;
	en_vk::CDescriptorManager m_descriptorManager;
	CResourceFactory m_factory;

private:
	std::unordered_map<uint32_t, vk_structs::CTexture> m_aTexture;
	std::map<uint32_t, SDraw> m_mapRender;


};
}