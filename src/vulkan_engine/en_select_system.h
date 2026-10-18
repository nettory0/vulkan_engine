#pragma once
#include <cstdint>
#include <unordered_set>
#include "en_scene_component.h"

namespace en_vk
{
class CSelectSystem
{
public:
	CSelectSystem();
	void setHighlighted(common::entityId64 meshId);
	void setSelected(bool bAppend);
	common::entityId64 getHighlighted() const;
	std::vector<common::entityId64> getSelected() const;
	bool isSelected(common::entityId64 meshId) const;
	bool isHighlighted(common::entityId64 meshId) const;
private:
	common::entityId64 m_highlighted{common::cInvalidID};
	std::unordered_set<common::entityId64> m_aSelected;
};
}
