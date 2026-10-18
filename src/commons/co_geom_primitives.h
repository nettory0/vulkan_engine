#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <array>
#include "co_structs.h"


namespace commons_geom_primitives
{


class COMMON_STRUCT CBoundingBox
{
public:
    CBoundingBox() = default;
    ~CBoundingBox() = default;
    std::array<glm::vec3, 8> getBoundPoints();
    glm::vec3 m_center{0, 0, 0};
    glm::vec3 m_halfDistance{0, 0, 0};
};
static CBoundingBox calculateBoundBox(const std::vector<commons_struct::SVertex>& vertices);

class COMMON_STRUCT CSphere
{
public:
    // ctor/dtor
    CSphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, bool smooth = true, int up = 3);
    ~CSphere()
    {}
private:
    void set(float radius, int sectorCount, int stackCount, bool smooth = true, int up = 3);
    // member functions
    void buildVerticesSmooth();
    void buildVerticesFlat();
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    static glm::vec3 computeFaceNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

    float m_radius{1};
    int sectorCount;                        // longitude, # of slices
    int stackCount;                         // latitude, # of stacks
    bool m_smooth{true};
    int upAxis;

public:
    commons_struct::SIndexedVx m_vxIndexed;
};

COMMON_STRUCT std::vector<commons_struct::SVertex> cube();
COMMON_STRUCT std::vector<commons_struct::SVertex> plane();
COMMON_STRUCT commons_struct::SIndexedVx sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, bool smooth = true, int up = 3);
}

