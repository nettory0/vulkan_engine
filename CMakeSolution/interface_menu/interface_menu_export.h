#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef INTERFACE_EXPORTS
    #define INTERFACE_MENU __declspec(dllexport)
  #else
    #define INTERFACE_MENU __declspec(dllimport)
  #endif
#else
  #define INTERFACE_MENU __attribute__((visibility("default")))
#endif