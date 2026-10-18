#pragma once
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <list>
#include <unordered_map>
#include "glfw_window.h"
#include "surface.h"

class CApplication
{
public:
    CApplication();
    ~CApplication();
    void run();
private:
    void mainLoop3d(float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
    void mainLoopRT(float& lastFrame, int& counterFrame, float& currentFrame, std::chrono::steady_clock::time_point& timePoint);
    void cleanup();

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseCallback(GLFWwindow* window, double x, double y);
    static void resizeCallback(GLFWwindow* window, int w, int h);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
private:
    GLfloat m_deltaTime{0.0f};	// Time between current frame and last frame
    GLfloat m_lastFrame{0.0f};

    std::unique_ptr<CGlWindow> m_glWindow;

    CSurface m_surface;
    vk_structs::SInstance m_instance;
    


};
