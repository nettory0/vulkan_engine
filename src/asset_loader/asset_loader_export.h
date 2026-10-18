#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef ASSET_EXPORTS
    #define ASSET_MODULE __declspec(dllexport)
  #else
    #define ASSET_MODULE __declspec(dllimport)
  #endif
#else
  #define ASSET_MODULE __attribute__((visibility("default")))
#endif