#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef VULKAN_STRUCT_EXPORTS
    #define VULKAN_STRUCT __declspec(dllexport)
  #else
    #define VULKAN_STRUCT __declspec(dllimport)
  #endif
#else
  #define VULKAN_STRUCT __attribute__((visibility("default")))
#endif