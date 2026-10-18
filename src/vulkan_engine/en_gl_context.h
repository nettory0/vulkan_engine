#pragma once
#include <vector>
#include <unordered_map>
#include "vk_struct_builders.h"
#include "vk_objects.h"
#include "en_vulkan_engine_export.h"
#include "en_graphics_pipeline.h"
#include "en_compute_system.h"

namespace en_vk
{
struct SEntityDescriptorSets;
struct SEntityDescriptorSetsForBinding;

class CSwapChainFrameSystem
{
public:
	vk_structs::CSemaphoreFence& getFrame(int frameIndex);
	VkResult acquireNextImage(VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
	void create(VkDevice device, VkSurfaceKHR surface, const vk_structs::SQueueFamilyIndices& indices, const vk_structs::SSupportDetails& details, VkSampleCountFlagBits samples, bool bRt);
	void destroy();
	int m_swapchainImageCount{0};
	vk_structs::SFrameSyncPool2& getSemaphoreFence();
public:
	vk_structs::SSwapchain m_swapChain;
	vk_structs::SFrameSyncPool m_computeFrame;
	vk_structs::SFrameSyncPool m_renderFrame;
	vk_structs::SFrameSyncPool2 m_renderFrame2;
	vk_structs::SRenderTargets m_frameBuffer;
};

struct SQueueByType
{
	vk_base::CQueue m_present;
	vk_base::CQueue m_graphic;
	vk_base::CQueue m_compute;
};

struct SDeviceContext
{
	SDeviceContext() = default;

	SDeviceContext(const SDeviceContext&) = delete;
	SDeviceContext& operator=(const SDeviceContext&) = delete;

	SDeviceContext(SDeviceContext&&) noexcept = default;
	SDeviceContext& operator=(SDeviceContext&&) noexcept = default;
	vk_structs::CPhysicalDevice m_physicalDevice;
	vk_structs::CLogicalDevice m_logicalDevice;
	vk_structs::SQueueFamilyIndices m_queueFamily;
	vk_structs::SSupportDetails m_details;
	SQueueByType m_aQueue;
};

class CDeviceManager
{
public:
	CDeviceManager() = default;

	CDeviceManager(const CDeviceManager&) = delete;
	CDeviceManager& operator=(const CDeviceManager&) = delete;

	CDeviceManager(CDeviceManager&&) noexcept = default;
	CDeviceManager& operator=(CDeviceManager&&) noexcept = default;
	void init(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& deviceExtensions, bool bRt);
	SDeviceContext& getCurrentDevice();
	void destroy();
private:
	std::vector<SDeviceContext> m_aDevice;
};

class VULKAN_ENGINE CVulkanContext
{
public:
	CVulkanContext();
	~CVulkanContext();
	void create(VkInstance instance, VkSurfaceKHR surface, bool bRt);
	vk_structs::CPhysicalDevice& physicalDevice();
	vk_structs::CLogicalDevice& logicalDevice();
	vk_structs::SQueueFamilyIndices& queueFamilyIndices();
	const vk_structs::SSupportDetails& details();
	VkInstance getInstance();
	VkSurfaceKHR getSurface();
	void destroy();
	void initDetails();
	SQueueByType& queue();
	
public:
	VkSampleCountFlagBits m_msaaSamples{VK_SAMPLE_COUNT_1_BIT};

private:
	CDeviceManager m_deviceManager;
	VkSurfaceKHR m_surface{VK_NULL_HANDLE};
	VkInstance m_vulkanInstance{VK_NULL_HANDLE};
	
	
};
}
