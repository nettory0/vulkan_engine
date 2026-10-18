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
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <map>
#include <iostream>
#include <vector>
#include "en_gl_draw_object.h"
#include "pe_physic_engine.h"
#include "asset_loader.h"
#include "co_structs.h"

#define MAX_BONE_INFLUENCE 4

constexpr int cMaxBones = 100;


namespace en_model
{

struct SMatrixSource
{
	glm::vec3 m_pos{0};
	glm::vec3 m_scale{0};
	glm::vec4 m_rotate{0};
};

struct SMatBones
{
	glm::mat4 bones[cMaxBones];
};

struct SKeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct SKeyRotation
{
	glm::quat orientation;
	float timeStamp;
};

struct SKeyScale
{
	glm::vec3 scale;
	float timeStamp;
};

struct SBoneInfo
{
	int id{-1};
	glm::mat4 offset{1};

};

struct SMaterial2
{
	glm::vec4 ambient{0, 0, 0, 0};
	glm::vec4 diffuse{0, 0, 0, 0};
	glm::vec4 specular{0, 0, 0, 0};
	glm::vec4 emission{0, 0, 0, 0};
	int textureIndex{0};
};

struct SVertexBone
{
	glm::vec3 pos; // position
	glm::vec3 normal; // normal
	glm::vec2 tex; // texCoords
	glm::vec4 color; // normal
	glm::vec3 tangent; // tangent
	glm::vec3 bitangent; // bitangent
	int m_boneIDs[MAX_BONE_INFLUENCE]; //bone indexes which will influence this vertex
	float m_weights[MAX_BONE_INFLUENCE]; //weights from each bone
};

struct SVertexData
{
	VkDeviceSize bufferSize1;
	size_t vertexSize;
	size_t indexSize;
	const void* vertexData;
	const void* indexData;
};

class CMeshBase
{
public:
	enum class EMeshType
	{
		eStatic, eAnimated
	};
public:
	CMeshBase() = default;
	virtual ~CMeshBase() = default;
	unsigned int m_materialIndex{0};
	virtual EMeshType type() const = 0;
	virtual SVertexData getVertexDataSource() const = 0;
};

class CMeshAnimated:public CMeshBase
{
public:
	// mesh Data
	CMeshAnimated(std::vector<SVertexBone> vertices, std::vector<unsigned int> indices, unsigned int material);
	CMeshAnimated(const CMeshAnimated&) = delete;
	CMeshAnimated& operator=(const CMeshAnimated&) = delete;
	CMeshAnimated(CMeshAnimated&&) = default;
	CMeshAnimated& operator=(CMeshAnimated&&) = default;
	const en_vk_draw::CDrawObjectIndexed<SVertexBone>& getVertexData() const;
	SVertexData getVertexDataSource() const override;
	en_vk_draw::CDrawObjectIndexed<SVertexBone> m_object;
	virtual EMeshType type() const override
	{
		return EMeshType::eAnimated;
	}

};

class CMeshStatic:public CMeshBase
{
public:
	// mesh Data
	CMeshStatic(std::vector<commons_struct::SVertex> vertices, std::vector<unsigned int> indices, unsigned int material);
	CMeshStatic(const CMeshStatic&) = delete;
	CMeshStatic& operator=(const CMeshStatic&) = delete;
	CMeshStatic(CMeshStatic&&) = default;
	CMeshStatic& operator=(CMeshStatic&&) = default;
	const en_vk_draw::CDrawObjectIndexed<commons_struct::SVertex>& getVertexData() const;
	SVertexData getVertexDataSource() const override;
	en_vk_draw::CDrawObjectIndexed<commons_struct::SVertex> m_object;
	virtual EMeshType type() const override
	{
		return EMeshType::eStatic;
	}

};

class CBone
{
public:

	CBone(const CBone& other);
	CBone() = default;
	void update(float animationTime);
	CBone& operator= (const CBone& other);
	glm::mat4 getLocalTransform()
	{
		return m_localTransform;
	}
	const glm::mat4& getLocalTransform() const
	{
		return m_localTransform;
	}
	std::string getBoneName() const
	{
		return m_name;
	}
	int getBoneID()
	{
		return m_id;
	}
	int getPositionIndex(float animationTime);
	int getRotationIndex(float animationTime);
	int getScaleIndex(float animationTime);
	glm::mat4 m_offset{1};
public:
	glm::mat4 m_localTransform{1};
	std::string m_name;
	int m_id{0};
	std::vector<SKeyPosition> m_positions;
	std::vector<SKeyRotation> m_rotations;
	std::vector<SKeyScale> m_scales;
private:
	float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
	glm::mat4 interpolateTranslation(float animationTime);
	glm::mat4 interpolateRotation(float animationTime);
	glm::mat4 interpolateScaling(float animationTime);

};
struct SAssimpNodeData
{
	glm::mat4 m_transformation{1};
	std::string m_name;
	std::vector<SAssimpNodeData> m_children;
};

class CAnimation
{
public:
	CAnimation() = default;
	~CAnimation()
	{}
	CBone* findBone(const std::string& name);
	void calculateBoneTransform(const glm::mat4& parentTransform, std::vector<glm::mat4>& finalBoneMatrices);
	float getTicksPerSecond() const
	{
		return m_ticksPerSecond;
	}
	float getDuration() const
	{
		return m_duration;
	}
	const SAssimpNodeData& getRootNode() const
	{
		return m_rootNode;
	}
	std::unordered_map<std::string, CBone>& getBoneMap()
	{
		return m_bones;
	}
	std::string m_animationName;

public:
	std::unordered_map<std::string, CBone> m_bones;
	float m_duration{0.f};
	int m_ticksPerSecond{0};
	SAssimpNodeData m_rootNode;
private:
	void calculateBoneTransform(const SAssimpNodeData& node, const glm::mat4& parentTransform, std::vector<glm::mat4>& finalBoneMatrices);
};

class CAnimator
{
public:
	CAnimator() = default;
	CAnimator(const std::string& animationPath, std::unordered_map<std::string, SBoneInfo>& boneInfoMap, int& boneCount);
	void updateAnimation(float dt);
	void setActiveAnimation(std::string name);
	void calculateBoneTransform(const glm::mat4& parentTransform, std::vector<glm::mat4>& finalBoneMatrices);
	const std::vector<glm::mat4>& GetFinalBoneMatrices() const
	{
		return m_finalBoneMatrices;
	}

public:
	std::vector<glm::mat4> m_finalBoneMatrices;
	std::vector<CAnimation> m_aAnimation;
	float m_currentTime{0.0};
	int m_activeAnimation{0};
};

class CNodeMesh
{
public:
	CNodeMesh() = default;
	std::vector<std::unique_ptr<CNodeMesh>> m_aChild;
	int m_meshId{-1};
};

static void traverse(const CNodeMesh* node)
{
	for(const auto& child : node->m_aChild)
		traverse(child.get());
}

class CModel
{
public:
	// constructor, expects a filepath to a 3D model.
	CModel(const asset::SLoadModel& path, bool bAnimated);
	CModel() = default;

public:

	// model data 
	std::vector<std::unique_ptr<CMeshBase>> m_aMeshes;
	std::vector<commons_struct::STexture> m_aTexture;
	std::unique_ptr<CNodeMesh> m_pRoot;
	CAnimator m_animator;
	asset::SLoadModel m_paths;
	std::vector<SMaterial2> m_aMaterial;
};

class CModelScene
{
public:
	CModelScene() = default;
	~CModelScene() = default;
public:
	std::unique_ptr < CNodeMesh> m_pRoot;
	uint32_t m_animator{0};
};

}




