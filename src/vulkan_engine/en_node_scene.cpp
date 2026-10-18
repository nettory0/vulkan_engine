#include "en_node_scene.h"

namespace en_scene
{
static const SNodeRecord emptyRecord{.m_parent{common::cInvalidID}};

CScene::CScene()
{
	m_root = m_objectCount;
	m_aNode[m_root].m_tag = "Root";
	m_aNode[m_root].m_parent = common::cInvalidID;
	m_objectCount++;
}

common::entityId64 CScene::root() const
{
	return m_root;
}

const std::vector<common::entityId64>& CScene::getChildren(common::entityId64 nodeId) const
{
	return nodeRecord(nodeId).m_aChildren;
}

common::entityId64 CScene::getParent(common::entityId64 nodeId) const
{
	return nodeRecord(nodeId).m_parent;
}

const std::string& CScene::tag(common::entityId64 nodeId) const
{
	return nodeRecord(nodeId).m_tag;
}

bool CScene::attachToParent(common::entityId64 parentId, common::entityId64 childId)
{
	if(!isNodeExist(parentId) || !isNodeExist(childId))
		return false;

	if(hasChildren(parentId, childId) || isDescendant(parentId, childId))
		return false;

	auto& parentNode = m_aNode.at(parentId);
	auto& childNode = m_aNode.at(childId);

	if(childNode.m_parent != common::cInvalidID)
		return false;

	childNode.m_parent = parentId;
	parentNode.m_aChildren.push_back(childId);
	return true;
}

bool CScene::detachFromParent(common::entityId64 parentId, common::entityId64 childId)
{
	if(!isNodeExist(parentId) || !isNodeExist(childId))
		return false;

	if(!hasChildren(parentId, childId))
		return false;

	auto& parentNode = m_aNode.at(parentId);
	auto& childNode = m_aNode.at(childId);

	if(childNode.m_parent != parentId)
		return false;

	childNode.m_parent = common::cInvalidID;
	auto& parentChildren = parentNode.m_aChildren;
	parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), childId), parentChildren.end());
	return true;
}

bool CScene::hasChildren(common::entityId64 parentId, common::entityId64 childId)
{
	if(!isNodeExist(parentId) || !isNodeExist(childId))
		return false;

	auto& aParentChildren = m_aNode.at(parentId).m_aChildren;
	auto it = std::find(aParentChildren.begin(), aParentChildren.end(), childId);
	return it != aParentChildren.end();
}

bool CScene::isNodeExist(common::entityId64 nodeId) const
{
	return m_aNode.contains(nodeId);
}

const SNodeRecord& CScene::nodeRecord(common::entityId64 nodeId) const
{
	return m_aNode.at(nodeId);
	/*
	if(isNodeExist(nodeId))
		return m_aNode.at(nodeId);
	return emptyRecord;
	*/
}

common::entityId64 CScene::addNode(std::string tag)
{
	const common::entityId64 freeId = m_objectCount++;
	auto& newNode = m_aNode[freeId];
	newNode.m_tag = tag;
	return freeId;
}

common::entityId64 CScene::addNode(common::entityId64 parentId, std::string tag)
{
	if(!isNodeExist(parentId))
		return common::cInvalidID;

	const common::entityId64 freeId = addNode(tag);
	attachToParent(parentId, freeId);
	return freeId;
}

void CScene::removeNode(common::entityId64 id)
{
	if(!isNodeExist(id) || id == m_root)
		return;

	auto& rec = m_aNode.at(id);
	if(rec.m_parent != common::cInvalidID)
		detachFromParent(rec.m_parent, id);

	auto children = rec.m_aChildren;
	for(auto child : children)
		removeNode(child);

	m_aNode.erase(id);
}

bool CScene::isDescendant(common::entityId64 candidateParent, common::entityId64 node) const
{
	while(candidateParent != common::cInvalidID)
	{
		if(candidateParent == node)
			return true;
		candidateParent = getParent(candidateParent);
	}
	return false;
}


}