#include "en_gl_utils.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <format>
#include "co_logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace en_vk_utils
{

std::vector<unsigned char> loadTexture(int& texWidth, int& texHeight, int& texChannels, char const* filename)
{
    stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels)
    {
        common::logError(std::format(" loadTexture {}\n", filename));
        throw std::runtime_error("failed to load texture image!");
    }
	std::vector<unsigned char> result{pixels, pixels + texWidth * texHeight * 4};
	stbi_image_free(pixels);
	return result;
}

std::vector<unsigned char> loadTextureFromMemory(int& texWidth, int& texHeight, int& texChannels, const unsigned char* pData, int size)
{
    stbi_set_flip_vertically_on_load(true);

    stbi_uc* pixels = stbi_load_from_memory(pData, size, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels)
    {
        common::logError("failed to load texture image!");
        throw std::runtime_error("failed to load texture image!");
    }
    std::vector<unsigned char> result{pixels, pixels + texWidth * texHeight * 4};
    stbi_image_free(pixels);
    return result;
}

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    std::cout << filename << std::endl;
    if(!file.is_open())
    {
        common::logError(std::format("failed to open file {}", filename));
        throw std::runtime_error("failed to open file!");
    }
        

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

std::string readFile2(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    std::cout << filename << std::endl;
    if(!file.is_open())
    {
        common::logError(std::format("failed to open file {}", filename));
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::string buffer;// (fileSize);
    buffer.resize(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

vk_structs::CBuffer createBufferCpu(VkDeviceSize bufferSize, void* dataIn, const VkDevice& logicalDevice, const vk_structs::SSupportDetails& details, VkBufferUsageFlags flag)
{
    vk_structs::CBuffer staging;
    VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, bufferSize, flag, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr)};//
    staging.create(logicalDevice, details, bufferInfo, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging.load((size_t)bufferSize, dataIn, 0);
    return staging;
}

vk_structs::CBuffer createMultiBufferCpu(VkDeviceSize bufferSize1, const void* dataIn1, VkDeviceSize bufferSize2, const void* dataIn2, const VkDevice& logicalDevice, const vk_structs::SSupportDetails& details, VkBufferUsageFlags flag)
{
    vk_structs::CBuffer staging;
    VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, bufferSize1 + bufferSize2, flag, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr)};//
    staging.create(logicalDevice, details, bufferInfo, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging.load((size_t)bufferSize1, dataIn1, 0);
    staging.load((size_t)bufferSize2, dataIn2, (size_t)bufferSize1);
    return staging;
}

void createBufferGpu(VkDeviceSize bufferSize, void* dataIn, vk_structs::CBuffer& buffer, VkBufferUsageFlags usage, const VkDevice& logicalDevice, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue, const vk_structs::SSupportDetails& details)
{
    vk_structs::CBuffer staging = createBufferCpu(bufferSize, dataIn, logicalDevice, details, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, bufferSize, usage, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr)};
    buffer.create(logicalDevice, details, bufferInfo, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto commandBuffer = commandPool.beginSingleTimeCommands();
    commandBuffer.copyBuffer(staging.handle(), buffer.handle(), 0, 0, bufferSize);
    commandPool.endSingleTimeCommands(commandBuffer, queue);
    staging.clean();
}

void createMultiBufferGpu(VkDeviceSize bufferSize1, const void* dataIn1, VkDeviceSize bufferSize2, const void* dataIn2, vk_structs::CBuffer& buffer, VkBufferUsageFlags usage, const VkDevice& logicalDevice, vk_structs::CCommandPool& commandPool, vk_base::CQueue& queue, const vk_structs::SSupportDetails& details, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo)
{
    vk_structs::CBuffer staging = createMultiBufferCpu(bufferSize1, dataIn1, bufferSize2, dataIn2, logicalDevice, details, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, bufferSize1+ bufferSize2, usage, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr)};
    buffer.create(logicalDevice, details, bufferInfo, memoryAllocateFlagsInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto commandBuffer = commandPool.beginSingleTimeCommands();
    commandBuffer.copyBuffer(staging.handle(), buffer.handle(), 0, 0, bufferSize1 + bufferSize2);
    commandPool.endSingleTimeCommands(commandBuffer, queue);
    staging.clean();
}

}