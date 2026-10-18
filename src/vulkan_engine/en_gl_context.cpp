#include <tuple>
#include "en_gl_context.h"
#include "en_gl_descriptor_manager.h"
#include "en_gl_utils.h"
#include "en_animation_structs.h"

const std::vector<const char*> deviceExtensions = 
{
	//VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};


namespace en_vk
{
CVulkanContext::CVulkanContext()
{}

CVulkanContext::~CVulkanContext()
{
	destroy();
}

void CVulkanContext::create(VkInstance instance, VkSurfaceKHR surface, bool bRt)
{
	m_surface = surface;
	m_vulkanInstance = instance;
	m_deviceManager.init(getInstance(), surface, deviceExtensions, bRt);
	m_msaaSamples = physicalDevice().getMaxUsableSampleCount();
}

void CVulkanContext::initDetails()
{
	m_deviceManager.getCurrentDevice().m_details = {};
	m_deviceManager.getCurrentDevice().m_details.init(physicalDevice().handle(), m_surface);
}
void CVulkanContext::destroy()
{
	logicalDevice().waitIdle();
	//m_commandPool.destroy();
	//m_frameSystem.destroy();
	m_deviceManager.destroy();
}

vk_structs::CPhysicalDevice& CVulkanContext::physicalDevice()
{
	return m_deviceManager.getCurrentDevice().m_physicalDevice;
}

vk_structs::CLogicalDevice& CVulkanContext::logicalDevice()
{
	return m_deviceManager.getCurrentDevice().m_logicalDevice;
}

vk_structs::SQueueFamilyIndices& CVulkanContext::queueFamilyIndices()
{
	return m_deviceManager.getCurrentDevice().m_queueFamily;
}

vk_structs::CSemaphoreFence& CSwapChainFrameSystem::getFrame(int currentFrameInd)
{
	return m_renderFrame.m_imageRenderFences[currentFrameInd];
}

VkResult CSwapChainFrameSystem::acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
	return vk_init::acquireNextImageKHR(m_swapChain.m_swapchain.device(), m_swapChain.m_swapchain.handle(), UINT64_MAX, semaphore, fence, pImageIndex);
}

void CSwapChainFrameSystem::create(VkDevice device, VkSurfaceKHR surface, const vk_structs::SQueueFamilyIndices& indices, const vk_structs::SSupportDetails& details, VkSampleCountFlagBits samples, bool bRt)
{
	m_swapChain.createSwapChain(device, surface, details, indices);

	if(bRt)
		m_frameBuffer.createRT(device, samples, m_swapChain.m_swapchain.getSwapchainImages(), details);
	else
	{
		m_frameBuffer.create(device, samples, m_swapChain.m_swapchain.getSwapchainImages(), details);
	}
	m_swapchainImageCount = m_frameBuffer.m_framebuffersImageViews.size();

	if(!bRt)
		m_computeFrame.create(device, m_swapchainImageCount);

	m_renderFrame.create(device, m_swapchainImageCount);
	m_renderFrame2.create(device, m_swapchainImageCount);
}

void CSwapChainFrameSystem::destroy()
{
	m_swapChain.destroy();
	m_frameBuffer.destroy();
	m_renderFrame.cleanRenderFences();
	m_renderFrame2.cleanRenderFences();
	m_computeFrame.cleanRenderFences();
	m_renderFrame2.destroy();
}

vk_structs::SFrameSyncPool2& CSwapChainFrameSystem::getSemaphoreFence()
{
	return m_renderFrame2;
}

VkInstance CVulkanContext::getInstance()
{
	return m_vulkanInstance;
}

VkSurfaceKHR CVulkanContext::getSurface()
{
	return m_surface;
}

const vk_structs::SSupportDetails& CVulkanContext::details()
{
	return m_deviceManager.getCurrentDevice().m_details;
}

SQueueByType& CVulkanContext::queue()
{
	return m_deviceManager.getCurrentDevice().m_aQueue;
}
void CDeviceManager::init(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions, bool bRt)
{
	auto aDevices = vk_structs::CPhysicalDevice::pickPhysicalDevices(instance, surface, deviceExtensions);//3

	m_aDevice.reserve(aDevices.size());
	for(auto& device : aDevices)
	{
		m_aDevice.emplace_back();
		m_aDevice.back().m_physicalDevice.init(device);
		m_aDevice.back().m_queueFamily.findQueueFamilies(device, surface);
	}
	const std::vector<const char*> m_validationLayers{"VK_LAYER_KHRONOS_validation"};

	getCurrentDevice().m_logicalDevice.create(getCurrentDevice().m_physicalDevice.handle(), m_validationLayers, getCurrentDevice().m_queueFamily, deviceExtensions, bRt);
	
	getCurrentDevice().m_aQueue.m_present.getDeviceQueue(getCurrentDevice().m_logicalDevice.handle(), getCurrentDevice().m_queueFamily.m_presentFamily.value());
	getCurrentDevice().m_aQueue.m_graphic.getDeviceQueue(getCurrentDevice().m_logicalDevice.handle(), getCurrentDevice().m_queueFamily.m_graphicsFamily.value());
	getCurrentDevice().m_aQueue.m_compute.getDeviceQueue(getCurrentDevice().m_logicalDevice.handle(), getCurrentDevice().m_queueFamily.m_computeFamily.value());
	getCurrentDevice().m_details.init(getCurrentDevice().m_physicalDevice.handle(), surface);
}
SDeviceContext& CDeviceManager::getCurrentDevice()
{
	return m_aDevice.front();
}
void CDeviceManager::destroy()
{
	for(auto& [physical, logical, queue, q, d] : m_aDevice)
		logical.destroy();
}
}