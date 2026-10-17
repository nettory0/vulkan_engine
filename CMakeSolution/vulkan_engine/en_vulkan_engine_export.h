#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef VULKAN_ENGINE_EXPORTS
    #define VULKAN_ENGINE __declspec(dllexport)
  #else
    #define VULKAN_ENGINE __declspec(dllimport)
  #endif
#else
  #define VULKAN_ENGINE __attribute__((visibility("default")))
#endif