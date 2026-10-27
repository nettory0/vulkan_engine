#include "glfw_window.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <stdexcept>
#include <algorithm>


inline EKey GLFWKeyToEKey(int glfwKey)
{
	switch(glfwKey)
	{
	case GLFW_KEY_W: return EKey::W;
	case GLFW_KEY_A: return EKey::A;
	case GLFW_KEY_S: return EKey::S;
	case GLFW_KEY_D: return EKey::D;
	case GLFW_KEY_SPACE: return EKey::SPACE;
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL: return EKey::LEFT_CTRL;
	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT: return EKey::LEFT_SHIFT;
	case GLFW_MOUSE_BUTTON_RIGHT: return EKey::eMouseRight;
	case GLFW_MOUSE_BUTTON_LEFT: return EKey::eMouseLeft;
	case GLFW_MOUSE_BUTTON_MIDDLE: return EKey::eMouseMiddle;

	default: return EKey::UNKNOWN;
	}
}

//static std::set<CGlWindow*> apps;
const float cameraSpeed = 1;
CGlWindow::CGlWindow()
{
	m_vulkan = std::make_unique<CEngine>();
}

static void fps(int& counterFrame, std::chrono::steady_clock::time_point& timePoint)
{
	const int counter{1000};
	counterFrame++;
	if(counterFrame == counter)
	{
		counterFrame = 0;
		auto timeElapst = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timePoint);
		timePoint = std::chrono::steady_clock::now();
		int frameRate = counter * 1000 / timeElapst.count();
	}
}

CGlWindow::~CGlWindow()
{
}

void CGlWindow::initWindow(void* parent, GLFWkeyfun keyCallback, GLFWcursorposfun mouseCallback, GLFWframebuffersizefun resize, GLFWmousebuttonfun mouseButtonCallback, GLFWscrollfun scrollCallback)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_vulkan->setWindowResolution(1920, 1080);

	m_window = glfwCreateWindow(1920, 1080,  "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, parent);
	// glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCursorPosCallback(m_window, mouseCallback);
	glfwSetFramebufferSizeCallback(m_window, resize);
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	glfwSetScrollCallback(m_window, scrollCallback);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

std::vector<const char*> CGlWindow::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if(m_bEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	extensions.push_back("VK_KHR_get_physical_device_properties2");
	extensions.push_back("VK_KHR_surface");
	return extensions;
}

void CGlWindow::scrollCallback(double x, double y)
{
	if(m_vulkan)
	{
		m_vulkan->handleScroll(x, y);
	}
}

void CGlWindow::mouseCallback(double x, double y)
{
	if(m_vulkan)
	{
		m_vulkan->handleMouseMove(x, y);
	}
}

void CGlWindow::mouseButtonCallback(int button, int action, int mods)
{
	if(!m_vulkan)
		return;

	m_vulkan->handleKeyInput(GLFWKeyToEKey(button), -1, action, mods);
}

void CGlWindow::resizeCallback(int w, int h)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while(width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}
	m_vulkan->setWindowResolution(width, height);
	m_vulkan->resizeSwapchain(width, height);
}

double CGlWindow::getTime()
{
	return glfwGetTime();
}

void CGlWindow::updateFrameTime(float& lastFrame, float& currentFrame)
{
	if(!m_vulkan)
		return;
	currentFrame = getTime();
	m_vulkan->update(currentFrame - lastFrame);
	lastFrame = currentFrame;
	update(-1, -1, -1, -1);
}

void CGlWindow::runRT(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint)
{
	m_vulkan->initialize(SEngineInitConfig{instance, surface, true});
	lastFrame = getTime();
	while(mainLoop())
	{
		updateFrameTime(lastFrame, currentFrame);
		m_vulkan->renderRayTracingFrame(false);
		fps(counterFrame, timePoint);
	}
}

void CGlWindow::run(VkInstance instance, VkSurfaceKHR surface, float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint)
{
	m_vulkan->initialize(SEngineInitConfig{instance, surface, false});
	lastFrame = getTime();
	while(mainLoop())
	{
		double mouseX;
		double mouseY;

		glfwGetCursorPos(m_window, &mouseX, &mouseY);

		updateFrameTime(lastFrame, currentFrame);
		m_vulkan->renderSceneFrame(true, glm::uvec2(mouseX, mouseY));
		fps(counterFrame, timePoint);
	}
}

bool CGlWindow::mainLoop()
{
	if(!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		return true;
	}
	return false;
}

void CGlWindow::destroy()
{
	m_vulkan->destroy();
	glfwDestroyWindow(m_window);
}


VkExtent2D CGlWindow::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if(capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

void CGlWindow::update(int key, int scancode, int action, int mode)
{
	if(!m_vulkan)
		return;

	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, GL_TRUE);

	m_vulkan->handleKeyInput(GLFWKeyToEKey(key), scancode, action, mode);

}