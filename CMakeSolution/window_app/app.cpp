#include "app.h"
#include <algorithm>

enum class ERenderPipeline
{
	e3d, eRT
};

CApplication::CApplication()
{
}

CApplication::~CApplication()
{
	m_glWindow.reset();
	glfwTerminate();
}

void CApplication::run()
{
	m_glWindow = std::make_unique<CGlWindow>();

	if(!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	m_glWindow->initWindow(this, keyCallback, mouseCallback, resizeCallback, mouseButtonCallback, scrollCallback);


	float lastFrame = m_glWindow->getTime();
	auto timePoint = std::chrono::steady_clock::now();
	int counterFrame{0};
	float currentFrame = m_glWindow->getTime();

	const ERenderPipeline p{ERenderPipeline::e3d};
	switch(p)
	{
	case ERenderPipeline::e3d:
	mainLoop3d(lastFrame, counterFrame, currentFrame, timePoint);
	break;
	case ERenderPipeline::eRT:
	mainLoopRT(lastFrame, counterFrame, currentFrame, timePoint);
	break;
	}
	cleanup();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* data,
	void* userData)
{
	if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		common::logError(std::format("[VULKAN] {}", data->pMessage));
	else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		common::logWarning(std::format("[VULKAN] {}", data->pMessage));
	else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		common::logInfo(std::format("[VULKAN] {}", data->pMessage));

	return VK_FALSE;
}

void CApplication::mainLoop3d(float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint)
{
	const std::vector<const char*> m_validationLayers{"VK_LAYER_KHRONOS_validation"};

	m_instance.create(m_glWindow->getRequiredExtensions(), m_validationLayers, debugCallback);//1
	m_surface.create(m_instance.m_instance.handle(), *m_glWindow->m_window);
	m_glWindow->run(m_instance.m_instance.handle(), m_surface.handle(), lastFrame, counterFrame, currentFrame, timePoint);
}

void CApplication::mainLoopRT(float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint)
{
	const std::vector<const char*> m_validationLayers{"VK_LAYER_KHRONOS_validation"};
	m_instance.create(m_glWindow->getRequiredExtensions(), m_validationLayers, debugCallback);//
	m_surface.create(m_instance.m_instance.handle(), *m_glWindow->m_window);

	m_glWindow->runRT(m_instance.m_instance.handle(), m_surface.handle(), lastFrame, counterFrame, currentFrame, timePoint);
}

void CApplication::cleanup()
{
	m_glWindow->destroy();
	m_surface.destroy();
	m_instance.destroy();
}

void CApplication::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));
	app->m_glWindow->scrollCallback(xoffset, yoffset);
}

void CApplication::mouseCallback(GLFWwindow* window, double x, double y)
{
	CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));
	app->m_glWindow->mouseCallback(x, y);
}

void CApplication::resizeCallback(GLFWwindow* window, int w, int h)
{
	CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));

	app->m_glWindow->resizeCallback(w, h);

}

void CApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));

	app->m_glWindow->update(key, scancode, action, mode);
}

void CApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));

	app->m_glWindow->mouseButtonCallback(button, action, mods);
}