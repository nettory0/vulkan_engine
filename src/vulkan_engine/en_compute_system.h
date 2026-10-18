#pragma once
#include <vector>
#include "vk_objects.h"


namespace en_vk
{
struct SEntityDescriptorSetsForBinding;
class CComputeSystem
{
public:
	CComputeSystem() = default;
	~CComputeSystem() = default;
	void dispatch(uint32_t& currentFrameInd, 
				  const SEntityDescriptorSetsForBinding& aDescriptors, 
				  VkPipeline pipeline, 
				  VkPipelineLayout pipelineLayout, 
				  uint32_t totalTriangles, 
				  vk_base::CQueue& queue,
				  vk_structs::CCommandPool& commandPool,
				  vk_structs::CSemaphoreFence& semaphore);
public:

};
}