
#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef PHYSIC_EXPORT
    #define PHYSIC_API __declspec(dllexport)
  #else
    #define PHYSIC_API __declspec(dllimport)
  #endif
#else
  #define PHYSIC_API __attribute__((visibility("default")))
#endif