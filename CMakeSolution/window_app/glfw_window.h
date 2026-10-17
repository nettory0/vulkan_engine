#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <array>
#include "en_core.h"

class CGlWindow
{

public:
	CGlWindow();
	~CGlWindow();
	void initWindow(void* parent, GLFWkeyfun keyCallback, GLFWcursorposfun mouseCallback, GLFWframebuffersizefun resize, GLFWmousebuttonfun mouseButtonCallback, GLFWscrollfun scrollCallback);
	void destroy();
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)  const;
	bool mainLoop();
	void run(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
	void runRT(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
	void update(int key, int scancode, int action, int mode);
	std::vector<const char*> getRequiredExtensions();
	GLFWwindow* m_window{ nullptr };
	double getTime();
	
	void scrollCallback(double x, double y);
	void mouseCallback(double x, double y);
	void resizeCallback(int w, int h);
	void mouseButtonCallback(int button, int action, int mods);

private:
	void updateFrameTime(float& lastFrame, float& currentFrame);
private:
	std::unique_ptr<CEngine> m_vulkan;
#ifdef NDEBUG
	const bool m_bEnableValidationLayers = false;
#else
	const bool m_bEnableValidationLayers = true;
#endif
};
