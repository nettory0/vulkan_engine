#include "en_compute_system.h"
#include "en_descriptor_manager.h"

namespace en_vk
{
void CComputeSystem::dispatch(uint32_t& currentFrameInd, 
							  const SEntityDescriptorSetsForBinding& aDescriptors, 
							  VkPipeline pipeline, 
							  VkPipelineLayout pipelineLayout, 
							  uint32_t totalTriangles, 
							  vk_base::CQueue& queue, 
							  vk_structs::CCommandPool& commandPool,
							  vk_structs::CSemaphoreFence& semaphore)
{
	auto& imageComputeFences = semaphore;
	imageComputeFences.waitForFence();
	imageComputeFences.resetFence();

	VkSemaphore signalSemaphores[] = {imageComputeFences.renderfinishedSemaphore()};
	VkSemaphore waitSemaphores[] = {imageComputeFences.availableSemaphore()};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};
	auto& pt = aDescriptors.m_mapDescriptor.at(currentFrameInd);

	VkCommandBufferBeginInfo beginInfo{vk_init::commandBufferBeginInfo(nullptr, 0, nullptr)};

	std::vector<uint32_t> bufferIndices;
	std::vector<VkDeviceSize> offsetsDesc;
	uint32_t i = 0;
	for(auto& s : pt)
	{
		bufferIndices.push_back(i);
		offsetsDesc.push_back(0);
		i++;
	}
	auto command = commandPool.beginSingleTimeCommands();
	command.bindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	command.bindDescriptorBuffersEXT(pt.size(), pt.data());
	command.setDescriptorBufferOffsetsEXT(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, pt.size(), bufferIndices.data(), offsetsDesc.data());

	uint32_t triangleCount = totalTriangles;

	uint32_t groupSize = 64;
	uint32_t dispatchX = (triangleCount + groupSize - 1) / groupSize;

	command.dispatch(dispatchX, 1, 1);

	//command.dispatch(1, 1, 1);
	command.endCommandBuffer();

	VkSubmitInfo submitInfo{vk_init::submitInfo(nullptr, 0, 0, waitStages, 1, &command.handle(), 0, 0)};
	queue.submit(submitInfo, imageComputeFences.fence());
	queue.waitIdle();
	commandPool.freeSingleTimeCommand(command);
}
}