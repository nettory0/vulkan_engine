#pragma once
#include <set>
#include <array>
#include <vector>
#include <SDL3/SDL.h>
#include "en_gl_camera.h"
#include "en_core.h"

class CGlWindow
{

public:
	CGlWindow();
	~CGlWindow();
	void initWindow();
	void destroy();
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)  const;
	bool mainLoop();
	void run(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
	void runRT(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
	void update(int key, int scancode, int action, int mode);
	std::vector<const char*> getRequiredExtensions();
	SDL_Window* m_window{ nullptr };
	double getTime();

private:
	void framebufferResizeCallback(int width, int height);
	void scrollCallback(double xoffset, double yoffset);
	void mouseCallback(double x, double y);
	void resizeCallback(int w, int h);
	void keyCallback(int key, int scancode, int action, int mode);


	void updateFrameTime(float& lastFrame, float& currentFrame);
private:
	std::unique_ptr<CEngine> m_vulkan;
#ifdef NDEBUG
	const bool m_bEnableValidationLayers = false;
#else
	const bool m_bEnableValidationLayers = true;
#endif
};
