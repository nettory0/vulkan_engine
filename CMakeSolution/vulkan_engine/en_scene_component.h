#pragma once
#include "en_component_system.h"
#include "en_node_scene.h"
#include "en_animation_structs.h"
#include "en_common_utils.h"

struct STag {};

struct SMaterialId
{
	uint32_t m_id{0};
};

struct STextureId
{
	uint32_t m_id{0};
};

struct SMeshId
{
	uint32_t m_id{0};
};

enum class EPhysicsObjectType
{
	eBox, eSphere, eLevelMesh, eNone
};
struct SPhysicsObject
{
	EPhysicsObjectType m_type{EPhysicsObjectType::eNone};
	glm::vec3 origin{0};
	glm::vec3 boxSize{0};
	float mass{0};
	float radius{0};
};

class VULKAN_ENGINE CSceneResources
{
public:
	CSceneResources() = default;
	CSceneResources(CSceneResources&&) = default;
	CSceneResources& operator=(CSceneResources&&) = default;
	CSceneResources(const CSceneResources&) = delete;
	CSceneResources& operator=(const CSceneResources&) = delete;
	SMaterialId addMaterial(en_model::SMaterial2&& material);
	STextureId addTexture(commons_struct::STexture&& texture);
	const en_model::CMeshBase* getMeshBase(const SMeshId& meshId) const;
	const commons_struct::STexture* getTexture(const STextureId& textureId) const;
	const en_model::SMaterial2* getMaterial(const SMaterialId& materialId) const;
	SMeshId addStaticMesh(en_model::CMeshStatic&& mesh);
	SMeshId addAnimatedMesh(en_model::CMeshAnimated&& mesh);
	SMeshId addStaticMesh(commons_struct::SIndexedVx vxIndexed, unsigned int material);

private:
	en_util::CResourceKeeper<en_model::SMaterial2> m_aMaterial;
	en_util::CResourceKeeper<commons_struct::STexture> m_aTexture;
	en_util::CResourceKeeper<en_model::CMeshBase> m_aMesh;
};

class VULKAN_ENGINE CSceneComponentSystem
{
public:
	void addCompute();

	uint64_t addTransformNode(common::entityId64 parentId, const glm::mat4& transform, const std::string& tag);
	void addCube(common::entityId64 pNodeParent, glm::vec3 pos, float mass, glm::vec3 boxSize);
	void addSphere(common::entityId64 pNodeParent, glm::vec3 origin, float mass, float radius);
	void addWord(common::entityId64 pNodeParent, commons_struct::SIndexedVx vxIndexed, const std::string& tag);
	void traverseNodeMeshes(common::entityId64 id, const en_model::CNodeMesh& node, const std::map<int, SMeshId>& mapMesh, const std::map<int, SMaterialId>& mapMaterial, const std::map<int, STextureId>& mapTexture, bool bLevel, bool bAnimated);
	glm::mat4 calculateTransformation(common::entityId64 nodeId) const;
	template<typename T>
	const T* getParentNode(uint64_t node)
	{

		for(auto pParent = node; pParent != common::cInvalidID; pParent = m_scene.getParent(pParent))
		{
			if(m_componentManager.hasComponent<T>(pParent))
			{
				return m_componentManager.getComponentConst<T>(pParent);
			}
		}

		return nullptr;
	}
	en_ecs::CComponentManager& componentManager()
	{
		return m_componentManager;
	};
	CSceneResources& sceneResources()
	{
		return m_resources;
	}
	uint64_t addNode(common::entityId64 parentId, const std::string& tag);
	std::vector<common::entityId64> getAnimatedMeshes();
	std::vector<common::entityId64> getStaticMeshes();
	std::vector<common::entityId64> get2dMeshes();
	std::vector<common::entityId64> getUIMeshes();
	std::vector<common::entityId64> getCompute();
	bool isMeshStatic(common::entityId64 id);
	bool isMeshAnimated(common::entityId64 id);
	bool isMesh2d(common::entityId64 id);
public:
	en_scene::CScene m_scene;
	common::entityId64 m_player{common::cInvalidID};
	common::entityId64 m_compute{common::cInvalidID};

private:
	en_ecs::CComponentManager m_componentManager;

	CSceneResources m_resources;
};