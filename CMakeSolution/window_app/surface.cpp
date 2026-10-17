#include "surface.h"
#include "vk_base.h"

void CSurface::init(VkSurfaceKHR surface)
{
	m_handle = surface;
}

void CSurface::create(VkInstance instance, const GLFWwindow& createInfo)
{
	createVulkanObject(glfwCreateWindowSurface, &createInfo, instance);
}

void CSurface::destroy()
{
	destroyVulkanObject(vk_base::destroySurface);
}