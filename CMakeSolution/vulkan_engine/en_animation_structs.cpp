/*
 * Based on the LearnOpenGL "Skeletal Animation" chapter
 * (https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation,
 *  https://github.com/JoeyDeVries/LearnOpenGL)
 * Copyright (c) Joey de Vries and LearnOpenGL contributors.
 * Licensed under CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/),
 * see third_party_licenses/CC-BY-NC-4.0.txt
 *
 * Modified by nettory0, 2026: ported to Vulkan, renamed types, integrated into the engine.
 */
#include "en_animation_structs.h"
#include "en_animation_load.h"
#include <omp.h>

namespace en_model
{

CModel::CModel(const asset::SLoadModel& path, bool bAnimated)
{
	m_pRoot = std::make_unique<CNodeMesh>();
	m_paths = path;
	std::unordered_map<std::string, SBoneInfo> boneInfoMap;
	int boneCount{0};
	if(bAnimated)
	{
		en_io::loadModel(m_paths.m_model, *this, boneInfoMap, boneCount);
		m_animator = CAnimator(m_paths.m_model, boneInfoMap, boneCount);
	}
	else
		en_io::loadModel2(m_paths.m_model, *this, boneCount);
}

CMeshAnimated::CMeshAnimated(std::vector<SVertexBone> vertices, std::vector<unsigned int> indices, unsigned int material)
{
	m_object.m_vertices = vertices;
	m_object.m_indices = indices;
	m_materialIndex = material;
	printf("mat ind %d\n", m_materialIndex);
}

const en_vk_draw::CDrawObjectIndexed<SVertexBone>& CMeshAnimated::getVertexData() const
{
	return m_object;
}

SVertexData CMeshAnimated::getVertexDataSource() const
{
	SVertexData result;
	result.bufferSize1 = sizeof(en_model::SVertexBone);
	result.vertexSize = getVertexData().m_vertices.size();
	result.indexSize = getVertexData().m_indices.size();
	result.vertexData = getVertexData().m_vertices.data();
	result.indexData = getVertexData().m_indices.data();

	return result;
}

CMeshStatic::CMeshStatic(std::vector<commons_struct::SVertex> vertices, std::vector<unsigned int> indices, unsigned int material)
{
	m_object.m_vertices = vertices;
	m_object.m_indices = indices;
	m_materialIndex = material;
}

const en_vk_draw::CDrawObjectIndexed<commons_struct::SVertex>& CMeshStatic::getVertexData() const
{
	return m_object;
}

SVertexData CMeshStatic::getVertexDataSource() const
{
	SVertexData result;
	result.bufferSize1 = sizeof(commons_struct::SVertex);
	result.vertexSize= getVertexData().m_vertices.size();
	result.indexSize = getVertexData().m_indices.size();
	result.vertexData = getVertexData().m_vertices.data();
	result.indexData = getVertexData().m_indices.data();

	return result;
}

CAnimator::CAnimator(const std::string& animationPath, std::unordered_map<std::string, SBoneInfo>& boneInfoMap, int& boneCount)
{
	en_io::loadAnimation(animationPath, boneInfoMap, boneCount, *this);
}

void CAnimator::updateAnimation(float dt)
{
	if(m_activeAnimation < 0 || m_activeAnimation >= (int)m_aAnimation.size())
		return;
	m_currentTime += m_aAnimation[m_activeAnimation].getTicksPerSecond() * dt;
	m_currentTime = fmod(m_currentTime, m_aAnimation[m_activeAnimation].getDuration());
	for(auto& [name, bone] : m_aAnimation[m_activeAnimation].getBoneMap())
		bone.update(m_currentTime);
	m_aAnimation[m_activeAnimation].calculateBoneTransform(glm::mat4(1.0f), m_finalBoneMatrices);
}

void CAnimator::setActiveAnimation(std::string name)
{
	if(m_activeAnimation > -1 && m_activeAnimation < (int)m_aAnimation.size()
	   && m_aAnimation[m_activeAnimation].m_animationName == name)
		return;


	int counter{0};
	for(auto& an : m_aAnimation)
	{
		if(an.m_animationName == name)
		{
			m_activeAnimation = counter;
			m_currentTime = 0;
			break;
		}
		counter++;
	}
}

CBone::CBone(const CBone& other)
{
	*this = other;
}
CBone& CBone::operator= (const CBone& other)
{
	m_positions = other.m_positions;
	m_rotations = other.m_rotations;
	m_scales = other.m_scales;

	m_localTransform = other.m_localTransform;
	m_name = other.m_name;
	m_id = other.m_id;
	m_offset = other.m_offset;
	return *this;
}


void CBone::update(float animationTime)
{
	m_localTransform = interpolateTranslation(animationTime) * interpolateRotation(animationTime) * interpolateScaling(animationTime);
	//m_localTransform = interpolateScaling(animationTime) * interpolateRotation(animationTime) * interpolateTranslation(animationTime);//in some models
}

int CBone::getPositionIndex(float animationTime)
{
	int counter{0};
	for(auto& pos : m_positions)
	{
		if(animationTime < pos.timeStamp)
		{
			if(counter == 0)
				return m_positions.size() - 1;
			return counter - 1;
		}
		counter++;
	}
	return 0;
}

int CBone::getRotationIndex(float animationTime)
{
	int counter{0};
	for(auto& rotation : m_rotations)
	{
		if(animationTime < rotation.timeStamp)
		{
			if(counter == 0)
				return m_rotations.size() - 1;
			return counter - 1;
		}
		counter++;
	}
	return 0;
}

int CBone::getScaleIndex(float animationTime)
{
	int counter{0};
	for(auto& scale : m_scales)
	{
		if(animationTime < scale.timeStamp)
		{
			if(counter == 0)
				return m_scales.size() - 1;
			return counter - 1;
		}
		counter++;
	}
	return 0;
}

float CBone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

glm::mat4 CBone::interpolateTranslation(float animationTime)
{
	if(1 == m_positions.size())
		return glm::translate(glm::mat4(1.0f), m_positions[0].position);

	const int p0Index = getPositionIndex(animationTime);
	const int p1Index = p0Index == (m_positions.size() - 1) ? 0 : p0Index + 1;
	auto& pos = m_positions[p0Index];
	auto& pos1 = m_positions[p1Index];
	const float scaleFactor = getScaleFactor(pos.timeStamp, pos1.timeStamp, animationTime);
	return glm::translate(glm::mat4(1.0f), glm::mix(pos.position, pos1.position, scaleFactor));
}

glm::mat4 CBone::interpolateRotation(float animationTime)
{
	if(1 == m_rotations.size())
		return glm::toMat4(glm::normalize(m_rotations[0].orientation));

	const int p0Index = getRotationIndex(animationTime);
	const int p1Index = p0Index == (m_rotations.size() - 1) ? 0 : p0Index + 1;
	auto& rot = m_rotations[p0Index];
	auto& rot1 = m_rotations[p1Index];
	const float scaleFactor = getScaleFactor(rot.timeStamp, rot1.timeStamp, animationTime);
	return glm::toMat4(glm::normalize(glm::slerp(rot.orientation, rot1.orientation, scaleFactor)));
}

glm::mat4 CBone::interpolateScaling(float animationTime)
{
	if(1 == m_scales.size())
		return glm::scale(glm::mat4(1.0f), m_scales[0].scale);

	const int p0Index = getScaleIndex(animationTime);
	const int p1Index = p0Index == (m_scales.size() - 1) ? 0 : p0Index + 1;
	auto& scale = m_scales[p0Index];
	auto& scale1 = m_scales[p1Index];
	const float scaleFactor = getScaleFactor(scale.timeStamp, scale1.timeStamp, animationTime);
	return glm::scale(glm::mat4(1.0f), glm::mix(scale.scale, scale1.scale, scaleFactor));
}

CBone* CAnimation::findBone(const std::string& name)
{
	if(auto it = m_bones.find(name); it != m_bones.end())
		return &m_bones[name];
	return nullptr;
}

void CAnimation::calculateBoneTransform(const glm::mat4& parentTransform, std::vector<glm::mat4>& finalBoneMatrices)
{
	calculateBoneTransform(m_rootNode, parentTransform, finalBoneMatrices);
}

void CAnimation::calculateBoneTransform(const SAssimpNodeData& node, const glm::mat4& parentTransform, std::vector<glm::mat4>& finalBoneMatrices)
{
	if(CBone* pBone = findBone(node.m_name))
	{
		const int boneId = pBone->getBoneID();
		if(boneId < 0 || boneId >= (int)finalBoneMatrices.size())
			return;

		auto& boneMat = finalBoneMatrices[boneId];
		boneMat = {parentTransform * pBone->getLocalTransform()};
		for(auto& childNode : node.m_children)
			calculateBoneTransform(childNode, boneMat, finalBoneMatrices);
		boneMat *= pBone->m_offset;
	}
	else
	{
		const glm::mat4 globalTransformation{parentTransform * node.m_transformation};
		for(auto& childNode : node.m_children)
			calculateBoneTransform(childNode, globalTransformation, finalBoneMatrices);
	}
}

}