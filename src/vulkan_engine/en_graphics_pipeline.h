#pragma once
#include <array>
#include <unordered_map>
#include "vk_objects.h"
#include "en_utils.h"

namespace en_vk
{


class CPipelineManager
{
public:
	CPipelineManager() = default;
	~CPipelineManager() = default;
    VkPipeline pipeline(en_vk_utils::EPipelineType pipe) const;
    void createRayTracingPipeline(VkDevice device, VkPipelineLayout layout);
    
    void createPipeline3D(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout);
    void createPipeline2D(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout);
    void createPipeline2DInterface(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout);
    void createPipeline3DAnimated(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout);
    void createPipelinesCompute(VkDevice device, VkPipelineLayout layout);

    void clear();
private:
    vk_structs::SPipeline& getPipeline(en_vk_utils::EPipelineType type);

private:
    std::array<vk_structs::SPipeline, static_cast<size_t>(en_vk_utils::EPipelineType::eCount)> m_aPipelines;
};

class CPipelineLayoutManager
{
public:
    CPipelineLayoutManager() = default;
    ~CPipelineLayoutManager() = default;
    const vk_structs::CPipelineLayout& getLayout(en_vk_utils::EPipelineLayoutType type);
    void clear();
    void createGraphicsPipelineLayouts(VkDevice device);
    void createRayTracingLayout(VkDevice device);
    
    const std::map< en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo>& getDescriptorSetLayoutsSchema() const;
private:
    void getDescriptorSetLayouts();
private:
    std::array<vk_structs::CPipelineLayout, static_cast<size_t>(en_vk_utils::EPipelineLayoutType::eCount)> m_aPipelineLayout;
    std::map< en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo> m_aPipelineLayoutSchema;
};

}