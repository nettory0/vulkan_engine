#include "en_render_resource.h" 
#include "en_animation_structs.h"

namespace en_vk
{

void CRenderResources::init(VkDevice device, const vk_structs::SSupportDetails& details, VkPhysicalDevice physicalDevice, vk_structs::CCommandPool* pCommandPool)
{
	m_factory.init(device, details, physicalDevice, pCommandPool);
	m_descriptorManager.init(device, details, physicalDevice);
}

void CRenderResources::updateBuffer(uint32_t bufferId, size_t bufferSize, const void* dataIn, size_t offset)
{
	if(auto* pBuffer = m_bufferManager.get(bufferId))
		pBuffer->load(bufferSize, dataIn, offset);
}

void CRenderResources::readBuffer(uint32_t bufferId, size_t bufferSize, void* dataIn)
{
	if(auto* pBuffer = m_bufferManager.get(bufferId))
		pBuffer->read(bufferSize, dataIn);
}

uint32_t CRenderResources::addBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlagBits, const uint32_t* pQueueFamilyIndices, VkMemoryPropertyFlags memoryPropertyFlags, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo)
{
	return m_bufferManager.createBuffer(std::make_unique<vk_structs::CBuffer>(m_factory.createBufferResource(bufferSize, bufferUsageFlagBits, pQueueFamilyIndices, memoryPropertyFlags, memoryAllocateFlagsInfo)));
}

CBufferManager::~CBufferManager()
{
	clear();
}

uint32_t CBufferManager::createBuffer(std::unique_ptr<vk_structs::CBuffer> pResource)
{
	return m_aBuffer.add(std::move(pResource));
}

uint32_t CBufferManager::createBuffer()
{
	return m_aBuffer.add(std::make_unique<vk_structs::CBuffer>());
}

vk_structs::CBuffer* CBufferManager::get(uint32_t id)
{
	return  m_aBuffer.get(id);
}

void CBufferManager::destroy(uint32_t id)
{
	if(auto* pBuffer = m_aBuffer.get(id))
	{
		pBuffer->clean();
		m_aBuffer.remove(id);
	}
}

void CBufferManager::clear()
{
	for(auto& [id, buffer] : m_aBuffer.resource())
		buffer->clean();
	m_aBuffer.clear();
}

const SDraw* CRenderResources::getMesh(uint32_t meshId)
{
	if(auto it = m_mapRender.find(meshId); it != m_mapRender.end())
		return &it->second;
	return nullptr;
}

const vk_structs::CTexture* CRenderResources::getTextureUniform(uint32_t textureId) const
{
	if(const auto it = m_aTexture.find(textureId); it != m_aTexture.end())
		return &it->second;
	return nullptr;
}

void CRenderResources::clear()
{
	for(auto& [id, mesh] : m_mapRender)
		m_bufferManager.destroy(mesh.m_vertexIndexBuffer);
	m_mapRender.clear();
	m_bufferManager.clear();
	m_aTexture.clear();
}

vk_structs::CBuffer* CRenderResources::getBuffer(uint32_t bufferId)
{
	return m_bufferManager.get(bufferId);
}

vk_structs::CTexture* CRenderResources::getTexture(uint32_t textureId)
{
	if(auto it = m_aTexture.find(textureId); it != m_aTexture.end())
		return &it->second;
	return nullptr;

}
vk_structs::CUniformBase* CRenderResources::getUniform(uint64_t idElement, uint32_t frame, uint32_t numLayout, int dstBinding)
{
	if(auto* pBindings = m_descriptorManager.getDescriptorsBinding(idElement, frame))
	{
		if(auto itL = pBindings->m_resourceIndex.find(SDescriptorResourceIndex(numLayout,dstBinding)); itL != pBindings->m_resourceIndex.end())
		{
			return itL->second.m_uniform.get();
		}
	}
	return nullptr;
}

uint32_t CRenderResources::getUniformBuffer(uint64_t idElement, int frame, uint32_t numLayout, int dstBinding)
{
	if(auto* pUniform = getUniform(idElement, frame, numLayout, dstBinding))
	{
		auto* pBuffer = static_cast<vk_structs::CUniformBuffer*>(pUniform);
		return pBuffer->m_bufferId;
	}
	return 0;
}

}