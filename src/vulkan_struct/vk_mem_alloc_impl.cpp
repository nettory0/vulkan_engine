// Single translation unit with the VMA implementation.
// volk.h must come first so VMA sees the Vulkan types without prototypes (VK_NO_PROTOTYPES).
#include "volk.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
