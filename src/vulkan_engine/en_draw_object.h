#pragma once
#include <vector>
#include <cmath>
#include <stdexcept>
#include <map>
#include "vk_objects.h"
#include "co_geom_primitives.h"
#include "en_utils.h"
#include "pe_physic_engine.h"

namespace en_vk_draw
{

struct SUniformStructure
{
	glm::mat4 view{1};
	glm::mat4 projection{1};
	glm::vec4 m_cameraPosition = {0, 0, 0, 1};
	unsigned int frameCount{0};
};

struct SUniformBufferObject
{
	alignas(16) glm::mat4 view{1};
	alignas(16) glm::mat4 proj{1};
};

glm::mat4 makeTranslationMatrix(glm::vec3 cameraPos);
glm::mat4 makeRotationMatrixQuaternion(glm::vec3 orientation);
glm::mat4 makeScaleMatrix();

template <typename T>
class CDrawObject
{
public:
	CDrawObject() = default;
	virtual ~CDrawObject() = default;

	CDrawObject(const CDrawObject&) = delete;
	CDrawObject& operator=(const CDrawObject&) = delete;
	CDrawObject(CDrawObject&&) = default;
	CDrawObject& operator=(CDrawObject&&) = default;

	std::vector<T> m_vertices;
    commons_geom_primitives::CBoundingBox m_boundingBox;
};


template<typename T> 
class CDrawObjectIndexed:public CDrawObject<T>
{
public:
	CDrawObjectIndexed() = default;
	~CDrawObjectIndexed() = default;

	CDrawObjectIndexed(const CDrawObjectIndexed&) = delete;
	CDrawObjectIndexed& operator=(const CDrawObjectIndexed&) = delete;
	CDrawObjectIndexed(CDrawObjectIndexed&&) = default;
	CDrawObjectIndexed& operator=(CDrawObjectIndexed&&) = default;
	std::vector<uint32_t> m_indices;
};

}