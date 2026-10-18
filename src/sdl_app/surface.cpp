#include "surface.h"
#include "vk_base.h"

void CSurface::init(VkSurfaceKHR surface)
{
	m_handle = surface;
}

void CSurface::create(VkInstance instance, SDL_Window& createInfo)
{
	SDL_Vulkan_CreateSurface(&createInfo, instance, nullptr, &m_handle);
}

void CSurface::destroy(VkInstance instance)
{
	vk_base::destroySurface(instance, m_handle, nullptr);
}