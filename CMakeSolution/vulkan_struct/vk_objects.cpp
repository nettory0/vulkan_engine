#include "vk_objects.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <cstring>
#include <set>
#include<string>
#include <stdexcept>
#include "vk_struct_builders.h"

#ifdef NDEBUG
const bool m_bEnableValidationLayers{false};
#else
const bool m_bEnableValidationLayers{true};
#endif

namespace vk_structs
{
void CCommandPool::create(const VkDevice& device, VkCommandPoolCreateFlags flags, uint32_t  queueFamilyIndices, const int size)
{
	VkCommandPoolCreateInfo poolInfo{.sType{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}, .pNext = NULL, .flags = flags, .queueFamilyIndex{queueFamilyIndices}};
	m_pool.create(device, poolInfo);
	createCommandBuffers(size); //16
}

void CCommandPool::free()
{
	for(auto& command : m_commandBuffers)
	{
		m_pool.freeCommandBuffers(1, &command.handle());
	}
}

void CCommandPool::destroy()
{
	free();
	m_pool.destroy();
}
void CCommandPool::createCommandBuffers(const int size)
{
	m_commandBuffers.resize(size);
	for(auto& command : m_commandBuffers)
	{
		command.setDevice(m_pool.device());
		m_pool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &command.handle());
	}
}

vk_base::CCommandBuffer CCommandPool::beginSingleTimeCommands()
{
	vk_base::CCommandBuffer command(m_pool.device());
	m_pool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &command.handle());

	VkCommandBufferBeginInfo beginInfo{vk_init::commandBufferBeginInfo(nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)};
	command.beginCommandBuffer(beginInfo);

	return command;
}
void CCommandPool::endSingleTimeCommands(vk_base::CCommandBuffer& commandBuffer, vk_base::CQueue& queue)
{
	commandBuffer.endCommandBuffer();
	queue.submit(vk_init::submitInfo(nullptr, 0, nullptr, nullptr, 1, &commandBuffer.handle(), 0, nullptr), VK_NULL_HANDLE);
	queue.waitIdle();
	m_pool.freeCommandBuffers(1, &commandBuffer.handle());
}

void CCommandPool::freeSingleTimeCommand(vk_base::CCommandBuffer& commandBuffer)
{
	m_pool.freeCommandBuffers(1, &commandBuffer.handle());
}

void SPipeline::destroy()
{
	m_pipeline.destroy();
}

void SPipeline::createRayTracing(const VkDevice& device, const VkRayTracingPipelineCreateInfoKHR& rayTracingPipelineCreateInfo)
{
	m_pipeline.createRT(device, rayTracingPipelineCreateInfo);
}

void SPipeline::create(const VkDevice& device, const VkGraphicsPipelineCreateInfo& config)//const VkDevice& device, const SSupportDetails& details, VkSampleCountFlagBits msaa, std::vector<VkPipelineShaderStageCreateInfo>& shaders, VkPrimitiveTopology topoType, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayoutCreateInfo>& info)
{
	m_pipeline.create(device, config);
}

void SPipeline::createCompute(const VkDevice& device, const VkComputePipelineCreateInfo& config)
{
	m_pipeline.createCompute(device, config);
}

VkPresentModeKHR SSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height)
{
	if(capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

	actualExtent.width = std::clamp(
		actualExtent.width,
		capabilities.minImageExtent.width,
		capabilities.maxImageExtent.width
	);

	actualExtent.height = std::clamp(
		actualExtent.height,
		capabilities.minImageExtent.height,
		capabilities.maxImageExtent.height
	);

	return actualExtent;
}

void SSwapchain::createSwapChain(
	const VkDevice& device,
	const VkSurfaceKHR& surface,
	const SSupportDetails& swapChainSupport,
	const SQueueFamilyIndices& indices,
	bool enableTransferDst)
{
	uint32_t imageCount = swapChainSupport.m_swapChainInfo.m_capabilities.minImageCount + 1;

	if(swapChainSupport.m_swapChainInfo.m_capabilities.maxImageCount > 0 &&
	   imageCount > swapChainSupport.m_swapChainInfo.m_capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.m_swapChainInfo.m_capabilities.maxImageCount;
	}


	const VkSurfaceFormatKHR& surfaceFormat = swapChainSupport.m_swapChainConfig.m_surfaceFormat;
	const VkPresentModeKHR presentMode = swapChainSupport.m_swapChainConfig.m_presentMode;
	const VkExtent2D& extent = chooseSwapExtent(swapChainSupport.m_swapChainInfo.m_capabilities, 1920, 1080);

	// ---------------------------
	// 3. Swapchain create info
	// ---------------------------
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;

	// usage flags
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if(enableTransferDst)
	{
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	// ---------------------------
	// 4. Queue families (graphics vs present)
	// ---------------------------
	uint32_t queueFamilyIndices[] =
	{
		indices.m_graphicsFamily.value(),
		indices.m_presentFamily.value()
	};

	if(indices.m_graphicsFamily != indices.m_presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	// ---------------------------
	// 5. Remaining parameters
	// ---------------------------
	createInfo.preTransform = swapChainSupport.m_swapChainInfo.m_capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// ---------------------------
	// 6. Create
	// ---------------------------
	create(device, createInfo);
}
void SSwapchain::create(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo)
{
	m_swapchain.create(device, createInfo);
}

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for(const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for(const auto& layerProperties : availableLayers)
		{
			if(strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if(!layerFound)
			return false;
	}
	return true;
}

void SInstance::create(const std::vector<const char*>& aExtantion, const std::vector<const char*>& validationLayers, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
	if(vkGetInstanceProcAddr == NULL)
	{
		VkResult res = volkInitialize();
		printf("res %d\n", res);
	}

	if(m_bEnableValidationLayers && !checkValidationLayerSupport(validationLayers))
		throw std::runtime_error("validation layers requested, but not available!");


	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Minimal Vulkan App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;
	/*
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "NewEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;
	*/
	//VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	auto extensions = aExtantion;
	VkInstanceCreateInfo instanceInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &appInfo};
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	if(m_bEnableValidationLayers)
	{
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.pNext = nullptr;
	}
	m_instance.create(instanceInfo);
	volkLoadInstanceOnly(m_instance.handle());
	setupDebugMessenger(debugCallback);
}

void SInstance::destroy()
{
	destroyDebugMessenger();
	m_instance.destroy();
}

VkSurfaceFormatKHR SSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];
}

void SSwapchain::destroy()
{
	m_swapchain.destroy();
}

void SInstance::setupDebugMessenger(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
	if(!m_bEnableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	m_debugMessenger.create(m_instance.handle(), createInfo);
}

void SInstance::destroyDebugMessenger()
{
	if(m_bEnableValidationLayers)
		m_debugMessenger.destroy(m_instance.handle());
}

void SInstance::initVolkExternal()
{
	volkInitialize();
}

void SInstance::initVolkLoadInstanceOnlyExternal(VkInstance instance)
{
	volkLoadInstanceOnly(instance);
}

void SFrameSyncPool::createRenderFences(const VkDevice& device, int size)
{
	//m_imageRenderFences.resize(size);
	//for(auto& render : m_imageRenderFences)
	//	render.create(device);
	m_imageRenderFences.reserve(size);
	for(int t = 0; t < size; t++)
	{
		m_imageRenderFences.emplace_back();// 
		m_imageRenderFences.back().create(device);
	}
}


void SFrameSyncPool::create(const VkDevice& logicalDevice, int sizeRenderFences)
{
	createRenderFences(logicalDevice, sizeRenderFences);
}

void SFrameSyncPool::destroy()
{
	for(auto& r : m_imageRenderFences)
	{
		r.destroy();
	}

	m_imageRenderFences.clear();
}

void SFrameSyncPool::cleanRenderFences()
{
	for(auto& renderFences : m_imageRenderFences)
		renderFences.destroy();
}

void SFrameSyncPool2::createRenderFences(const VkDevice& device, int size)
{
	//m_imageRenderFences.resize(size);
	//for(auto& render : m_imageRenderFences)
	//	render.create(device);
	m_imageRenderFences.reserve(m_framesInFlight);
	for(int t = 0; t < m_framesInFlight; t++)
	{
		m_imageRenderFences.emplace_back();// 
		m_imageRenderFences.back().create(device);
	}

	m_renderFinishedSemaphores.reserve(size);
	for(int t = 0; t < size; t++)
	{
		m_renderFinishedSemaphores.emplace_back();
		m_renderFinishedSemaphores.back().create(device, vk_init::semaphoreCreateInfo(nullptr, 0));
	}
}

void SFrameSyncPool2::create(const VkDevice& logicalDevice, int sizeRenderFences)
{
	createRenderFences(logicalDevice, sizeRenderFences);
}

void SFrameSyncPool2::destroy()
{
	for(auto& r : m_imageRenderFences)
	{
		r.destroy();
	}

	for(auto& r : m_renderFinishedSemaphores)
	{
		r.destroy();
	}

	m_imageRenderFences.clear();
}

void SFrameSyncPool2::cleanRenderFences()
{
	for(auto& renderFences : m_imageRenderFences)
		renderFences.destroy();
}

CUniformBuffer::CUniformBuffer(VkDescriptorType descriptorType, int destinationBinding):CUniformBase(descriptorType, destinationBinding)
{}

CUniformBuffer::~CUniformBuffer()
{}

void CUniformBuffer::destroy()
{
	CUniformBase::destroy();
}

void CUniformBuffer::init(uint32_t bufferId, const CBuffer& resourceBuffer)
{
	m_bufferId = bufferId;
	m_descriptorInfo = {.buffer{resourceBuffer.handle()}, .offset{0}, .range{resourceBuffer.size()}};
}

VkWriteDescriptorSet CUniformBuffer::writeDescriptorSet()
{
	return vk_init::writeDescriptorSetBuffer(NULL, {}, m_destinationBinding, 0, 1, m_descriptorType, &m_descriptorInfo);
}

void SRenderTargets::create(const VkDevice& device, VkSampleCountFlagBits msaaSamples, const std::vector<VkImage>& swapChainImages, const SSupportDetails& details)
{
	auto imageCount = swapChainImages.size();

	m_framebuffersImageViews.resize(swapChainImages.size());
	for(int i{0}; i < swapChainImages.size(); i++)
		m_framebuffersImageViews[i].m_swapChainImage = swapChainImages[i];

	for(auto& swapChainFramebuffersImageView : m_framebuffersImageViews)
	{
		VkImageViewCreateInfo viewInfo{vk_init::imageViewCreateInfo(nullptr, 0, swapChainFramebuffersImageView.m_swapChainImage, VK_IMAGE_VIEW_TYPE_2D, details.m_swapChainConfig.m_surfaceFormat.format, {},
			{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1})};


		swapChainFramebuffersImageView.m_imageView.create(device, viewInfo);
	}

	m_color.createResources(device, details, msaaSamples, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, details.m_swapChainConfig.m_surfaceFormat.format);//6
	m_depth.createResources(device, details, msaaSamples, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, details.m_supportedFormat);//6
}

void SRenderTargets::createRT(const VkDevice& device, VkSampleCountFlagBits msaaSamples, const std::vector<VkImage>& swapChainImages, const SSupportDetails& details)
{
	auto imageCount = swapChainImages.size();

	m_framebuffersImageViews.resize(swapChainImages.size());
	for(int i{0}; i < swapChainImages.size(); i++)
		m_framebuffersImageViews[i].m_swapChainImage = swapChainImages[i];

	for(auto& swapChainFramebuffersImageView : m_framebuffersImageViews)
	{
		VkImageViewCreateInfo viewInfo{vk_init::imageViewCreateInfo(nullptr, 0, swapChainFramebuffersImageView.m_swapChainImage, VK_IMAGE_VIEW_TYPE_2D, details.m_swapChainInfo.m_formats[0].format,
			{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
			{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})};

		swapChainFramebuffersImageView.m_imageView.create(device, viewInfo);
	}
}

void SRenderTargets::destroy()
{
	m_color.clean();
	m_depth.clean();
	for(auto& framebuffer : m_framebuffersImageViews)
		framebuffer.destroy();
}

void SSwapchainImageView::destroy()
{
	m_imageView.destroy();
}

void SSupportDetails::init(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &m_swapChainInfo.m_capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if(formatCount)
	{
		m_swapChainInfo.m_formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_swapChainInfo.m_formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if(presentModeCount)
	{
		m_swapChainInfo.m_presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, m_swapChainInfo.m_presentModes.data());
	}

	m_swapChainConfig.m_surfaceFormat = SSwapchain::chooseSwapSurfaceFormat(m_swapChainInfo.m_formats);
	m_swapChainConfig.m_presentMode = SSwapchain::chooseSwapPresentMode(m_swapChainInfo.m_presentModes);
	findSupportedFormat(device);
	vkGetPhysicalDeviceMemoryProperties(device, &m_memProperties);
	vkGetPhysicalDeviceProperties(device, &m_physicalDeviceProperties);
	printf("%s\n", m_physicalDeviceProperties.deviceName);

	if(m_swapChainInfo.m_capabilities.currentExtent.height != UINT32_MAX)
	{
		m_swapChainConfig.m_extent = m_swapChainInfo.m_capabilities.currentExtent;
	}
	else
	{
		m_swapChainConfig.m_extent.height = 400;
		m_swapChainConfig.m_extent.width = 400;
	}
}

void SSupportDetails::findSupportedFormat(VkPhysicalDevice device)
{
	std::vector<VkFormat> candidates{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
	VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};
	VkFormatFeatureFlags features{VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT};
	for(VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device, format, &props);

		if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			m_supportedFormat = format;
			return;
		}
		else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			m_supportedFormat = format;
			return;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

uint32_t SSupportDetails::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const
{
	for(uint32_t i = 0; i < m_memProperties.memoryTypeCount; i++)
	{
		if((memoryTypeBits & (1 << i)) && (m_memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

CUniformBase::CUniformBase(VkDescriptorType descriptorType, int destinationBinding):m_descriptorType{descriptorType}, m_destinationBinding{destinationBinding}
{}

void CUniformBase::destroy()
{}

void CUniformTexture::destroy()
{
	CUniformBase::destroy();
}

void CUniformTexture::init(uint32_t texturId, VkImageLayout imageLayout, const CTexture& resourceTexture)
{
	m_textureId = texturId;
	m_descriptorInfo = {.sampler{resourceTexture.m_textureSampler.handle()}, .imageView{resourceTexture.m_texture.view()}, .imageLayout{imageLayout}};
}

VkWriteDescriptorSet CUniformTexture::writeDescriptorSet()
{
	return vk_init::writeDescriptorSetImage(NULL, {}, m_destinationBinding, 0, m_descriptorType, &m_descriptorInfo);
}

VkWriteDescriptorSet CUniformTlas::writeDescriptorSet()
{
	m_asDescriptorInfo = vk_init::writeDescriptorSetAccelerationStructureKHR(nullptr, 1, &m_tlas.m_accelerationStructureHandle);

	return vk_init::writeDescriptorSetBuffer(&m_asDescriptorInfo, {}, m_destinationBinding, 0, 1, m_descriptorType, NULL); //VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
}

CUniformTlas::~CUniformTlas()
{}

VkWriteDescriptorSet CUniformBlas::writeDescriptorSet()
{
	return VkWriteDescriptorSet();
}

CUniformBlas::~CUniformBlas()
{}

CUniformBlas::CUniformBlas(VkDescriptorType descriptorType, int destinationBinding):CUniformBase(descriptorType, destinationBinding)
{}

void CUniformBlas::destroy()
{
	m_blas.clean();
}

CUniformTlas::CUniformTlas(VkDescriptorType descriptorType, int destinationBinding):CUniformBase(descriptorType, destinationBinding)
{}

void CUniformTlas::destroy()
{
	m_tlas.clean();
}

CUniformTexture::CUniformTexture(VkDescriptorType descriptorType, int destinationBinding):CUniformBase(descriptorType, destinationBinding)
{}

CUniformTexture::~CUniformTexture()
{}

void CDescriptorBufferResource::create(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkDescriptorSetLayout layout,
	const std::vector<VkWriteDescriptorSet>& writes,
	const SSupportDetails& details)
{
	auto& vk = vk_init::volkTable(device);
	// =========================
	// Descriptor buffer properties
	// =========================
	VkPhysicalDeviceDescriptorBufferPropertiesEXT descProps{};
	descProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

	VkPhysicalDeviceProperties2 props2{};
	props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	props2.pNext = &descProps;

	vkGetPhysicalDeviceProperties2(physicalDevice, &props2);

	// =========================
	// Validate layout (VERY IMPORTANT)
	// =========================
	// If this is NOT descriptor-buffer layout -> AMD will explode
	// (you should ideally assert feature at creation time)

	// =========================
	// Get layout size
	// =========================
	VkDeviceSize layoutSize = 0;
	vk.vkGetDescriptorSetLayoutSizeEXT(device, layout, &layoutSize);
	//printf("layoutSize %llu\n", (unsigned long long)layoutSize);
	if(layoutSize == 0)
		throw std::runtime_error("DescriptorSetLayoutSize is 0 (invalid layout or missing descriptorBuffer feature)");

	// =========================
	// CPU staging buffer for descriptor blob
	// =========================
	std::vector<uint8_t> descriptorData(layoutSize);
	std::memset(descriptorData.data(), 0, descriptorData.size());

	// =========================
	// Iterate writes
	// =========================
	for(const auto& write : writes)
	{
		VkDeviceSize bindingOffset = 0;

		vk.vkGetDescriptorSetLayoutBindingOffsetEXT(device, layout, write.dstBinding, &bindingOffset);

		if(write.descriptorCount > 1)
		{
			//printf("need new implementation");
		}
		for(uint32_t i = 0; i < write.descriptorCount; ++i)
		{
			VkDeviceSize descriptorSize = 0;

			switch(write.descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			descriptorSize = descProps.uniformBufferDescriptorSize;
			break;

			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			descriptorSize = descProps.storageBufferDescriptorSize;
			break;

			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			descriptorSize = descProps.combinedImageSamplerDescriptorSize;
			break;

			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			descriptorSize = descProps.storageImageDescriptorSize;
			break;

			case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			descriptorSize = descProps.accelerationStructureDescriptorSize;
			break;

			default:
			continue;
			}

			VkDeviceSize dstOffset = bindingOffset + (write.dstArrayElement + i) * descriptorSize;
			// =========================
			// Descriptor write via EXT
			// =========================
			VkDescriptorGetInfoEXT getInfo{};
			getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
			getInfo.type = write.descriptorType;

			// =========================
			// BUFFER DESCRIPTORS
			// =========================
			VkDescriptorAddressInfoEXT descAddr{};
			VkDescriptorImageInfo imgInfo{};
			if(write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
			{
				VkBufferDeviceAddressInfo addrInfo{};
				addrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				addrInfo.buffer = write.pBufferInfo[i].buffer;

				VkDeviceAddress baseAddr = vk.vkGetBufferDeviceAddress(device, &addrInfo);


				descAddr.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
				descAddr.address = baseAddr + write.pBufferInfo[i].offset;
				descAddr.range = write.pBufferInfo[i].range;
				descAddr.format = VK_FORMAT_UNDEFINED;

				getInfo.data.pUniformBuffer = &descAddr;
			}

			// =========================
			// IMAGE + SAMPLER (FIXED: NO dangling pointers)
			// =========================
			else if(write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{

				imgInfo.sampler = write.pImageInfo[i].sampler;
				imgInfo.imageView = write.pImageInfo[i].imageView;
				imgInfo.imageLayout = write.pImageInfo[i].imageLayout;

				getInfo.data.pCombinedImageSampler = &imgInfo;
			}

			else if(write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			{

				imgInfo.sampler = VK_NULL_HANDLE;
				imgInfo.imageView = write.pImageInfo[i].imageView;
				imgInfo.imageLayout = write.pImageInfo[i].imageLayout;

				getInfo.data.pStorageImage = &imgInfo;
			}

			// =========================
			// ACCELERATION STRUCTURE (correct EXT usage)
			// =========================
			else if(write.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
			{
				const auto* asInfo = reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureKHR*>(write.pNext);

				if(!asInfo || !asInfo->pAccelerationStructures)
					continue;

				VkDescriptorAddressInfoEXT addr{};
				addr.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;

				// NOTE: EXT descriptor buffer uses address form
				VkAccelerationStructureDeviceAddressInfoKHR asAddrInfo{};
				asAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
				asAddrInfo.accelerationStructure = asInfo->pAccelerationStructures[i];

				VkDeviceAddress addrValue = vk.vkGetAccelerationStructureDeviceAddressKHR(device, &asAddrInfo);

				addr.address = addrValue;
				addr.range = VK_WHOLE_SIZE;

				getInfo.data.accelerationStructure = addr.address;
			}

			// =========================
			// WRITE DESCRIPTOR
			// =========================

			if(dstOffset + descriptorSize > layoutSize)
				throw std::runtime_error("descriptor blob overflow");

			vk.vkGetDescriptorEXT(device, &getInfo, descriptorSize, descriptorData.data() + dstOffset);
		}
	}

	// =========================
	// Create GPU buffer for descriptor blob
	// =========================
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = layoutSize;
	bufferInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkMemoryAllocateFlagsInfo flagsInfo{};
	flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	m_buffer.create(device, details, bufferInfo, &flagsInfo, memFlags);
	m_buffer.load(descriptorData.size(), descriptorData.data(), 0);

	// =========================
	// Get buffer device address
	// =========================
	VkBufferDeviceAddressInfo addrInfo1{};
	addrInfo1.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addrInfo1.buffer = m_buffer.handle();

	VkDeviceAddress descriptorBufferAddress = vk.vkGetBufferDeviceAddress(device, &addrInfo1);

	m_bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
	m_bindingInfo.address = descriptorBufferAddress;
	m_bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
}
void CDescriptorBufferResource::destroy()
{
	m_buffer.clean();
}

void CBlas::clean()
{
	if(m_accelerationStructureHandle)
		vk_init::volkTable(m_asBuffer.device()).vkDestroyAccelerationStructureKHR(m_asBuffer.device(), m_accelerationStructureHandle, NULL);
	m_asBuffer.clean();
}

void CTlas::clean()
{
	if(m_accelerationStructureHandle)
		vk_init::volkTable(m_asBuffer.device()).vkDestroyAccelerationStructureKHR(m_asBuffer.device(), m_accelerationStructureHandle, NULL);

	m_instanceBuffer.clean();
	m_asBuffer.clean();
}
CTexture::~CTexture()
{
	clean();
}
void CTexture::createTextureImage(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const std::vector<unsigned char>& pixels, int texWidth, int texHeight, int texChannels, CCommandPool& commandPool, vk_base::CQueue& queue, const SSupportDetails& details)
{
	if(pixels.empty() || texWidth == 0 || texHeight == 0)
	{
		printf("create texture fail\n");
		return;
	}

	//uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	const auto maxTex = texWidth > texHeight ? texWidth : texHeight;// std::max(texWidth, texHeight);
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(maxTex))) + 1;


	VkDeviceSize sz = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;
	CBuffer staging;
	VkBufferCreateInfo bufferInfo{vk_init::bufferCreateInfo(nullptr, 0, sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr)};
	staging.create(device, details, bufferInfo, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	staging.load((size_t)sz, pixels.data(), 0);
	m_texture.createImage(device, details, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	transitionImageLayout(device, m_texture.handle(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, commandPool, queue);
	copyBufferToImage(device, staging.handle(), m_texture.handle(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), commandPool, queue);

	staging.clean();
	generateMipmaps(device, physicalDevice, commandPool, m_texture.handle(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, queue);


	VkImageViewCreateInfo viewInfo = CImage::defaultcreateCreateInfo(m_texture.handle(), device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	m_texture.createImageView(device, viewInfo);

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	m_textureSampler.create(device, samplerInfo);
}

void CTexture::transitionImageLayout(const VkDevice& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, CCommandPool& commandPool, vk_base::CQueue& queue)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw std::invalid_argument("unsupported layout transition!");

	auto commandBuffer = commandPool.beginSingleTimeCommands();
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	commandPool.endSingleTimeCommands(commandBuffer, queue);
}

void transitionImageLayout(
	VkDevice device,
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t mipLevels,
	VkImageAspectFlags aspectMask,
	CCommandPool& commandPool,
	vk_base::CQueue& queue)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectMask;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask =
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else
		throw std::invalid_argument("unsupported layout transition!");

	auto commandBuffer = commandPool.beginSingleTimeCommands();
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	commandPool.endSingleTimeCommands(commandBuffer, queue);
}

void CTexture::clean()
{
	m_textureSampler.destroy();
	m_texture.clean();
}

void CTexture::copyBufferToImage(const VkDevice& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, CCommandPool& commandPool, vk_base::CQueue& queue)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	auto commandBuffer = commandPool.beginSingleTimeCommands();
	commandBuffer.copyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	commandPool.endSingleTimeCommands(commandBuffer, queue);
}

void CTexture::generateMipmaps(const VkDevice& device, const VkPhysicalDevice& physicalDevice, CCommandPool& commandPool, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, vk_base::CQueue& queue)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("texture image format does not support linear blitting!");

	auto commandBuffer = commandPool.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for(uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		commandBuffer.blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if(mipWidth > 1) mipWidth /= 2;
		if(mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	commandBuffer.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	commandPool.endSingleTimeCommands(commandBuffer, queue);
}

CPipelineBuilder::CPipelineBuilder(VkDevice device)
{
	m_device = device;
}

CPipelineBuilder::~CPipelineBuilder()
{
	for(auto& shader : m_aShaderStages)
		shader->destroy();
}

void CPipelineBuilder::set(const SSupportDetails& details, VkSampleCountFlagBits msaa, VkPrimitiveTopology topoType, bool bBlending)
{
	setImputAssambly({}, topoType, VK_FALSE);
	setViewportState(0.0f, 0.0f, (float)details.m_swapChainConfig.m_extent.width, (float)details.m_swapChainConfig.m_extent.height, 0.0f, 1.0f, {0, 0}, details.m_swapChainConfig.m_extent);

	m_rasterizer = vk_init::pipelineRasterizationStateCreateInfo(nullptr, {}, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, {}, VK_FALSE, {}, {}, {}, 1.0f);

	setMultisampling(msaa);
	setDepthStencilState(0, VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE, {}, {}, 0.f, 0.f);
	setColorBlending(bBlending);
}

VkGraphicsPipelineCreateInfo CPipelineBuilder::initPipelineCreateInfo(VkPipelineRenderingCreateInfo* renderingCreateInfo)
{
	if(renderingCreateInfo)
		return vk_init::graphicsPipelineCreateInfo(renderingCreateInfo, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, m_shaders.size(), m_shaders.data(), &m_vertexInputInfo, &m_inputAssembly, nullptr, &m_viewportState, &m_rasterizer, &m_multisampling, &m_depthStencil,
												   &m_colorBlending, nullptr, m_layout, nullptr, 0, VK_NULL_HANDLE, 0);

	return vk_init::graphicsPipelineCreateInfo(nullptr, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, m_shaders.size(), m_shaders.data(), &m_vertexInputInfo, &m_inputAssembly, nullptr, &m_viewportState, &m_rasterizer, &m_multisampling, &m_depthStencil,
											   &m_colorBlending, nullptr, m_layout, m_renderPass, 0, VK_NULL_HANDLE, 0);
}

VkRayTracingPipelineCreateInfoKHR CPipelineBuilder::initPipelineCreateInfoRT()
{
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
		.stageCount = static_cast<uint32_t>(m_shaders.size()),
		.pStages = m_shaders.data(),
		.groupCount = 4,
		.pGroups = m_rayTracingShaderGroupCreateInfoList.data(),
		.maxPipelineRayRecursionDepth = 1,
		.pLibraryInfo = NULL,
		.pLibraryInterface = NULL,
		.pDynamicState = NULL,
		.layout = m_layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0};

	return rayTracingPipelineCreateInfo;
}

VkComputePipelineCreateInfo CPipelineBuilder::initPipelineCreateInfoCompute()
{
	return vk_init::computePipelineCreateInfo(nullptr, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, m_shaders[0], m_layout, nullptr, 0);
}

void CPipelineBuilder::setImputAssambly(VkPipelineInputAssemblyStateCreateFlags flags, VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable)
{
	m_inputAssembly = vk_init::pipelineInputAssemblyStateCreateInfo(nullptr, flags, topology, primitiveRestartEnable);
}

void CPipelineBuilder::addAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
{
	m_aAttributeDescription.push_back({vk_init::vertexInputAttributeDescription(location, binding, format, offset)});
}

void CPipelineBuilder::addAttributeDescription(VkVertexInputAttributeDescription vertexInputAttributeDescription)
{
	m_aAttributeDescription.push_back(vertexInputAttributeDescription);
}

void CPipelineBuilder::setBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	m_bindingDescription = vk_init::vertexInputBindingDescription(binding, stride, inputRate);
	m_vertexInputInfo = {vk_init::pipelineVertexInputStateCreateInfo(nullptr, 0, 1, &m_bindingDescription, static_cast<uint32_t>(m_aAttributeDescription.size()), m_aAttributeDescription.data())};
}

void CPipelineBuilder::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	m_viewport = vk_init::viewport(x, y, width, height, minDepth, maxDepth);
}

void CPipelineBuilder::setDepthStencilState(
	VkPipelineDepthStencilStateCreateFlags flags,
	VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable,
	VkCompareOp depthCompareOp,
	VkBool32 depthBoundsTestEnable,
	VkBool32 stencilTestEnable,
	VkStencilOpState front,
	VkStencilOpState back,
	float minDepthBounds,
	float maxDepthBounds)
{
	m_depthStencil = vk_init::pipelineDepthStencilStateCreateInfo(nullptr, flags, depthTestEnable, depthWriteEnable, depthCompareOp, depthBoundsTestEnable, stencilTestEnable, front, back, minDepthBounds, maxDepthBounds);
}

void CPipelineBuilder::setViewportState(float x, float y, float width, float height, float minDepth, float maxDepth, VkOffset2D offset, VkExtent2D extent)
{
	setViewport(x, y, width, height, minDepth, maxDepth);

	m_scissor = {offset, extent};

	m_viewportState = vk_init::pipelineViewportStateCreateInfo(nullptr, 0, 1, &m_viewport, 1, &m_scissor);
}

void CPipelineBuilder::setColorBlending(bool bOn)
{
	if(bOn)
	{
		m_colorBlendAttachment = vk_init::pipelineColorBlendAttachmentState(
			VK_TRUE,
			VK_BLEND_FACTOR_ONE,                  // srcColorBlendFactor  ← было SRC_ALPHA
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstColorBlendFactor
			VK_BLEND_OP_ADD,                      // colorBlendOp
			VK_BLEND_FACTOR_ONE,                  // srcAlphaBlendFactor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // dstAlphaBlendFactor
			VK_BLEND_OP_ADD,                      // alphaBlendOp
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

		m_colorBlending = {vk_init::pipelineColorBlendStateCreateInfo(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &m_colorBlendAttachment, f)};
	}
	else
	{
		const auto zero = VK_BLEND_FACTOR_ZERO;
		const auto zeroOp = VK_BLEND_OP_ADD;
		m_colorBlendAttachment = vk_init::pipelineColorBlendAttachmentState(VK_FALSE, zero, zero, zeroOp, zero, zero, zeroOp, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
		m_colorBlending = {vk_init::pipelineColorBlendStateCreateInfo(nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &m_colorBlendAttachment, f)};
	}
}

void CPipelineBuilder::setMultisampling(VkSampleCountFlagBits msaa)
{
	m_multisampling = {};
	m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling.sampleShadingEnable = VK_FALSE;
	m_multisampling.rasterizationSamples = msaa;
}

void CPipelineBuilder::addShaderStageCreateInfo(VkPipelineShaderStageCreateFlags flags, VkShaderStageFlagBits stage, std::string shaderSource, const char* pName, const VkSpecializationInfo* pSpecializationInfo)
{
	m_aShaderStages.emplace_back();
	m_aShaderStages.back() = std::make_unique<vk_base::CShaderModule>();
	m_aShaderStages.back()->create(m_device, vk_init::shaderModuleCreateInfo(nullptr, 0, shaderSource.size(), reinterpret_cast<const uint32_t*>(shaderSource.data())));

	m_shaders.push_back(vk_init::pipelineShaderStageCreateInfo(nullptr, flags, stage, m_aShaderStages.back()->handle(), pName, pSpecializationInfo));
}

void CPipelineBuilder::setLayout(VkPipelineLayout layout)
{
	m_layout = layout;
}

void CPipelineBuilder::setRenderPass(VkRenderPass renderpass)
{
	m_renderPass = renderpass;
}

void CPipelineBuilder::setRayTracingShaderGroupCreateInfoList(std::vector<VkRayTracingShaderGroupCreateInfoKHR>& group)
{
	m_rayTracingShaderGroupCreateInfoList = group;
}

void CPipelineLayoutBuilder::addPushConstant(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
	m_aPushConstant.push_back({stageFlags, offset, size});
}

void CPipelineLayoutBuilder::setLayoutCreateInfo()
{
	for(auto& [ind, aBinding] : m_aBinding)
	{
		m_aLayoutInfo.push_back(vk_init::descriptorSetLayoutCreateInfo(nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, static_cast<uint32_t>(aBinding.size()), aBinding.data()));
	}
}

void CPipelineLayoutBuilder::addDescriptorSetLayoutBinding(int index, uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers)
{
	m_aBinding[index].push_back(vk_init::descriptorSetLayoutBinding(binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers));
	m_mapDescriptorType[index].m_mapDescriptorType[binding] = descriptorType;
}

void CPipelineLayoutBuilder::addDescriptorSetLayoutBinding(int index, VkDescriptorSetLayoutBinding descriptorSetLayoutBinding)
{
	m_aBinding[index].push_back(descriptorSetLayoutBinding);
	m_mapDescriptorType[index].m_mapDescriptorType[descriptorSetLayoutBinding.binding] = descriptorSetLayoutBinding.descriptorType;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for(const auto& extension : availableExtensions)
	{
		std::string extentionName{extension.extensionName};
		printf("extension supported %s\n", std::string(extension.extensionName).c_str());
		requiredExtensions.erase(extentionName);
	}
	for(auto& extention : requiredExtensions)
		printf("not extension supported %s\n", extention.c_str());
	return requiredExtensions.empty();
}

bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
	SQueueFamilyIndices queue;
	queue.findQueueFamilies(device, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);
	bool swapChainAdequate = false;
	if(extensionsSupported)
	{
		SSupportDetails swapChainSupport;
		swapChainSupport.init(device, surface);
		swapChainAdequate = !swapChainSupport.m_swapChainInfo.m_formats.empty() && !swapChainSupport.m_swapChainInfo.m_presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	return queue.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void CPipelineLayout::create(const VkDevice& device, const std::vector<VkDescriptorSetLayoutCreateInfo>& info, const std::vector<VkPushConstantRange>& aPushConstantRange)
{
	m_descriptorSetLayout.create(device, info);
	auto t = m_descriptorSetLayout.getAll();
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{vk_init::pipelineLayoutCreateInfo(nullptr, 0, t.size(), t.data(), aPushConstantRange.size(), aPushConstantRange.data())};
	m_layout.create(device, pipelineLayoutInfo);

	auto& vk = vk_init::volkTable(device);

	int layCounter = 0;
	for(auto& t : info)
	{
		for(int bind = 0; bind < t.bindingCount; bind++)
		{
			VkDeviceSize bindingOffset = 0;
			vk.vkGetDescriptorSetLayoutBindingOffsetEXT(device, m_descriptorSetLayout.setLayout()[layCounter].handle(), bind, &bindingOffset);
		}
		layCounter++;
	}
}

const CDescriptorSetLayout& CPipelineLayout::descriptorSet() const
{
	return m_descriptorSetLayout;
}

VkPipelineLayout CPipelineLayout::layout() const
{
	return m_layout.handle();
}
void CPipelineLayout::destroy()
{
	m_layout.destroy();
	m_descriptorSetLayout.destroy();
}

void SDescriptorSet::create(const VkDevice& device, VkDescriptorSetAllocateInfo& allocInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites)
{
	m_descriptorSet.create(device, allocInfo);
	for(auto& dsw : descriptorWrites)
		dsw.dstSet = m_descriptorSet.m_set;
	m_descriptorSet.update(descriptorWrites);
}

void SDescriptorSet::destroy()
{
	m_descriptorSet.destroy();
}

void CImage::createResources(const VkDevice& device, const SSupportDetails& details, VkSampleCountFlagBits msaaSamples, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags, VkFormat format)
{
	createImage(device, details, details.m_swapChainConfig.m_extent.width, details.m_swapChainConfig.m_extent.height, 1, msaaSamples, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);// VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkImageViewCreateInfo viewInfo = CImage::defaultcreateCreateInfo(handle(), device, format, aspectFlags, 1);
	createImageView(device, viewInfo);
}

void CImage::clean()
{
	m_imageView.destroy();
	m_image.destroy();
	m_imageMemory.destroy();
}

void CImage::bind(const VkDevice& device)
{
	vk_init::volkTable(device).vkBindImageMemory(device, m_image.handle(), m_imageMemory.handle(), 0);
}

void CImage::createImage(const VkDevice& device, const SSupportDetails& details, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkImageCreateInfo imageInfo{
		vk_init::imageCreateInfo(nullptr, 0, VK_IMAGE_TYPE_2D, format, {.width = width, .height = height, .depth = 1}, mipLevels, 1, numSamples, tiling, usage, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED)
	};

	create(device, details, imageInfo, nullptr, properties);
}

void CImage::create(const VkDevice& device, const SSupportDetails& details, const VkImageCreateInfo& rayTraceImageCreateInfo, const void* memoryAllocateFlagsInfo, VkMemoryPropertyFlags properties)
{
	m_image.create(device, rayTraceImageCreateInfo);
	VkMemoryRequirements memoryRequirements{m_image.memoryRequirements()};
	m_imageMemory.create(device, vk_init::memoryAllocateInfo(memoryAllocateFlagsInfo, memoryRequirements.size, details.findMemoryType(memoryRequirements.memoryTypeBits, properties)));
	bind(device);
}

void CImage::createImageView(const VkDevice& device, VkImageViewCreateInfo& viewInfo)
{
	m_imageView.create(device, viewInfo);
}

const VkImage& CImage::handle() const
{
	return m_image.handle();
};

const VkDeviceMemory& CImage::memory() const
{
	return m_imageMemory.handle();
}
const VkImageView& CImage::view() const
{
	return m_imageView.handle();
};

VkImageViewCreateInfo CImage::defaultcreateCreateInfo(VkImage image, const VkDevice& device, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo{vk_init::imageViewCreateInfo(nullptr, 0, image, VK_IMAGE_VIEW_TYPE_2D, format, {},
	{.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1})};
	return viewInfo;
}

std::vector<VkDescriptorSetLayout> CDescriptorSetLayout::getAll()
{
	std::vector<VkDescriptorSetLayout> result;
	for(auto& d : m_aSetLayout)
		result.push_back(d.handle());
	return result;
}

void CDescriptorSetLayout::create(const VkDevice& device, const std::vector < VkDescriptorSetLayoutCreateInfo>& layoutInfo)
{
	for(auto& info : layoutInfo)
	{
		m_aSetLayout.emplace_back();
		m_aSetLayout.back().create(device, info);
	}
}

void CDescriptorSetLayout::destroy()
{
	for(auto& l : m_aSetLayout)
		l.destroy();
	m_aSetLayout.clear();
}

void CSemaphoreFence::destroy()
{
	m_renderFinishedSemaphores.destroy();
	m_imageAvailableSemaphores.destroy();
	m_fence.destroy();
}

VkResult CSemaphoreFence::waitForFence()
{
	return m_fence.waitForFence();
}

const VkSemaphore& CSemaphoreFence::availableSemaphore() const
{
	return m_imageAvailableSemaphores.handle();
}

const VkSemaphore& CSemaphoreFence::renderfinishedSemaphore() const
{
	return m_renderFinishedSemaphores.handle();
}

const VkFence& CSemaphoreFence::fence() const
{
	return m_fence.handle();
}

void CSemaphoreFence::resetFence()
{
	m_fence.reset();
}

void CSemaphoreFence::create(const VkDevice& device)
{
	m_imageAvailableSemaphores.create(device, vk_init::semaphoreCreateInfo(nullptr, 0));
	m_renderFinishedSemaphores.create(device, vk_init::semaphoreCreateInfo(nullptr, 0));
	m_fence.create(device, vk_init::fenceCreateInfo(nullptr, VK_FENCE_CREATE_SIGNALED_BIT));
}

void CSemaphoreFence2::destroy()
{
	m_imageAvailableSemaphores.destroy();
	m_fence.destroy();
}

VkResult CSemaphoreFence2::waitForFence()
{
	return m_fence.waitForFence();
}

const VkSemaphore& CSemaphoreFence2::availableSemaphore() const
{
	return m_imageAvailableSemaphores.handle();
}

const VkFence& CSemaphoreFence2::fence() const
{
	return m_fence.handle();
}

void CSemaphoreFence2::resetFence()
{
	m_fence.reset();
}

void CSemaphoreFence2::create(const VkDevice& device)
{
	m_imageAvailableSemaphores.create(device, vk_init::semaphoreCreateInfo(nullptr, 0));
	m_fence.create(device, vk_init::fenceCreateInfo(nullptr, VK_FENCE_CREATE_SIGNALED_BIT));
}

const VkDevice& CLogicalDevice::handle()
{
	return m_device.device();
};

void CLogicalDevice::destroy()
{
	m_device.destroy();
}

void CLogicalDevice::waitIdle()
{
	m_device.waitIdle();
}

void CLogicalDevice::create(
	VkPhysicalDevice physicalDevice,
	const std::vector<const char*>& validationLayers,
	const SQueueFamilyIndices& indices,
	const std::vector<const char*>& deviceExtensions,
	bool enableRayTracing)
{
	// =========================
	// Queue setup
	// =========================
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = {
		indices.m_graphicsFamily.value(),
		indices.m_presentFamily.value()
	};

	if(indices.m_computeFamily.has_value())
		uniqueQueueFamilies.insert(indices.m_computeFamily.value());

	float queuePriority = 1.0f;

	for(uint32_t family : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo q{};
		q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		q.queueFamilyIndex = family;
		q.queueCount = 1;
		q.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(q);
	}

	// =========================
	// Base features chain root
	// =========================
	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	features2.features.samplerAnisotropy = VK_TRUE;
	features2.features.geometryShader = VK_TRUE;

	// =========================
	// Vulkan 1.2 features
	// =========================
	VkPhysicalDeviceVulkan12Features features12{};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

	features12.bufferDeviceAddress = VK_TRUE;
	features12.descriptorIndexing = VK_TRUE;
	features12.runtimeDescriptorArray = VK_TRUE;
	features12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;

	// =========================
	// Descriptor Buffer EXT
	// =========================
	VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBuffer{};
	descriptorBuffer.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;

	descriptorBuffer.descriptorBuffer = VK_TRUE;

	// =========================
	// Synchronization2
	// =========================
	VkPhysicalDeviceSynchronization2Features sync2{};
	sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2.synchronization2 = VK_TRUE;

	// =========================
	// Dynamic rendering
	// =========================
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamicRendering.dynamicRendering = VK_TRUE;

	// =========================
	// Ray tracing (optional)
	// =========================
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accel{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt{};

	if(enableRayTracing)
	{
		accel.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		accel.accelerationStructure = VK_TRUE;

		rt.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rt.rayTracingPipeline = VK_TRUE;

		// =========================
		// Build correct linear pNext chain
		// =========================
		features2.pNext = &features12;
		features12.pNext = &descriptorBuffer;
		descriptorBuffer.pNext = &sync2;
		sync2.pNext = &dynamicRendering;
		dynamicRendering.pNext = &accel;
		accel.pNext = &rt;
		rt.pNext = nullptr;
	}
	else
	{
		features2.pNext = &features12;
		features12.pNext = &descriptorBuffer;
		descriptorBuffer.pNext = &sync2;
		sync2.pNext = &dynamicRendering;
		dynamicRendering.pNext = nullptr;
	}

	// =========================
	// Validation layers
	// =========================
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &features2;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifndef NDEBUG
	createInfo.enabledLayerCount = 0;//static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = nullptr;//validationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	// =========================
	// Create device
	// =========================
	m_device.create(physicalDevice, createInfo);
}
void CPhysicalDevice::init(VkPhysicalDevice device)
{
	m_handle = device;
	vkGetPhysicalDeviceProperties(m_handle, &m_physicalDeviceProperties);
}

const VkPhysicalDeviceProperties& CPhysicalDevice::physicalDeviceProperties()
{
	return m_physicalDeviceProperties;
}

void CPhysicalDevice::physicalDeviceProperties2(VkPhysicalDeviceProperties2& physicalDeviceProperties)
{
	physicalDeviceProperties.properties = m_physicalDeviceProperties;
	vkGetPhysicalDeviceProperties2(handle(), &physicalDeviceProperties);
}

std::vector<VkPhysicalDevice> CPhysicalDevice::pickPhysicalDevices(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if(deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> aResult;
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for(const auto& device : devices)
	{
		if(isDeviceSuitable(device, surface, deviceExtensions))
		{
			aResult.push_back(device);
		}
	}
	return aResult;
}

const VkPhysicalDevice& CPhysicalDevice::handle() const
{
	return m_handle;
}

VkSampleCountFlagBits CPhysicalDevice::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties{m_physicalDeviceProperties};
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if(counts & VK_SAMPLE_COUNT_64_BIT)
	{
		printf("64\n"); return VK_SAMPLE_COUNT_64_BIT;
	}
	if(counts & VK_SAMPLE_COUNT_32_BIT)
	{
		printf("32\n");  return VK_SAMPLE_COUNT_32_BIT;
	}
	if(counts & VK_SAMPLE_COUNT_16_BIT)
	{
		printf("16\n");  return VK_SAMPLE_COUNT_16_BIT;
	}
	if(counts & VK_SAMPLE_COUNT_8_BIT)
	{
		printf("8\n"); return VK_SAMPLE_COUNT_8_BIT;
	}
	if(counts & VK_SAMPLE_COUNT_4_BIT)
	{
		printf("4\n"); return VK_SAMPLE_COUNT_4_BIT;
	}
	if(counts & VK_SAMPLE_COUNT_2_BIT)
	{
		printf("2\n");  return VK_SAMPLE_COUNT_2_BIT;
	}
	return VK_SAMPLE_COUNT_1_BIT;
}

void SQueueFamilyIndices::findQueueFamilies(VkPhysicalDevice device, const VkSurfaceKHR& surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	printf("Queue family size %zu\n", queueFamilies.size());
	for(const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			m_graphicsFamily = i;

		if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			m_computeFamily = i;


		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &bPresentSupport);

		if(bPresentSupport)
			m_presentFamily = i;

		if(isComplete())
			break;
		i++;
	}
	if(!isComplete())
		throw std::runtime_error("SQueueFamilyIndices is not complete!");
}

bool SQueueFamilyIndices::isComplete()
{
	return m_graphicsFamily.has_value() && m_presentFamily.has_value() && m_computeFamily.has_value();
}

CBuffer::CBuffer(CBuffer&& other) noexcept
{
	*this = std::move(other);
}

CBuffer& CBuffer::operator=(CBuffer&& other) noexcept
{
	if(this == &other)
		return *this;
	m_buffer = std::move(other.m_buffer);
	m_bufferMemory = std::move(other.m_bufferMemory);
	m_adress = other.m_adress;
	other.m_adress = 0;
	m_device = other.m_device;
	other.m_device = nullptr;
	m_bufferSize = other.m_bufferSize;
	other.m_bufferSize = 0;
	return *this;
}

CBuffer::~CBuffer()
{
	clean();
}

void CBuffer::create(const VkDevice& device, const SSupportDetails& details, const VkBufferCreateInfo& bufferInfo, const VkMemoryAllocateFlagsInfo* memoryAllocateFlagsInfo, VkMemoryPropertyFlags properties)
{
	m_device = device;
	m_bufferSize = bufferInfo.size;
	m_buffer.create(device, bufferInfo);
	VkMemoryRequirements memoryRequirements{m_buffer.memoryRequirements()};
	m_bufferMemory.create(device, vk_init::memoryAllocateInfo(memoryAllocateFlagsInfo, memoryRequirements.size, details.findMemoryType(memoryRequirements.memoryTypeBits, properties)));
	bindBufferMemory(device);

	if(memoryAllocateFlagsInfo && bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT && memoryAllocateFlagsInfo->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)
	{
		VkBufferDeviceAddressInfo info = vk_init::bufferDeviceAddressInfo(nullptr, handle());
		m_adress = vk_init::volkTable(device).vkGetBufferDeviceAddressKHR(device, &info);
	}
}

void CBuffer::clean()
{
	m_buffer.destroy();
	m_bufferMemory.destroy();
	m_bufferSize = 0;
	m_adress = 0;
	m_device = VK_NULL_HANDLE;
}

void CBuffer::bindBufferMemory(const VkDevice& device)
{
	vk_init::volkTable(device).vkBindBufferMemory(device, m_buffer.handle(), m_bufferMemory.handle(), 0);
}

void CBuffer::load(size_t size, const void* pData, size_t offset)
{
	if(m_bufferSize < size)
		printf("CBuffer:: bad size\n");

	m_bufferMemory.loadMemory(size, pData, offset);
}

void CBuffer::read(size_t size, void* pData)
{
	m_bufferMemory.readMemory(size, pData);
}
VkDeviceAddress CBuffer::adress() const
{
	return m_adress;
}
}