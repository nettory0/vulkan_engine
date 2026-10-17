#pragma once
#include "co_export.h"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <functional>

namespace commons_struct
{

struct COMMON_STRUCT STexture
{
    int h{0};
    int w{0};
    int texChannels{0};
    std::vector<unsigned char> m_data;

};

struct COMMON_STRUCT SVertex
{
    glm::vec4 pos;
    glm::vec4 normal;
    glm::vec4 color;
    glm::vec2 texCoord;
    glm::vec2 pad;
    bool operator==(const SVertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

struct COMMON_STRUCT SIndexedVx
{
    std::vector<SVertex> m_aVx;
    std::vector<uint32_t> m_aIndices;
};

COMMON_STRUCT void testFunc();
}

namespace std
{
template<> struct hash<commons_struct::SVertex>
{
    size_t operator()(const commons_struct::SVertex& vertex) const
    {
        return ((hash<glm::vec4>()(vertex.pos) ^ (hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
}