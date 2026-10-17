#pragma once
#include <stdint.h>

namespace common
{
using entityId64 = uint64_t;

static constexpr entityId64 cInvalidID{0};
static constexpr entityId64 cGlobalRenderEntity{1};
static constexpr entityId64 cRootID{2};
}