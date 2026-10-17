#pragma once
#include <vector>
#include <string>
#include "co_geom_primitives.h"
#include "vk_objects.h"

namespace en_vk_utils
{

enum class EPipelineType
{
    e3d = 0,
    e3dAnimation,
    e2d,
    e2dInterface,
    e2dLines,
    eRt,
    eRt2,
    eCompute,
    eCount
};

enum class EPipelineLayoutType
{
    e3d = 0,
    e3dAnimation,
    e2d,
    e2dInterface,
    eRt,
    eRt2,
    eCompute,
    eCount
};

struct SDescriptorLayoutIndex
{
    EPipelineLayoutType m_type;
    uint32_t m_index{0};
    bool operator==(const SDescriptorLayoutIndex& other) const noexcept
    {
        return m_type == other.m_type && m_index == other.m_index;
    }

    bool operator<(const SDescriptorLayoutIndex& other) const noexcept
    {
        if(m_type != other.m_type)
            return m_type < other.m_type;
        return m_index < other.m_index;
    }
};

struct SMaterial
{
    glm::vec4 ambient{0, 0, 0, 0};
    glm::vec4 diffuse{0, 0, 0, 0};
    glm::vec4 specular{0, 0, 0, 0};
    glm::vec4 emission{0, 0, 0, 0};
};

struct SLoadedObject
{
    std::vector<commons_struct::SVertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<SMaterial> m_materialList;
    std::vector<uint32_t> m_materialIndexList;
};

std::vector<unsigned char> loadTexture(int& texWidth, int& texHeight, int& texChannels, char const* filename);
std::vector<unsigned char> loadTextureFromMemory(int& texWidth, int& texHeight, int& texChannels, const unsigned char* pData, int size);
std::vector<char> readFile(const std::string& filename);
std::string readFile2(const std::string& filename);

vk_structs::CBuffer createBufferCpu(VkDeviceSize bufferSize, void* dataIn, const VkDevice& logicalDevice, const vk_structs::SSupportDetails& details, VkBufferUsageFlags flag);
vk_structs::CBuffer createMultiBufferCpu(VkDeviceSize bufferSize1, const void* dataIn1, VkDeviceSize bufferSize2, const void* dataIn2, const VkDevice& logicalDevice, const vk_structs::SSupportDetails& details, VkBufferUsageFlags flag);
void createBufferGpu(VkDeviceSize bufferSize, void* dataIn, vk_structs::CBuffer& buffer, VkBufferUsageFlags usage, const VkDevice& logicalDevice, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue, const vk_structs::SSupportDetails& details);
void createMultiBufferGpu(VkDeviceSize bufferSize1, const void* dataIn1, VkDeviceSize bufferSize2, const void* dataIn2, vk_structs::CBuffer& buffer, VkBufferUsageFlags usage, const VkDevice& logicalDevice, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue, const vk_structs::SSupportDetails& details, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo);
}

