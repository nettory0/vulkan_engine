#include "sdl_window.h"
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_keycode.h>
#include <stdexcept>
#include <algorithm>
#include "co_logging.h"


inline EKey sdlKeyToEKey(int key)
{
	switch(key)
	{
		// WASD
	case SDLK_W: return EKey::W;
	case SDLK_A: return EKey::A;
	case SDLK_S: return EKey::S;
	case SDLK_D: return EKey::D;

		// Space
	case SDLK_SPACE: return EKey::SPACE;

		// Ctrl
	case SDLK_LCTRL:
	case SDLK_RCTRL:
	return EKey::LEFT_CTRL;

	default:
	return EKey::UNKNOWN;
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
{}

void CGlWindow::initWindow()
{

	m_window = SDL_CreateWindow("SDL3 + Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if(!m_window)
	{
		common::logError("Failed to create window\n");
		return;
	}
}

std::vector<const char*> CGlWindow::getRequiredExtensions()
{
	uint32_t count_instance_extensions;
	const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

	if(instance_extensions == NULL)
	{
		return {};
	}
	std::vector<const char*> extensions(instance_extensions, instance_extensions + count_instance_extensions);

	if(m_bEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

void CGlWindow::resizeCallback(int w, int h)
{
	int width = 0, height = 0;
	SDL_GetWindowSizeInPixels(m_window, &width, &height);
	while(width == 0 || height == 0)
	{
		SDL_Event e;
		SDL_WaitEvent(&e);
		SDL_GetWindowSizeInPixels(m_window, &width, &height);
	}
	m_vulkan->setWindowResolution(width, height);
	m_vulkan->resizeSwapchain(width, height);
}

double CGlWindow::getTime()
{
	double t = SDL_GetTicks() / 1000.0;

	return t;
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

		float mouseX, mouseY;

		SDL_GetMouseState(&mouseX, &mouseY);
		//glfwGetCursorPos(m_window, &mouseX, &mouseY);

		updateFrameTime(lastFrame, currentFrame);
		m_vulkan->renderSceneFrame(true, glm::uvec2(mouseX, mouseY));
		fps(counterFrame, timePoint);
	}
}

bool CGlWindow::mainLoop()
{
	SDL_Event event;
	bool running = true;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_EVENT_QUIT:
		running = false;
		break;
		case SDL_EVENT_WINDOW_RESIZED:
		resizeCallback(event.window.data1, event.window.data2);
		break;
		case SDL_EVENT_MOUSE_MOTION:
		mouseCallback(event.motion.x, event.motion.y);
		break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			int button = event.button.button;
			EKey resultButton{EKey::UNKNOWN};
			switch(event.button.button)
			{
			case SDL_BUTTON_LEFT:
			resultButton = EKey::eMouseLeft;
			break;
			case SDL_BUTTON_RIGHT:
			resultButton = EKey::eMouseRight;
			break;
			case SDL_BUTTON_MIDDLE:
			resultButton = EKey::eMouseMiddle;
			break;
			default:
			break;
			};
			int mods = event.button.clicks;
			int action = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;

			m_vulkan->handleKeyInput(resultButton, -1, action, mods);
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL:
		scrollCallback(event.wheel.x, event.wheel.y);
		break;
		case SDL_EVENT_KEY_UP:
		keyCallback(event.key.key, event.key.scancode, 0, event.key.mod);
		break;
		case SDL_EVENT_KEY_DOWN:
		keyCallback(event.key.key, event.key.scancode, 1, event.key.mod);
		break;
		default:
		break;
		}

	}

	return running;
}

void CGlWindow::destroy()
{
	m_vulkan->destroy();
	SDL_DestroyWindow(m_window);
}


VkExtent2D CGlWindow::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if(capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		int width = 0, height = 0;
		SDL_GetWindowSizeInPixels(m_window, &width, &height);
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

	//if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(m_window, GL_TRUE);

	m_vulkan->handleKeyInput(sdlKeyToEKey(key), scancode, action, mode);

}

void CGlWindow::framebufferResizeCallback(int width, int height)
{
	//auto app = reinterpret_cast<CApplication*>(glfwGetWindowUserPointer(window));
	//  app->framebufferResized = true;
}

void CGlWindow::keyCallback(int key, int scancode, int action, int mode)
{
	//CApplication* app = static_cast<CApplication*>(glfwGetWindowUserPointer(window));

	update(key, scancode, action, mode);
}

