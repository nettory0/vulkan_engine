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
#include "sdl_window.h"
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
  
    
private:
    float m_deltaTime{0.0f};	// Time between current frame and last frame
    float m_lastFrame{0.0f};

    std::unique_ptr<CGlWindow> m_glWindow;

    CSurface m_surface;
    vk_structs::SInstance m_instance;
    


};
