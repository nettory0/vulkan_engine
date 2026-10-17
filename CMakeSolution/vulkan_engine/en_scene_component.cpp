#include "en_scene_component.h"
#include "en_gl_descriptor_manager.h"
#include <ranges>
#include "co_structs.h"
#include "co_geom_primitives.h"

struct STagAnimated: public STag {};
struct STagStatic: public STag {};
struct STagUI: public STag {};
struct STag2d: public STag {};
struct STagCompute: public STag {};

static commons_struct::SIndexedVx genIndexed(const std::vector<commons_struct::SVertex>& vertexIn)
{
	commons_struct::SIndexedVx result;
	std::unordered_map<commons_struct::SVertex, uint32_t> aVertex;
	uint32_t counter{0};
	result.m_aIndices.reserve(vertexIn.size());
	result.m_aVx.reserve(vertexIn.size());
	for(const auto& vertex : vertexIn)
	{
		auto [pair, bRes] = aVertex.try_emplace(vertex, counter);
		if(bRes)
		{
			result.m_aVx.push_back(vertex);
			counter++;
		}
		result.m_aIndices.push_back(pair->second);
	}
	return result;
}

SMeshId CSceneResources::addStaticMesh(commons_struct::SIndexedVx vxIndexed, unsigned int material)
{
	en_model::CMeshStatic mesh{vxIndexed.m_aVx, vxIndexed.m_aIndices, 0};
	return addStaticMesh(std::move(mesh));
}

SMeshId CSceneResources::addStaticMesh(en_model::CMeshStatic&& mesh)
{
	SMeshId result;
	result.m_id = m_aMesh.add(std::make_unique<en_model::CMeshStatic>(std::move(mesh)));
	return result;
}

SMeshId CSceneResources::addAnimatedMesh(en_model::CMeshAnimated&& mesh)
{
	SMeshId result;
	result.m_id = m_aMesh.add(std::make_unique<en_model::CMeshAnimated>(std::move(mesh)));
	return result;
}
void CSceneComponentSystem::addCompute()
{
	m_compute = m_scene.addNode(m_scene.root(), "compute");
	componentManager().addComponent<STagCompute>(m_compute) = {};
}

common::entityId64 CSceneComponentSystem::addNode(common::entityId64 parentId, const std::string& tag)
{
	return m_scene.addNode(parentId, tag);
}

std::vector<common::entityId64> CSceneComponentSystem::getAnimatedMeshes()
{
	std::vector<common::entityId64> result;
	auto mapAnimated = componentManager().getEntitiesWithData<STagAnimated>();
	for(auto& [id, type] : mapAnimated)
		result.push_back(id);
	return result;
}

std::vector<common::entityId64> CSceneComponentSystem::getStaticMeshes()
{
	std::vector<common::entityId64> result;
	auto mapStatic = componentManager().getEntitiesWithData<STagStatic>();
	for(auto& [id, type] : mapStatic)
		result.push_back(id);
	return  result;
}

std::vector<common::entityId64> CSceneComponentSystem::get2dMeshes()
{
	std::vector<common::entityId64> result;
	auto map2d = componentManager().getEntitiesWithData<STag2d>();
	for(auto& [id, type] : map2d)
		result.push_back(id);
	return  result;
}

std::vector<common::entityId64> CSceneComponentSystem::getUIMeshes()
{
	std::vector<common::entityId64> result;
	auto mapUI = componentManager().getEntitiesWithData<STagUI>();
	for(auto& [id, type] : mapUI)
		result.push_back(id);
	return  result;
}

std::vector<common::entityId64> CSceneComponentSystem::getCompute()
{
	std::vector<common::entityId64> result;
	auto aCompute = componentManager().getEntitiesWithData<STagCompute>();
	for(auto& [id, type] : aCompute)
		result.push_back(id);
	return  result;
}

bool CSceneComponentSystem::isMeshStatic(common::entityId64 id)
{
	return componentManager().getComponentConst<STagStatic>(id) != nullptr;
}

bool CSceneComponentSystem::isMeshAnimated(common::entityId64 id)
{
	return componentManager().getComponentConst<STagAnimated>(id) != nullptr;
}

bool CSceneComponentSystem::isMesh2d(common::entityId64 id)
{
	return componentManager().getComponentConst<STag2d>(id) !=nullptr;
}

common::entityId64 CSceneComponentSystem::addTransformNode(common::entityId64 parent, const glm::mat4& transform, const std::string& tag)
{
	auto pTransformNode = m_scene.addNode(parent, tag);
	m_componentManager.addComponent<glm::mat4>(pTransformNode) = transform;
	return pTransformNode;
}

void CSceneComponentSystem::addCube(common::entityId64 pNodeParent, glm::vec3 pos, float mass, glm::vec3 boxSize)
{
	glm::mat4 mat = glm::mat4{1};
	const glm::mat4 matCube = glm::scale(glm::mat4{1}, boxSize);

	auto pTransformNode = addTransformNode(pNodeParent, mat, "Box");

	auto pNode = m_scene.addNode(pTransformNode, "");


	auto vx = genIndexed(commons_geom_primitives::cube());
	auto pTest = m_resources.addStaticMesh(vx, 0);
	m_componentManager.addComponent<glm::mat4>(pNode) = matCube;

	componentManager().addComponent<SMeshId>(pNode) = pTest;
	componentManager().addComponent<SPhysicsObject>(pNode) = {EPhysicsObjectType::eBox,pos, boxSize, mass};
	componentManager().addComponent<STagStatic>(pNode) = {};
}

void CSceneComponentSystem::addWord(common::entityId64 pNodeParent, commons_struct::SIndexedVx vxIndexed, const std::string& tag)
{
	auto pNode = addNode(pNodeParent, tag);
	auto pMesh = m_resources.addStaticMesh(vxIndexed, 0);

	componentManager().addComponent<SMeshId>(pNode) = pMesh;
	componentManager().addComponent<STag2d>(pNode) = {};
}

void CSceneComponentSystem::addSphere(common::entityId64 pNodeParent, glm::vec3 origin, float mass, float radius)
{
	glm::mat4 mat = glm::mat4(1);
	mat = glm::scale(mat, {1.0, 1.0, 1.0});

	auto pTransformNode = addTransformNode(pNodeParent, mat, "Sphere");

	auto pNode = m_scene.addNode(pTransformNode, "");
	//	const float radius{1.5f};
	commons_geom_primitives::CSphere sphere{radius, 36, 18, true, 3};
	auto pTest = m_resources.addStaticMesh(commons_geom_primitives::sphere(radius, 36, 18, true, 3), 0);

	componentManager().addComponent<SMeshId>(pNode) = pTest;
	componentManager().addComponent<SPhysicsObject>(pNode) = {EPhysicsObjectType::eSphere,origin,{}, mass, radius};
	componentManager().addComponent<STagStatic>(pNode) = {};
}

void CSceneComponentSystem::traverseNodeMeshes(common::entityId64 id, const en_model::CNodeMesh& node, const std::map<int, SMeshId>& mapMesh, const std::map<int, SMaterialId>& mapMaterial, const std::map<int, STextureId>& mapTexture, bool bLevel, bool bAnimated)
{
	if(node.m_meshId != -1)
	{
		auto meshIt = mapMesh.find(node.m_meshId);
		if(meshIt == mapMesh.end())
			return;

		const auto& meshId = meshIt->second;
		if(auto* pMesh = sceneResources().getMeshBase(meshId))
		{
			SMaterialId idMat;
			STextureId idTex;
			if(auto it = mapMaterial.find(pMesh->m_materialIndex); it != mapMaterial.end())
			{
				idMat = it->second;
				if(auto* pMaterial = m_resources.getMaterial(idMat))
				{
					auto textureIndex = pMaterial->textureIndex;
					if(auto it = mapTexture.find(textureIndex); it != mapTexture.end())
					{
						idTex = it->second;
					}
				}
			}

			componentManager().addComponent<SMeshId>(id) = meshId;
			componentManager().addComponent<SPhysicsObject>(id) = {bLevel ? EPhysicsObjectType::eLevelMesh : EPhysicsObjectType::eNone};
			if(bAnimated)
				componentManager().addComponent<STagAnimated>(id) = {};
			else
				componentManager().addComponent<STagStatic>(id) = {};
			componentManager().addComponent<STextureId>(id) = idTex;
		}
	}

	for(auto& child : node.m_aChild)
	{
		auto childId = m_scene.addNode(id, {"Child"});
		traverseNodeMeshes(childId, *child.get(), mapMesh, mapMaterial, mapTexture, bLevel, bAnimated);
	}
}

const commons_struct::STexture* CSceneResources::getTexture(const STextureId& textureId) const
{
	return  m_aTexture.get(textureId.m_id);
}

const en_model::CMeshBase* CSceneResources::getMeshBase(const SMeshId& meshId) const
{
	return m_aMesh.get(meshId.m_id);
}

const en_model::SMaterial2* CSceneResources::getMaterial(const SMaterialId& materialId) const
{
	return m_aMaterial.get(materialId.m_id);
}

glm::mat4 CSceneComponentSystem::calculateTransformation(uint64_t nodeId) const
{
	std::vector<glm::mat4> aMat4;
	auto pNode = nodeId;
	while(pNode != common::cInvalidID)
	{
		if(auto* pMat = m_componentManager.getComponentConst<glm::mat4>(pNode))
		{
			aMat4.push_back(*pMat);
		}
		pNode = m_scene.getParent(pNode);
	}

	glm::mat4 res{1};
	for(auto& mat : std::views::reverse(aMat4))
	{
		res = res * mat;
	}
	return res;
}

SMaterialId CSceneResources::addMaterial(en_model::SMaterial2&& material)
{
	SMaterialId result;
	result.m_id = m_aMaterial.add(std::move(material));//  m_materialCounter;
	return result;
}
STextureId CSceneResources::addTexture(commons_struct::STexture&& texture)
{
	STextureId result;
	result.m_id = m_aTexture.add(std::move(texture));
	return result;
}