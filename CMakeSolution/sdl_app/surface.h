#pragma once 
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

class CSurface
{
public:
	void init(VkSurfaceKHR surface);
	void create(VkInstance instance, SDL_Window& createInfo);
	void destroy(VkInstance instance);
	VkSurfaceKHR handle() const { return m_handle; }

private:
	VkSurfaceKHR m_handle{nullptr};
};