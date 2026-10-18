#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef COMMON_STRUCT_EXPORTS
    #define COMMON_STRUCT __declspec(dllexport)
  #else
    #define COMMON_STRUCT __declspec(dllimport)
  #endif
#else
  #define COMMON_STRUCT __attribute__((visibility("default")))
#endif