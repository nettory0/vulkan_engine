#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <algorithm>
#include <glm/glm.hpp>

namespace character
{
struct CVertexText
{
    glm::vec3 m_pos;
    glm::vec3  m_norm;

    bool operator==(const CVertexText& other) const
    {
        return m_pos ==other.m_pos && m_norm == other.m_norm;
    }

    CVertexText operator-(const CVertexText& b) const
    {
        CVertexText r;
        r.m_pos.x = m_pos.x - b.m_pos.x;
        r.m_pos.y = m_pos.y - b.m_pos.y;
        r.m_pos.z = m_pos.z - b.m_pos.z;
        return r;
    }

    void computeNormal(CVertexText a, CVertexText b, CVertexText c)
    {
        CVertexText e1 = b - a;
        CVertexText e2 = c - a;
        m_norm.x = e1.m_pos.y * e2.m_pos.z - e1.m_pos.z * e2.m_pos.y;
        m_norm.y = e1.m_pos.z * e2.m_pos.x - e1.m_pos.x * e2.m_pos.z;
        m_norm.z = e1.m_pos.x * e2.m_pos.y - e1.m_pos.y * e2.m_pos.x;

        double norm = sqrt(m_norm.x * m_norm.x + m_norm.y * m_norm.y + m_norm.z * m_norm.z);
        m_norm.x /= norm;
        m_norm.y /= norm;
        m_norm.z /= norm;
    }
};

struct CVertexTextHash
{
    size_t operator()(const CVertexText& v) const
    {
        auto combine = [](size_t seed, float f)
        {
            size_t h = std::hash<float>{}(f);
            return seed ^ (h + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        };
        size_t seed = 0;
        seed = combine(seed, v.m_pos.x);
        seed = combine(seed, v.m_pos.y);
        seed = combine(seed, v.m_pos.z);
        seed = combine(seed, v.m_norm.x);
        seed = combine(seed, v.m_norm.y);
        seed = combine(seed, v.m_norm.z);
        return seed;
    }
};

struct CMeshText
{
    std::vector<CVertexText> vertices;
    std::vector<int> indices;
    float m_offset{0};
    int addVertex(const CVertexText& v)
    {
        auto result = m_vertexIndex.try_emplace(v, (int)vertices.size() + 1);
        if(result.second)
            vertices.push_back(v);

        return result.first->second;
    }

    void addTriangle(CVertexText& a, CVertexText& b, CVertexText& c)
    {
        a.computeNormal(a, b, c);
        c.m_norm.x = b.m_norm.x = a.m_norm.x;
        c.m_norm.y = b.m_norm.y = a.m_norm.y;
        c.m_norm.z = b.m_norm.z = a.m_norm.z;

        indices.push_back(addVertex(c));
        indices.push_back(addVertex(b));
        indices.push_back(addVertex(a));
    }

    std::string getOBJData()
    {
        std::stringstream objData("");

        for(int i = 0; i < vertices.size(); i++)
            objData << "v " << vertices[i].m_pos.x << " " << vertices[i].m_pos.y << " " << vertices[i].m_pos.z << "\n";

        for(int i = 0; i < indices.size(); i += 3)
            objData << "f " << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << "\n";

        return objData.str();
    }

private:
    std::unordered_map<CVertexText, int, CVertexTextHash> m_vertexIndex;
};
}
