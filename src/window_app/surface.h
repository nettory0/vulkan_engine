#pragma once 
#include "vk_object_template.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class CSurface: public vk_base::CVulkanObject<VkSurfaceKHR, VkInstance, GLFWwindow>
{
public:
	void init(VkSurfaceKHR surface);
	void create(VkInstance instance, const GLFWwindow& createInfo);
	void destroy();
};