#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "en_vulkan_engine_export.h"
#include "co_types.h"

namespace en_scene
{


struct SNodeRecord
{
	common::entityId64 m_parent{common::cInvalidID};
	std::vector<common::entityId64> m_aChildren;
	int m_state{0};
	std::string m_tag;
};

class VULKAN_ENGINE CScene
{
public:
	CScene();
	~CScene(){}
	common::entityId64 addNode(common::entityId64 parentId, std::string tag);
	common::entityId64 root() const;
	void removeNode(common::entityId64 id);
	const std::vector<common::entityId64>& getChildren(common::entityId64 nodeId) const;
	common::entityId64 getParent(common::entityId64 parentId) const;
	const std::string& tag(common::entityId64 nodeId) const;
	bool attachToParent(common::entityId64 parentId, common::entityId64 childId);
	bool detachFromParent(common::entityId64 parentId, common::entityId64 childId);
	bool hasChildren(common::entityId64 parentId, common::entityId64 childId);
	bool isNodeExist(common::entityId64 nodeId) const;
private:

	const SNodeRecord& nodeRecord(common::entityId64 nodeId) const;
	common::entityId64 addNode(std::string tag);
	bool isDescendant(common::entityId64 candidateParent, common::entityId64 node) const;

private:
	common::entityId64 m_root;
	common::entityId64 m_objectCount{common::cRootID};
	std::unordered_map<common::entityId64, SNodeRecord> m_aNode;
};

}