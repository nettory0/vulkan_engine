#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef TEXT_GEN_EXPORT
    #define TEXT_GEN __declspec(dllexport)
  #else
    #define TEXT_GEN __declspec(dllimport)
  #endif
#else
  #define TEXT_GEN __attribute__((visibility("default")))
#endif