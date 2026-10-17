#include "en_render_system.h"
#include <stdexcept>
#include "en_gl_utils.h"
#include "en_gl_draw_object.h"
#include "en_animation_structs.h"
#include "en_gl_descriptor_manager.h"

constexpr auto bufferCreateUsage{VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
constexpr auto bufferCreateUsage2{VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR};
constexpr auto bufferCreateUsage3{VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};


void transitionColorAttachment(vk_base::CCommandBuffer& command, VkImage image)
{
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &barrier;

	command.pipelineBarrier2(&depInfo);
}

void transitionDepthAttachment(vk_base::CCommandBuffer& command, VkImage image)
{
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
	barrier.srcAccessMask = VK_ACCESS_2_NONE;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &barrier;

	command.pipelineBarrier2(&depInfo);
}

void transitionToPresent(vk_base::CCommandBuffer& command, VkImage image)
{
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &barrier;

	command.pipelineBarrier2(&depInfo);
}

void transitionSwapchainToAttachment(vk_base::CCommandBuffer& command, VkImage image)
{
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.srcAccessMask = 0;
	barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	VkDependencyInfo dependencyInfo{};
	dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &barrier;
	command.pipelineBarrier2(&dependencyInfo);
}


void CRenderSystem::beginRendering(vk_base::CCommandBuffer& command, VkRect2D renderArea, vk_structs::SRenderTargets& framebuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	command.beginCommandBuffer(beginInfo);
	transitionSwapchainToAttachment(command, framebuffer.m_framebuffersImageViews[imageIndex].m_swapChainImage);
	transitionColorAttachment(command, framebuffer.m_color.handle());
	transitionDepthAttachment(command, framebuffer.m_depth.handle());

	VkClearValue colorClear{};
	colorClear.color = {{0.5f, 0.8f, 1.0f, 1.0f}};

	VkClearValue depthClear{};
	depthClear.depthStencil = {1.0f, 0};

	// MSAA color target
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = framebuffer.m_color.view();
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;

	// resolve -> swapchain
	colorAttachment.resolveImageView = framebuffer.m_framebuffersImageViews[imageIndex].m_imageView.handle();
	colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue = colorClear;

	// depth
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = framebuffer.m_depth.view();
	depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.clearValue = depthClear;

	// rendering info
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = renderArea;
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;


	command.beginRendering(&renderingInfo);
}

void CRenderSystem::endRendering(vk_base::CCommandBuffer& command, vk_structs::SRenderTargets& framebuffer, uint32_t imageIndex)
{
	command.endRendering();
	transitionToPresent(command, framebuffer.m_framebuffersImageViews[imageIndex].m_swapChainImage);
	command.endCommandBuffer();
}

void CPipelineSystem::createPipeRayTracing(VkDevice device)
{
	m_graphicPipelineLayoutManager.createRayTracingLayout(device);
	m_graphicPipelineManager.createRayTracingPipeline(device, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::eRt).layout());
}

void CPipelineSystem::createPipe3d(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details)
{
	m_graphicPipelineLayoutManager.createGraphicsPipelineLayouts(device);
	m_graphicPipelineManager.createPipeline3D(device, msaaSamples, renderPass, details, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3d).layout());
	m_graphicPipelineManager.createPipeline2D(device, msaaSamples, renderPass, details, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e2d).layout());
	m_graphicPipelineManager.createPipeline2DInterface(device, msaaSamples, renderPass, details, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e2dInterface).layout());
	m_graphicPipelineManager.createPipelinesCompute(device, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::eCompute).layout());
	m_graphicPipelineManager.createPipeline3DAnimated(device, msaaSamples, renderPass, details, m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3dAnimation).layout());
}

void CPipelineSystem::clear()
{
	m_graphicPipelineManager.clear();
	m_graphicPipelineLayoutManager.clear();
}

void CRenderSystem::clear()
{
	m_renderResources.clear();
	m_pipelineManager.clear();
}

CRenderSystem::CRenderSystem()
{
	m_uploadSystem.initialize(&m_renderResources, &m_context, &m_commandPool);
}

CRenderSystem::~CRenderSystem()
{
	clear();
}

void CRenderSystem::renderFrame(const SFrameRenderData& frameInfo, bool bUpdateBufferFrame)
{
	if(m_bNeedUpdate)
	{
		recreateSwapchain(); 
		return;
	}

	auto& imageRenderFences = m_frameSystem.getSemaphoreFence();

	imageRenderFences.m_imageRenderFences[m_currentFrameInd].waitForFence();
	uint32_t imageIndex;

	VkResult result = m_frameSystem.acquireNextImage(imageRenderFences.m_imageRenderFences[m_currentFrameInd].availableSemaphore(), VK_NULL_HANDLE, &imageIndex);

	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain(); 
		return;
	}
	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image!");

	updateUbo(frameInfo, m_currentFrameInd);

	VkSemaphore signalSemaphores[] = {imageRenderFences.m_renderFinishedSemaphores[imageIndex].handle()};
	VkSemaphore waitSemaphores[] = {imageRenderFences.m_imageRenderFences[m_currentFrameInd].availableSemaphore()};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSwapchainKHR swapChains[] = {m_frameSystem.m_swapChain.m_swapchain.handle()};

	imageRenderFences.m_imageRenderFences[m_currentFrameInd].resetFence();
	auto& command = m_commandPool.m_commandBuffers[m_currentFrameInd];
	if(bUpdateBufferFrame)
	{

		beginRendering(command, {{0, 0}, m_context.details().m_swapChainConfig.m_extent}, m_frameSystem.m_frameBuffer, imageIndex);
		drawSkinnedMeshes(command, m_currentFrameInd, frameInfo);
		drawStaticMeshes(command, m_currentFrameInd, frameInfo.m_highlighted, frameInfo.m_aSelected, frameInfo);
		drawLayer2d(command, m_currentFrameInd, frameInfo);
		drawLayer2dMenu(command, m_currentFrameInd, frameInfo);
		//layer2dMenu(imageIndex, command);
		endRendering(command, m_frameSystem.m_frameBuffer, imageIndex);

	}



	const VkSubmitInfo submitInfo{vk_init::submitInfo(nullptr, 1, waitSemaphores, waitStages, 1, &command.handle(), 1, signalSemaphores)};
	m_context.queue().m_graphic.submit(submitInfo, imageRenderFences.m_imageRenderFences[m_currentFrameInd].fence());
	const VkPresentInfoKHR presentInfo{vk_init::presentInfoKHR(nullptr, 1, signalSemaphores, 1, swapChains, &imageIndex, nullptr)};

	result = m_context.queue().m_present.present(presentInfo);

	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_bFramebufferResized)
	{
		m_bFramebufferResized = false;
		recreateSwapchain();
	}
	else if(result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image!");


	m_currentFrameInd = (m_currentFrameInd + 1) % imageRenderFences.m_framesInFlight;
}

uint32_t CRenderSystem::rayPick(const SFrameRenderData& frameInfo)
{
	if(auto id = frameInfo.m_computeInfo.m_computeEntity; id != 0)
	{
		float x = (2.0f * frameInfo.m_computeInfo.m_mouseCoord.x) / frameInfo.m_computeInfo.m_screenSize.x - 1.0f;
		float y = 1.0f - (2.0f * frameInfo.m_computeInfo.m_mouseCoord.y) / frameInfo.m_computeInfo.m_screenSize.y;
		glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

		auto matProj = frameInfo.m_camera.proj;
		matProj[1][1] *= -1;
		glm::mat4 invVP = glm::inverse(matProj * frameInfo.m_camera.view);
		glm::vec4 world = invVP * rayClip;
		world /= world.w;

		SRay ray;
		ray.origin = glm::vec4(frameInfo.m_computeInfo.m_cameraPos, 1);
		ray.direction = glm::vec4(glm::normalize(glm::vec3(world) - glm::vec3(ray.origin)), 0);
		ray.totalTriangles.x = frameInfo.m_computeInfo.m_totalTriangles;
		ray.view = frameInfo.m_camera.view;
		ray.proj = matProj;
		if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(id, m_currentFrameInd, 0, 3)))
			pBuffer->load(sizeof(SRay), &ray, 0);

		SHit hitReset;
		hitReset.tBits = std::numeric_limits<uint32_t>::max();
		hitReset.meshIndex = UINT32_MAX;
		hitReset.triangleIndex = UINT32_MAX;

		if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(id, m_currentFrameInd, 0, 4)))
			pBuffer->load(sizeof(SHit), &hitReset, 0);

		auto pDescriptorObj = m_renderResources.m_descriptorManager.getDescriptors(id);
		m_computeSystem.dispatch(m_currentFrameInd,
								 pDescriptorObj,
								 m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::eCompute),
								 m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::eCompute).layout(),
								 frameInfo.m_computeInfo.m_totalTriangles,
								 m_context.queue().m_compute,
								 m_commandPool,
								 m_frameSystem.m_computeFrame.m_imageRenderFences[m_currentFrameInd]);
		SHit hitResult;

		if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(id, m_currentFrameInd, 0, 4)))
			pBuffer->read(sizeof(SHit), &hitResult);

		return hitResult.meshIndex != UINT32_MAX ? hitResult.idObject : UINT32_MAX;
	}
	return UINT32_MAX;
}

void CRenderSystem::initializeRaster(VkInstance instance, VkSurfaceKHR surface)
{
	m_bRayTracing = false;
	m_context.create(instance, surface, false);
	createSecond(false);
	m_pipelineManager.createPipe3d(m_context.logicalDevice().handle(), m_context.m_msaaSamples, nullptr, m_context.details());
	m_renderResources.m_descriptorManager.descriptorLayout().setPipelineDescriptorLayouts(m_pipelineManager.m_graphicPipelineLayoutManager.getDescriptorSetLayoutsSchema());

	m_renderResources.init(m_context.logicalDevice().handle(), m_context.details(), m_context.physicalDevice().handle(), &m_commandPool);
}

void CRenderSystem::initializeRayTracing(VkInstance instance, VkSurfaceKHR surface, const std::vector<uint32_t>& indexVertex, const std::vector<en_vk::SVertexRT>& points)
{
	m_bRayTracing = true;
	m_context.create(instance, surface, true);
	createSecond(true);
	m_pipelineManager.createPipeRayTracing(m_context.logicalDevice().handle());
	m_renderResources.m_descriptorManager.descriptorLayout().setPipelineDescriptorLayouts(m_pipelineManager.m_graphicPipelineLayoutManager.getDescriptorSetLayoutsSchema());
	m_renderResources.init(m_context.logicalDevice().handle(), m_context.details(), m_context.physicalDevice().handle(), &m_commandPool);
	rayTracingDescriptors(indexVertex, points);
}

void CRenderSystem::recordMeshDrawCommands(vk_base::CCommandBuffer& command, const VkPipelineLayout& pipeline, const std::vector<VkDescriptorBufferBindingInfoEXT>& aDescriptors, const uint32_t meshId)
{
	if(auto* buffer = m_renderResources.getMesh(meshId))
	{
		if(auto* gpuBuffer = m_renderResources.getBuffer(buffer->m_vertexIndexBuffer))
		{
			VkDeviceSize offsets[] = {0};
			command.bindVertexBuffer(0, 1, &gpuBuffer->handle(), offsets);
			command.bindIndexBuffer(gpuBuffer->handle(), buffer->m_structSize * buffer->m_sizeVertices, VK_INDEX_TYPE_UINT32);

			if(!aDescriptors.empty())
			{
				std::vector<uint32_t> bufferIndices;
				std::vector<VkDeviceSize> offsetsDesc;
				uint32_t i = 0;
				for(auto& s : aDescriptors)
				{
					bufferIndices.push_back(i);
					offsetsDesc.push_back(0);
					i++;
				}

				command.bindDescriptorBuffersEXT(aDescriptors.size(), aDescriptors.data());
				command.setDescriptorBufferOffsetsEXT(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, 0, aDescriptors.size(), bufferIndices.data(), offsetsDesc.data());
			}
			command.drawIndexed(static_cast<uint32_t>(buffer->m_sizeIndexes), 1, 0, 0, 0);
		}
	}
}

void CRenderSystem::createSecond(bool bRt)
{
	m_frameSystem.create(m_context.logicalDevice().handle(), m_context.getSurface(), m_context.queueFamilyIndices(), m_context.details(), m_context.m_msaaSamples, bRt);
	m_commandPool.create(m_context.logicalDevice().handle(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, m_context.queueFamilyIndices().m_presentFamily.value(), m_frameSystem.m_renderFrame2.m_framesInFlight);
}

void CRenderSystem::recreateSwapchain()
{
	if(!m_context.getInstance())
		return;
	m_context.logicalDevice().waitIdle();
	m_frameSystem.m_swapChain.destroy();
	m_frameSystem.m_frameBuffer.destroy();
	m_frameSystem.m_renderFrame.destroy();
	m_frameSystem.m_renderFrame2.destroy();
	m_frameSystem.m_computeFrame.destroy();
	m_pipelineManager.m_graphicPipelineManager.clear();


	m_context.initDetails();
	createSecond(m_bRayTracing);
	if(m_bRayTracing)
		m_pipelineManager.createPipeRayTracing(m_context.logicalDevice().handle());
	else
		m_pipelineManager.createPipe3d(m_context.logicalDevice().handle(), m_context.m_msaaSamples, nullptr, m_context.details());
	m_bNeedUpdateFrame = true;
}

void CRenderSystem::renderRayTracingFrame(bool bUpdateBufferFrame, size_t sizeUbo, const void* pDataUbo)
{

	vk_structs::CSemaphoreFence& imageRenderFences = m_frameSystem.getFrame(m_currentFrameInd);

	imageRenderFences.waitForFence();
	uint32_t imageIndex;

	VkResult result = m_frameSystem.acquireNextImage(imageRenderFences.availableSemaphore(), VK_NULL_HANDLE, &imageIndex);

	if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(common::cGlobalRenderEntity, 0, 0, 1)))
		pBuffer->load(sizeUbo, pDataUbo, 0);


	if(m_bNeedUpdate || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapchain();
		return;
	}
	else if(result != VK_SUCCESS)
		throw std::runtime_error("failed to acquire swap chain image!");


	VkSemaphore signalSemaphores[] = {imageRenderFences.renderfinishedSemaphore()};
	VkSemaphore waitSemaphores[] = {imageRenderFences.availableSemaphore()};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSwapchainKHR swapChains[] = {m_frameSystem.m_swapChain.m_swapchain.handle()};

	imageRenderFences.resetFence();
	auto aSet = m_renderResources.m_descriptorManager.getDescriptors(common::cGlobalRenderEntity);



	auto* pUniformTextureRt = static_cast<vk_structs::CUniformTexture*>(m_renderResources.getUniform(common::cGlobalRenderEntity, 0, 0, 4));
	auto* pTextureRt = m_renderResources.getTexture(pUniformTextureRt->m_textureId);
	const VkStridedDeviceAddressRegionKHR callableShaderBindingTable = {};

	VkCommandBufferBeginInfo renderCommandBufferBeginInfo{vk_init::commandBufferBeginInfo(nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, nullptr)};
	const auto& t = m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::eRt).layout();

	VkImageMemoryBarrier swapchainCopyMemoryBarrier = vk_init::imageMemoryBarrier(
		NULL,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_frameSystem.m_frameBuffer.m_framebuffersImageViews[m_currentFrameInd].m_swapChainImage,
		{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});

	VkImageMemoryBarrier rayTraceCopyMemoryBarrier = vk_init::imageMemoryBarrier(
		NULL,
		0,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_context.queueFamilyIndices().m_presentFamily.value(),
		pTextureRt->m_texture.handle(),
		{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});

	VkImageCopy imageCopy = {
		.srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1},
		.srcOffset = {.x = 0, .y = 0, .z = 0},
		.dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1},
		.dstOffset = {.x = 0, .y = 0, .z = 0},
		.extent = {.width = m_context.details().m_swapChainInfo.m_capabilities.currentExtent.width,
		.height = m_context.details().m_swapChainInfo.m_capabilities.currentExtent.height,
		.depth = 1}};

	VkImageMemoryBarrier swapchainPresentMemoryBarrier = vk_init::imageMemoryBarrier(
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		0,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_frameSystem.m_frameBuffer.m_framebuffersImageViews[m_currentFrameInd].m_swapChainImage,
		{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});


	VkImageMemoryBarrier rayTraceWriteMemoryBarrier = vk_init::imageMemoryBarrier(
		NULL,
		VK_ACCESS_TRANSFER_READ_BIT,
		0,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		m_context.queueFamilyIndices().m_presentFamily.value(),
		m_context.queueFamilyIndices().m_presentFamily.value(),
		pTextureRt->m_texture.handle(),
		{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});


	auto& command = m_commandPool.m_commandBuffers[m_currentFrameInd];
	command.beginCommandBuffer(renderCommandBufferBeginInfo);
	command.bindPipeline(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::eRt));

	std::vector<uint32_t> bufferIndices;
	std::vector<VkDeviceSize> offsetsDesc;
	uint32_t i = 0;
	for(auto& s : aSet.m_mapDescriptor[0])
	{
		bufferIndices.push_back(i);
		offsetsDesc.push_back(0);
		i++;
	}

	command.bindDescriptorBuffersEXT(aSet.m_mapDescriptor[0].size(), aSet.m_mapDescriptor[0].data());
	command.setDescriptorBufferOffsetsEXT(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::eRt).layout(), 0, aSet.m_mapDescriptor.size(), bufferIndices.data(), offsetsDesc.data());

	//command.bindDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, t, 0, (uint32_t)aSet.m_mapDescriptor[0].size(), aSet.m_mapDescriptor[0].data(), 0, NULL);
	command.traceRaysKHR(&m_rayTracing.rgenShaderBindingTable, &m_rayTracing.rmissShaderBindingTable, &m_rayTracing.rchitShaderBindingTable, &callableShaderBindingTable, m_context.details().m_swapChainInfo.m_capabilities.currentExtent.width, m_context.details().m_swapChainInfo.m_capabilities.currentExtent.height, 1);
	command.pipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &swapchainCopyMemoryBarrier);
	command.pipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &rayTraceCopyMemoryBarrier);
	command.copyImage(pTextureRt->m_texture.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_frameSystem.m_frameBuffer.m_framebuffersImageViews[m_currentFrameInd].m_swapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
	command.pipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &swapchainPresentMemoryBarrier);
	command.pipelineBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &rayTraceWriteMemoryBarrier);
	command.endCommandBuffer();

	VkSubmitInfo submitInfo{vk_init::submitInfo(nullptr, 1, waitSemaphores, waitStages, 1, &command.handle(), 1, signalSemaphores)};
	m_context.queue().m_graphic.submit(submitInfo, imageRenderFences.fence());
	VkPresentInfoKHR presentInfo{vk_init::presentInfoKHR(nullptr, 1, signalSemaphores, 1, swapChains, &imageIndex, nullptr)};

	result = m_context.queue().m_present.present(presentInfo);

	m_currentFrameInd = (m_currentFrameInd + 1) % m_frameSystem.m_renderFrame2.m_framesInFlight;
}

void CRenderSystem::addTextureDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType, uint32_t textureId, common::entityId64 id, uint32_t index)
{
	for(int frameInd = 0; frameInd < m_frameSystem.m_renderFrame2.m_framesInFlight; frameInd++)
	{
		std::unique_ptr<vk_structs::CUniformTexture> texture = std::make_unique<vk_structs::CUniformTexture>(descriptorType, 0);
		texture->init(textureId, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, *m_renderResources.getTextureUniform(textureId));
		std::map<uint32_t, en_vk::SUniform> resourceIndex;
		resourceIndex[0].m_uniform = std::move(texture);
		m_renderResources.m_descriptorManager.addDescriptor(id, frameInd, index, descriptorSetLayout, std::move(resourceIndex));
	}
}

void CRenderSystem::addBufferDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType, uint32_t uniformBufferSize2, common::entityId64 id, uint32_t index)
{
	VkMemoryAllocateFlagsInfo flagsInfo{};
	flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	for(int frameInd = 0; frameInd < m_frameSystem.m_renderFrame2.m_framesInFlight; frameInd++)
	{
		auto bufferId2 = m_renderResources.addBuffer(uniformBufferSize2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &flagsInfo);

		std::unique_ptr <vk_structs::CUniformBuffer> uniformBuffer = std::make_unique<vk_structs::CUniformBuffer>(descriptorType, 0);
		uniformBuffer->init(bufferId2, *m_renderResources.getBuffer(bufferId2));

		std::map<uint32_t, en_vk::SUniform> resourceIndex2;
		resourceIndex2[0].m_uniform = std::move(uniformBuffer);

		m_renderResources.m_descriptorManager.addDescriptor(id, frameInd, index, descriptorSetLayout, std::move(resourceIndex2));
	}
}

void CRenderSystem::addEntityDescriptorSetStatic(common::entityId64 id, en_vk_utils::EPipelineLayoutType layout, uint32_t textureId)
{
	const auto layout1 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(layout, 1);
	addTextureDescriptor(layout1, m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(layout, 1, 0), textureId, id, 1);
}

void CRenderSystem::addEntityDescriptorSetInterface(common::entityId64 id, en_vk_utils::EPipelineLayoutType layout, uint32_t textureId)
{
	const auto layout1 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(layout, 0);
	addTextureDescriptor(layout1, m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(layout, 0, 0), textureId, id, 0);
}

void CRenderSystem::addEntityDescriptorSetAnimated(common::entityId64 id, en_vk_utils::EPipelineLayoutType layout, size_t uniformBufferSize2, uint32_t textureId)
{
	const auto layout1 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(layout, 1);
	const auto layout2 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(layout, 2);

	addTextureDescriptor(layout1, m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(layout, 1, 0), textureId, id, 1);
	addBufferDescriptor(layout2, m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(layout, 0, 0), uniformBufferSize2, id, 2);
}

void CRenderSystem::getMeshInfo(uint32_t meshId, SComputeInit& result, SMeshDesc& meshInfo)
{
	if(auto* buffer = m_renderResources.getMesh(meshId))
	{
		if(auto* pGpuBuffer = m_renderResources.getBuffer(buffer->m_vertexIndexBuffer))
		{
			auto vxSize = buffer->m_structSize * buffer->m_sizeVertices;
			auto indexSize = sizeof(uint32_t) * buffer->m_sizeIndexes;

			VkDescriptorBufferInfo bufferInfoVx{.buffer{pGpuBuffer->handle()}, .offset{0}, .range{vxSize}};
			result.aBufferInfoVx.push_back(bufferInfoVx);

			VkDescriptorBufferInfo bufferInfoIndex{.buffer{pGpuBuffer->handle()}, .offset{vxSize}, .range{indexSize}};
			result.aBufferInfoIndex.push_back(bufferInfoIndex);

			meshInfo.indexCount = buffer->m_sizeIndexes;

		}
	}
}

uint32_t CRenderSystem::getTriangleSize(uint32_t meshId)
{
	if(auto* buffer = m_renderResources.getMesh(meshId))
		return  buffer->m_sizeIndexes;
	return 0;
}

CGpuUploadSystem& CRenderSystem::uploadSystem()
{
	return m_uploadSystem;
}

void CRenderSystem::createTexture(vk_structs::CTexture& texture, const commons_struct::STexture& textureSource)
{
	texture.createTextureImage(m_context.logicalDevice().handle(),
							   m_context.physicalDevice().handle(),
							   textureSource.m_data,
							   textureSource.w,
							   textureSource.h,
							   textureSource.texChannels,
							   m_commandPool,
							   m_context.queue().m_graphic,
							   m_context.details());
}

void CRenderSystem::createGeometryBuffer(vk_structs::CBuffer& buffer, const commons_struct::SIndexedVx& geometry)
{
	en_model::SVertexData dataSource;
	dataSource.bufferSize1 = sizeof(geometry.m_aVx[0]);
	dataSource.vertexSize = geometry.m_aVx.size();
	dataSource.indexSize = geometry.m_aIndices.size();
	dataSource.vertexData = geometry.m_aVx.data();
	dataSource.indexData = geometry.m_aIndices.data();

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = NULL, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0};
	en_vk_utils::createMultiBufferGpu(dataSource.bufferSize1 * dataSource.vertexSize, dataSource.vertexData, sizeof(unsigned int) * dataSource.indexSize, dataSource.indexData, buffer, bufferCreateUsage3, m_context.logicalDevice().handle(), m_commandPool, m_context.queue().m_graphic, m_context.details(), &memoryAllocateFlagsInfo);
}

void CRenderSystem::createDescriptor(vk_structs::CDescriptorBufferResource& descriptor, const vk_structs::CTexture& textureIn)
{
	const auto descriptorType = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(en_vk_utils::EPipelineLayoutType::e2dInterface, 0, 0);
	const auto descriptorSetLayout = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType::e2dInterface, 0);

	std::unique_ptr<vk_structs::CUniformTexture> texture = std::make_unique<vk_structs::CUniformTexture>(descriptorType, 0);
	texture->init(0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textureIn);
	std::map<uint32_t, en_vk::SUniform> resourceIndex;
	resourceIndex[0].m_uniform = std::move(texture);

	std::vector<VkWriteDescriptorSet> aWriteDescriptorSetIn;

	for(auto& bind : resourceIndex)
		aWriteDescriptorSetIn.push_back(bind.second.m_uniform->writeDescriptorSet());

	descriptor.create(m_context.physicalDevice().handle(), m_context.logicalDevice().handle(), descriptorSetLayout, aWriteDescriptorSetIn, m_context.details());
}

void CRenderSystem::destroy()
{
	m_context.logicalDevice().waitIdle();
	m_rayTracing.clear(m_context.logicalDevice().handle());
	m_renderResources.m_descriptorManager.destroy();
	m_renderResources.m_uniformManager.destroy();
	clear();
	m_commandPool.destroy();
	m_frameSystem.destroy();
	m_context.destroy();
}
void CRenderSystem::initializeDescriptors(const SComputeInit& initCompute, common::entityId64 camera, en_vk_utils::EPipelineLayoutType type, size_t bufferSize)
{
	VkMemoryAllocateFlagsInfo flagsInfo{};
	flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	const auto layout1 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(type, 0);

	addBufferDescriptor(layout1, m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorType(type, 0, 0), bufferSize, camera, 0);

	const auto bits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	auto idObject = initCompute.m_id;

	if(idObject == 0)
		return;

	if(initCompute.aMeshInfo.empty())
		return;


	for(int frameInd{0}; frameInd < m_frameSystem.m_renderFrame2.m_framesInFlight; frameInd++)
	{
		auto meshInfo = m_renderResources.addBuffer(sizeof(SMeshDesc) * initCompute.aMeshInfo.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, nullptr, bits, &flagsInfo);
		auto ray = m_renderResources.addBuffer(sizeof(SRay), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, nullptr, bits, &flagsInfo);
		auto hitResult = m_renderResources.addBuffer(sizeof(SHit), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, nullptr, bits, &flagsInfo);


		std::unique_ptr <vk_structs::CUniformBuffer> uniformMeshInfo = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
		uniformMeshInfo->init(meshInfo, *m_renderResources.getBuffer(meshInfo));

		std::unique_ptr <vk_structs::CUniformBuffer> uniformRay = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
		uniformRay->init(ray, *m_renderResources.getBuffer(ray));

		std::unique_ptr <vk_structs::CUniformBuffer> uniformHitResult = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4);
		uniformHitResult->init(hitResult, *m_renderResources.getBuffer(hitResult));


		std::map<uint32_t, en_vk::SUniform> resourceIndex;
		//resourceIndex[0].m_uniform = std::move(uniformBuffer);
		//resourceIndex[1].m_uniform = std::move(uniformBuffer);
		resourceIndex[2].m_uniform = std::move(uniformMeshInfo);
		resourceIndex[3].m_uniform = std::move(uniformRay);
		resourceIndex[4].m_uniform = std::move(uniformHitResult);

		std::vector<VkWriteDescriptorSet> aWriteDescriptorSet;
		aWriteDescriptorSet.push_back(vk_init::writeDescriptorSetBuffer(NULL, {}, 0, 0, initCompute.aBufferInfoVx.size(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, initCompute.aBufferInfoVx.data()));
		aWriteDescriptorSet.push_back(vk_init::writeDescriptorSetBuffer(NULL, {}, 1, 0, initCompute.aBufferInfoIndex.size(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, initCompute.aBufferInfoIndex.data()));

		aWriteDescriptorSet.push_back(resourceIndex[2].m_uniform->writeDescriptorSet());
		aWriteDescriptorSet.push_back(resourceIndex[3].m_uniform->writeDescriptorSet());
		aWriteDescriptorSet.push_back(resourceIndex[4].m_uniform->writeDescriptorSet());

		const auto layout1 = m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType::eCompute, 0);
		m_renderResources.m_descriptorManager.addDescriptor(idObject,
															frameInd,
															0,
															layout1,
															std::move(resourceIndex),
															aWriteDescriptorSet);

		if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(idObject, frameInd, 0, 2)))
			pBuffer->load(sizeof(SMeshDesc) * initCompute.aMeshInfo.size(), initCompute.aMeshInfo.data(), 0);
	}
}

void CRenderSystem::updateUbo(const SFrameRenderData& data, int frame)
{
	if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(data.m_cameraId, frame, 0, 0)))
		pBuffer->load(sizeof(en_vk_draw::SUniformBufferObject), &data.m_camera, 0);

	for(auto& [id, meshId, uboInfo, bones] : data.m_aSkinnedMeshes)
	{
		en_model::SMatBones tmp;
		for(int t{0}; t < bones.size(); t++)
			tmp.bones[t] = bones[t];
		if(auto* pBuffer = m_renderResources.getBuffer(m_renderResources.getUniformBuffer(id, frame, 2, 0)))
			pBuffer->load(sizeof(en_model::SMatBones), &tmp, 0);
	}
}

void CRenderSystem::rayTracingDescriptors(const std::vector<uint32_t>& indexVertex, const std::vector<en_vk::SVertexRT>& points)
{
	uint32_t primitiveCount = 0;
	primitiveCount = points.size() / 3;

	std::vector<uint32_t> materialIndexList;
	materialIndexList.push_back(0);

	std::vector<en_vk_utils::SMaterial> materialList;
	materialList.push_back({});
	materialList[0].ambient = {1, 1, 1, 1};
	materialList[0].diffuse = {1, 1, 1, 1};
	materialList[0].specular = {1, 1, 1, 1};
	materialList[0].emission = {1, 1, 1, 1};
	printf("%d\n", primitiveCount);

	std::vector<en_vk::SIndexVertexBuffers> aBuffers;
	aBuffers.push_back({});//
	auto& back = aBuffers.back();
	back.m_maxVertex = (uint32_t)points.size();
	back.m_primitiveCount = (uint32_t)points.size() / 3;

	back.m_primitive.firstVertex = 0;
	back.m_primitive.primitiveCount = (uint32_t)points.size() / 3;
	back.m_primitive.primitiveOffset = 0;
	back.m_primitive.transformOffset = 0;


	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = NULL, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0};

	auto idBufferVertex = m_renderResources.addBuffer(sizeof(en_vk::SVertexRT) * points.size(), bufferCreateUsage, &m_context.queueFamilyIndices().m_presentFamily.value(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateFlagsInfo);
	auto idBufferIndex = m_renderResources.addBuffer(sizeof(uint32_t) * indexVertex.size(), bufferCreateUsage, &m_context.queueFamilyIndices().m_presentFamily.value(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateFlagsInfo);
	auto uniformBufferId = m_renderResources.addBuffer(sizeof(en_vk_draw::SUniformStructure), bufferCreateUsage2, &m_context.queueFamilyIndices().m_presentFamily.value(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateFlagsInfo);


	m_renderResources.updateBuffer(idBufferVertex, sizeof(en_vk::SVertexRT) * points.size(), points.data(), 0);
	m_renderResources.updateBuffer(idBufferIndex,  sizeof(uint32_t) * indexVertex.size(), indexVertex.data(), 0);

	auto idBlas = m_renderResources.m_uniformManager.createBlasResource(VK_DESCRIPTOR_TYPE_SAMPLER, 0); //Temporary, TODO remove VK_DESCRIPTOR_TYPE_SAMPLER


	auto* pBufferBlas = static_cast<vk_structs::CUniformBlas*>(m_renderResources.m_uniformManager.uniform(idBlas));


	auto idTextureRt = m_uploadSystem.uploadTextureRt(common::cGlobalRenderEntity, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_GENERAL);

	//m_rayTracing.createAS(pBufferTlas->m_tlas, pBufferBlas->m_blas, m_context.logicalDevice().handle(), aBuffers, *m_renderResources.getBuffer(pBufferVertex->m_bufferId), *m_renderResources.getBuffer(pBufferIndex->m_bufferId), m_context.queue().m_present, m_context.queueFamilyIndices().m_presentFamily.value(), m_context.m_details, m_context.m_commandPool);


	//const VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = NULL, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0};
	// =========================================================================
	// Build Top Level Acceleration Structure

	std::vector<VkDeviceAddress> aBottomLevelAccelerationStructureDeviceAddress;
	for(auto& buffer : aBuffers)
	{
		pBufferBlas->m_blas = m_renderResources.m_factory.createBottomLevel(m_context.logicalDevice().handle(), *m_renderResources.getBuffer(idBufferVertex), *m_renderResources.getBuffer(idBufferIndex), buffer, memoryAllocateFlagsInfo, m_context.queueFamilyIndices().m_presentFamily.value(), m_context.details(), m_commandPool, m_context.queue().m_present);
		aBottomLevelAccelerationStructureDeviceAddress.push_back(pBufferBlas->m_blas.m_accelerationStructureDeviceAddress);
	}
	std::unique_ptr <vk_structs::CUniformTlas> uniformTlas = std::make_unique<vk_structs::CUniformTlas>(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0);
	uniformTlas->m_tlas = m_renderResources.m_factory.createTopLevel(m_context.logicalDevice().handle(), aBottomLevelAccelerationStructureDeviceAddress, memoryAllocateFlagsInfo, m_context.queue().m_present, m_context.queueFamilyIndices().m_presentFamily.value(), m_context.details(), m_commandPool);


	auto idBufferMatInd = m_renderResources.addBuffer(sizeof(uint32_t) * materialIndexList.size(), bufferCreateUsage, &m_context.queueFamilyIndices().m_presentFamily.value(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateFlagsInfo);
	auto idBufferMatList = m_renderResources.addBuffer(sizeof(en_vk_utils::SMaterial) * materialList.size(), bufferCreateUsage, &m_context.queueFamilyIndices().m_presentFamily.value(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateFlagsInfo);

	m_renderResources.updateBuffer(idBufferMatInd, sizeof(uint32_t) * materialIndexList.size(), materialIndexList.data(), 0);
	m_renderResources.updateBuffer(idBufferMatList, sizeof(en_vk_utils::SMaterial) * materialList.size(), materialList.data(), 0);

	std::unique_ptr <vk_structs::CUniformBuffer> uniformBuffer = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	uniformBuffer->init(uniformBufferId, *m_renderResources.getBuffer(uniformBufferId));

	std::unique_ptr<vk_structs::CUniformBuffer> uniformBufferIndex = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
	uniformBufferIndex->init(idBufferIndex, *m_renderResources.getBuffer(idBufferIndex));

	std::unique_ptr<vk_structs::CUniformBuffer> uniformBufferVertex = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3);
	uniformBufferVertex->init(idBufferVertex, *m_renderResources.getBuffer(idBufferVertex));

	std::unique_ptr <vk_structs::CUniformTexture> uniformBufferTextureRt = std::make_unique<vk_structs::CUniformTexture>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4);
	uniformBufferTextureRt->init(idTextureRt, VK_IMAGE_LAYOUT_GENERAL, *m_renderResources.getTextureUniform(idTextureRt));

	std::map<uint32_t, en_vk::SUniform> resourceIndex;
	resourceIndex[0].m_uniform = std::move(uniformTlas);
	resourceIndex[1].m_uniform = std::move(uniformBuffer);
	resourceIndex[2].m_uniform = std::move(uniformBufferIndex);
	resourceIndex[3].m_uniform = std::move(uniformBufferVertex);
	resourceIndex[4].m_uniform = std::move(uniformBufferTextureRt);

	m_renderResources.m_descriptorManager.addDescriptor(common::cGlobalRenderEntity, 0,
														0,
														m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType::eRt, 0),
														std::move(resourceIndex));


	std::unique_ptr <vk_structs::CUniformBuffer> uniformBufferMatInd = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
	uniformBufferMatInd->init(idBufferMatInd, *m_renderResources.getBuffer(idBufferMatInd));

	std::unique_ptr <vk_structs::CUniformBuffer> uniformBufferMatList = std::make_unique<vk_structs::CUniformBuffer>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
	uniformBufferMatList->init(idBufferMatList, *m_renderResources.getBuffer(idBufferMatList));

	std::map<uint32_t, en_vk::SUniform> resourceIndex2;
	resourceIndex[0].m_uniform = std::move(uniformBufferMatInd);
	resourceIndex[1].m_uniform = std::move(uniformBufferMatList);

	m_renderResources.m_descriptorManager.addDescriptor(common::cGlobalRenderEntity, 0,
														1,
														m_renderResources.m_descriptorManager.descriptorLayout().getDescriptorSetLayout(en_vk_utils::EPipelineLayoutType::eRt, 1),
														std::move(resourceIndex2));

	m_rayTracing.shaderBindingTable(m_context.logicalDevice().handle(), m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::eRt), m_context.physicalDevice(), m_context.queueFamilyIndices().m_presentFamily.value(), m_context.details());
}

void CRenderSystem::drawLayer2d(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo)
{
	command.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::e2d));
	for(auto& [id, meshId] : frameInfo.m_a2dMeshes)
	{
		std::vector<VkDescriptorBufferBindingInfoEXT> descr;
		if(auto pDescriptorObj = m_renderResources.m_descriptorManager.getDescriptors(id); !pDescriptorObj.m_mapDescriptor.empty())
			descr = (pDescriptorObj.m_mapDescriptor.at(counter));
		recordMeshDrawCommands(command, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e2d).layout(), descr, meshId);
	}
}

void CRenderSystem::drawLayer2dMenu(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo)
{
	command.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::e2dInterface));
	for(auto& [pDescriptor, pBuffer, pDrawInfo, offset] : frameInfo.m_aInterface)
	{
		std::vector<VkDescriptorBufferBindingInfoEXT> aDescriptors;
		aDescriptors.push_back(pDescriptor->descriptorBufferBindingInfo());

		glm::vec4 t(offset.x, offset.y, 0, 0);
		command.pushConstant(m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e2dInterface).layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &t);
		//recordMeshDrawCommands(command, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk::EPipelineLayoutType::e2dInterface).layout(), aDescriptors, meshId);

		VkDeviceSize offsets[] = {0};
		command.bindVertexBuffer(0, 1, &pBuffer->handle(), offsets);
		command.bindIndexBuffer(pBuffer->handle(), pDrawInfo->m_structSize * pDrawInfo->m_sizeVertices, VK_INDEX_TYPE_UINT32);

		if(!aDescriptors.empty())
		{
			std::vector<uint32_t> bufferIndices;
			std::vector<VkDeviceSize> offsetsDesc;
			uint32_t i = 0;
			for(auto& s : aDescriptors)
			{
				bufferIndices.push_back(i);
				offsetsDesc.push_back(0);
				i++;
			}

			command.bindDescriptorBuffersEXT(aDescriptors.size(), aDescriptors.data());
			command.setDescriptorBufferOffsetsEXT(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e2dInterface).layout(), 0, aDescriptors.size(), bufferIndices.data(), offsetsDesc.data());
		}
		command.drawIndexed(static_cast<uint32_t>(pDrawInfo->m_sizeIndexes), 1, 0, 0, 0);
	}
}

void CRenderSystem::drawSkinnedMeshes(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo)
{
	auto pDescriptorCamera = m_renderResources.m_descriptorManager.getDescriptors(frameInfo.m_cameraId);

	command.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::e3dAnimation));
	for(auto& [id, meshId, uboInfo, aAnimatedMat] : frameInfo.m_aSkinnedMeshes)
	{
		std::vector<VkDescriptorBufferBindingInfoEXT> descr;
		if(auto pDescriptorObj = m_renderResources.m_descriptorManager.getDescriptors(id); !pDescriptorObj.m_mapDescriptor.empty())
		{
			descr.push_back(pDescriptorCamera.m_mapDescriptor.at(counter).front());
			descr.push_back(pDescriptorObj.m_mapDescriptor.at(counter)[0]);
			descr.push_back(pDescriptorObj.m_mapDescriptor.at(counter)[1]);
		}
		command.pushConstant(m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3dAnimation).layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), glm::value_ptr(uboInfo));
		recordMeshDrawCommands(command, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3dAnimation).layout(), descr, meshId);
	}
}

void CRenderSystem::drawStaticMeshes(vk_base::CCommandBuffer& command, const int counter, common::entityId64 selectMeshId, const std::vector<common::entityId64>& aSelected, const SFrameRenderData& frameInfo)
{
	command.bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.m_graphicPipelineManager.pipeline(en_vk_utils::EPipelineType::e3d));

	auto pDescriptorCamera = m_renderResources.m_descriptorManager.getDescriptors(frameInfo.m_cameraId);

	for(auto& [id, meshId, uboInfo] : frameInfo.m_aStaticMeshes)
	{
		if(const auto it = std::find(aSelected.begin(), aSelected.end(), id); it != aSelected.end())
		{
			continue;
		}
		if(id == selectMeshId)
			continue;

		std::vector<VkDescriptorBufferBindingInfoEXT> descr;
		if(auto pDescriptorObj = m_renderResources.m_descriptorManager.getDescriptors(id); !pDescriptorObj.m_mapDescriptor.empty())
		{
			descr.push_back(pDescriptorCamera.m_mapDescriptor.at(counter).front());
			descr.push_back(pDescriptorObj.m_mapDescriptor.at(counter)[0]);
		}

		command.pushConstant(m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3d).layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), glm::value_ptr(uboInfo));
		recordMeshDrawCommands(command, m_pipelineManager.m_graphicPipelineLayoutManager.getLayout(en_vk_utils::EPipelineLayoutType::e3d).layout(), descr, meshId);
	}
}

void CGpuUploadSystem::initialize(en_vk::CRenderResources* resources, en_vk::CVulkanContext* context, vk_structs::CCommandPool* commandPool)
{
	m_resources = resources;
	m_context = context;
	m_commandPool = commandPool;
}

void CGpuUploadSystem::uploadMesh(uint32_t meshId, const en_model::SVertexData& dataSource)
{
	if(auto it = m_resources->m_mapRender.find(meshId); it != m_resources->m_mapRender.end())
	{
		common::logError("upload failed");
		return;
	}
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .pNext = NULL, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, .deviceMask = 0};
	auto& buffer = m_resources->m_mapRender[meshId];
	buffer.m_sizeIndexes = dataSource.indexSize;
	buffer.m_sizeVertices = dataSource.vertexSize;
	buffer.m_structSize = dataSource.bufferSize1;
	buffer.m_vertexIndexBuffer = m_resources->m_bufferManager.createBuffer();
	auto& gpuBuffer = *m_resources->m_bufferManager.get(buffer.m_vertexIndexBuffer);
	en_vk_utils::createMultiBufferGpu(dataSource.bufferSize1 * dataSource.vertexSize, dataSource.vertexData, sizeof(unsigned int) * dataSource.indexSize, dataSource.indexData, gpuBuffer, bufferCreateUsage3, m_context->logicalDevice().handle(), *m_commandPool, m_context->queue().m_graphic, m_context->details(), &memoryAllocateFlagsInfo);
}

void CGpuUploadSystem::uploadTexture(uint32_t textureId, const std::vector<unsigned char>& pixels, int texWidth, int texHeight, int texChannels)
{
	if(auto it = m_resources->m_aTexture.find(textureId); it != m_resources->m_aTexture.end())
		return;
	auto& texture = m_resources->m_aTexture[textureId];
	texture.createTextureImage(m_context->logicalDevice().handle(), m_context->physicalDevice().handle(), pixels, texWidth, texHeight, texChannels, *m_commandPool, m_context->queue().m_graphic, m_context->details());
}

uint32_t CGpuUploadSystem::uploadTextureRt(uint32_t textureId, const VkFormat& format, VkImageLayout imageLayout)
{
	if(auto it = m_resources->m_aTexture.find(textureId); it != m_resources->m_aTexture.end())
		return 0;
	auto& texture = m_resources->m_aTexture[textureId];
	texture = m_resources->m_factory.createTextureResourceRt(format, m_context->queueFamilyIndices().m_presentFamily.value(), m_context->queue().m_present);
	return textureId;
}

CGpuUploadSystem::CGpuUploadSystem()
{}
