#include "co_geom_primitives.h"
#include <cmath>


namespace commons_geom_primitives
{

namespace
{
const int cMinSectorCount = 2;
const int cMinStackCount = 2;

// the generated meshes carry no per-vertex color, so that channel stays zeroed
inline commons_struct::SVertex makeVertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv)
{
    return commons_struct::SVertex{glm::vec4{pos, 0}, glm::vec4{normal, 0}, glm::vec4{0}, uv};
}

// cos/sin of the sector angle only depend on the sector, so evaluate them once
// per sector instead of once per vertex
std::vector<glm::vec2> buildSectorTrig(int sectorCount, float sectorStep)
{
    std::vector<glm::vec2> trig(sectorCount + 1);
    for(int j = 0; j <= sectorCount; ++j)
    {
        const float sectorAngle = j * sectorStep;   // starting from 0 to 2pi
        trig[j] = glm::vec2{cosf(sectorAngle), sinf(sectorAngle)};
    }
    return trig;
}
}

CBoundingBox calculateBoundBox(const std::vector<commons_struct::SVertex>& vertices)
{
    CBoundingBox res;

    if(vertices.empty())
        return res;

    glm::vec3 posMax{vertices[0].pos};
    glm::vec3 posMin{vertices[0].pos};
    for(const auto& vertex : vertices)
    {
        const glm::vec3 pos{vertex.pos};
        posMax = glm::max(posMax, pos);
        posMin = glm::min(posMin, pos);
    }

    res.m_center = (posMax + posMin) * 0.5f;
    res.m_halfDistance = posMax - res.m_center;

    return res;
}

std::array<glm::vec3, 8> CBoundingBox::getBoundPoints()
{
    const glm::vec3& c = m_center;
    const glm::vec3& h = m_halfDistance;
    return {c + h * glm::vec3{1, 1, 1},
            c + h * glm::vec3{1, 1, -1},
            c + h * glm::vec3{1, -1, 1},
            c + h * glm::vec3{1, -1, -1},
            c + h * glm::vec3{-1, 1, 1},
            c + h * glm::vec3{-1, 1, -1},
            c + h * glm::vec3{-1, -1, 1},
            c + h * glm::vec3{-1, -1, -1}};
}

std::vector<commons_struct::SVertex> cube()
{
    const float x = 1;
    const float y = 1;
    const float z = 1;

    static const std::array<commons_struct::SVertex, 36> cubeVertices{{
        // Front
        {glm::vec4(x, y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(1, 0)},
        {glm::vec4(x, -y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(1, 1)},
        {glm::vec4(-x, -y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(0, 1)},
        {glm::vec4(x, y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},
        {glm::vec4(-x, -y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
        {glm::vec4(-x, y, z, 0), glm::vec4(0, 0, 1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},

        // Right
        {glm::vec4(x, y, z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(x, y, -z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},
        {glm::vec4(x, -y, -z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(x, y, z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(x, -y, -z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(x, -y, z, 0), glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},

        // Top
        {glm::vec4(x, y, z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(-x, y, z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
        {glm::vec4(-x, y, -z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(x, y, z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(-x, y, -z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(x, y, -z, 0), glm::vec4(0, 1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},

        // Back
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},
        {glm::vec4(x, -y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(x, y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},
        {glm::vec4(x, y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
        {glm::vec4(-x, y, -z, 0), glm::vec4(0, 0, -1, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},

        // Left
        {glm::vec4(-x, -y, -z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(-x, y, -z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
        {glm::vec4(-x, y, z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(-x, -y, -z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(-x, y, z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(-x, -y, z, 0), glm::vec4(-1, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},

        // Bottom
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(-x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0)},
        {glm::vec4(x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0)},
        {glm::vec4(x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1)},
        {glm::vec4(x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1)},
    }};

    return {cubeVertices.begin(), cubeVertices.end()};
}

std::vector<commons_struct::SVertex> plane()
{
    const float x = 1;
    const float y = 0;
    const float z = 1;

    static const std::array<commons_struct::SVertex, 6> planeVertices{{
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(0, 0)},
        {glm::vec4(-x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(1, 0)},
        {glm::vec4(x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(1, 1)},
        {glm::vec4(-x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(0, 0)},
        {glm::vec4(x, -y, z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(1, 1)},
        {glm::vec4(x, -y, -z, 0), glm::vec4(0, -1, 0, 0), glm::vec4{1, 0, 0, 0}, glm::vec2(0, 1)},
    }};

    return {planeVertices.begin(), planeVertices.end()};
}

COMMON_STRUCT commons_struct::SIndexedVx sphere(float radius, int sectorCount, int stackCount, bool smooth, int up)
{
    commons_geom_primitives::CSphere sphere{radius, sectorCount, stackCount, smooth, up};
    return sphere.m_vxIndexed;
}


///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
CSphere::CSphere(float radius, int sectors, int stacks, bool smooth, int up)
{
    set(radius, sectors, stacks, smooth, up);
}

///////////////////////////////////////////////////////////////////////////////
// setters
///////////////////////////////////////////////////////////////////////////////
void CSphere::set(float radiusIn, int sectors, int stacks, bool smoothIn, int up)
{
    if(radiusIn > 0)
        m_radius = radiusIn;
    sectorCount = sectors < cMinSectorCount ? cMinSectorCount : sectors;
    stackCount = stacks < cMinStackCount ? cMinStackCount : stacks;
    m_smooth = smoothIn;
    upAxis = (up < 1 || up > 3) ? 3 : up;
   
        m_vxIndexed.m_aVx.clear();
        m_vxIndexed.m_aIndices.clear();

    if(m_smooth)
        buildVerticesSmooth();
    else
        buildVerticesFlat();
}

///////////////////////////////////////////////////////////////////////////////
// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void CSphere::buildVerticesSmooth()
{
    const float pi = glm::pi<float>();
    const float sectorStep = 2 * pi / sectorCount;
    const float stackStep = pi / stackCount;
    const float lengthInv = 1.0f / m_radius;
    const std::vector<glm::vec2> sectorTrig = buildSectorTrig(sectorCount, sectorStep);

    m_vxIndexed.m_aVx.reserve(m_vxIndexed.m_aVx.size() + static_cast<size_t>(stackCount + 1) * (sectorCount + 1));

    for(int i = 0; i <= stackCount; ++i)
    {
        const float stackAngle = pi / 2 - i * stackStep;    // starting from pi/2 to -pi/2
        const float xy = m_radius * cosf(stackAngle);       // r * cos(u)
        const float z = m_radius * sinf(stackAngle);        // r * sin(u)
        const float t = (float)i / stackCount;              // texCoord

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            // r * cos(u) * cos(v), r * cos(u) * sin(v), r * sin(u)
            const glm::vec3 pos{xy * sectorTrig[j].x, xy * sectorTrig[j].y, z};

            // the position doubles as the normal, since |pos| == radius
            m_vxIndexed.m_aVx.push_back(makeVertex(pos, pos * lengthInv, glm::vec2{(float)j / sectorCount, t}));
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    m_vxIndexed.m_aIndices.reserve(m_vxIndexed.m_aIndices.size() + static_cast<size_t>(sectorCount) * (2 * stackCount - 2) * 3);
    for(int i = 0; i < stackCount; ++i)
    {
        unsigned int k1 = i * (sectorCount + 1);    // beginning of current stack
        unsigned int k2 = k1 + sectorCount + 1;     // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if(i != 0)
                addIndices(k1, k2, k1 + 1);         // k1---k2---k1+1

            if(i != (stackCount - 1))
                addIndices(k1 + 1, k2, k2 + 1);     // k1+1---k2---k2+1
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// generate vertices with flat shading
// each triangle is independent (no shared vertices)
///////////////////////////////////////////////////////////////////////////////
void CSphere::buildVerticesFlat()
{
    const float pi = glm::pi<float>();
    const float sectorStep = 2 * pi / sectorCount;
    const float stackStep = pi / stackCount;
    const std::vector<glm::vec2> sectorTrig = buildSectorTrig(sectorCount, sectorStep);

    // tmp vertex definition (position + tex coords, no normal)
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    // compute all vertices first, each vertex contains (x,y,z,s,t) except normal
    std::vector<Vertex> tmpVertices;
    tmpVertices.reserve(static_cast<size_t>(stackCount + 1) * (sectorCount + 1));
    for(int i = 0; i <= stackCount; ++i)
    {
        const float stackAngle = pi / 2 - i * stackStep;    // starting from pi/2 to -pi/2
        const float xy = m_radius * cosf(stackAngle);       // r * cos(u)
        const float z = m_radius * sinf(stackAngle);        // r * sin(u)
        const float t = (float)i / stackCount;              // t

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
            tmpVertices.push_back({glm::vec3{xy * sectorTrig[j].x, xy * sectorTrig[j].y, z},
                                   glm::vec2{(float)j / sectorCount, t}});
    }

    // first and last stack contribute 1 triangle per sector, every other stack a quad
    m_vxIndexed.m_aVx.reserve(m_vxIndexed.m_aVx.size() + static_cast<size_t>(sectorCount) * (6 + 4 * (stackCount - 2)));
    m_vxIndexed.m_aIndices.reserve(m_vxIndexed.m_aIndices.size() + static_cast<size_t>(sectorCount) * (2 * stackCount - 2) * 3);

    const auto pushVertex = [this](const Vertex& v, const glm::vec3& normal)
    {
        m_vxIndexed.m_aVx.push_back(makeVertex(v.pos, normal, v.uv));
    };

    int index = 0;                                  // index for vertex
    for(int i = 0; i < stackCount; ++i)
    {
        int vi1 = i * (sectorCount + 1);            // index of tmpVertices
        int vi2 = (i + 1) * (sectorCount + 1);

        for(int j = 0; j < sectorCount; ++j, ++vi1, ++vi2)
        {
            // get 4 vertices per sector
            //  v1--v3
            //  |    |
            //  v2--v4
            const Vertex& v1 = tmpVertices[vi1];
            const Vertex& v2 = tmpVertices[vi2];
            const Vertex& v3 = tmpVertices[vi1 + 1];
            const Vertex& v4 = tmpVertices[vi2 + 1];

            // if 1st stack and last stack, store only 1 triangle per sector
            // otherwise, store 2 triangles (quad) per sector
            if(i == 0) // a triangle for first stack ==========================
            {
                const glm::vec3 n = computeFaceNormal(v1.pos, v2.pos, v4.pos);
                pushVertex(v1, n);
                pushVertex(v2, n);
                pushVertex(v4, n);

                // put indices of 1 triangle
                addIndices(index, index + 1, index + 2);

                index += 3;     // for next
            }
            else if(i == (stackCount - 1)) // a triangle for last stack =========
            {
                const glm::vec3 n = computeFaceNormal(v1.pos, v2.pos, v3.pos);
                pushVertex(v1, n);
                pushVertex(v2, n);
                pushVertex(v3, n);

                // put indices of 1 triangle
                addIndices(index, index + 1, index + 2);

                index += 3;     // for next
            }
            else // 2 triangles for others ====================================
            {
                const glm::vec3 n = computeFaceNormal(v1.pos, v2.pos, v3.pos);
                pushVertex(v1, n);
                pushVertex(v2, n);
                pushVertex(v3, n);
                pushVertex(v4, n);

                // put indices of quad (2 triangles)
                addIndices(index, index + 1, index + 2);
                addIndices(index + 2, index + 1, index + 3);

                index += 4;     // for next
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// add 3 indices to array
///////////////////////////////////////////////////////////////////////////////
void CSphere::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
{
    m_vxIndexed.m_aIndices.push_back(i1);
    m_vxIndexed.m_aIndices.push_back(i2);
    m_vxIndexed.m_aIndices.push_back(i3);
}

///////////////////////////////////////////////////////////////////////////////
// return face normal of a triangle v1-v2-v3
// if a triangle has no surface (normal length = 0), then return a zero vector
///////////////////////////////////////////////////////////////////////////////
glm::vec3 CSphere::computeFaceNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
    const float cEpsilon = 0.000001f;

    // cross product of the 2 edge vectors v1-v2 and v1-v3
    const glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);

    // normalize only if the length is > 0
    const float length = glm::length(normal);
    if(length > cEpsilon)
        return normal * (1.0f / length);

    return glm::vec3{0};    // default return value (0,0,0)
}

}
