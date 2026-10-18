#include "en_select_system.h"

namespace en_vk
{

CSelectSystem::CSelectSystem()
{}
void CSelectSystem::setHighlighted(common::entityId64 id)
{
	if(m_highlighted == id)
		return;

	m_highlighted = id;
}

void CSelectSystem::setSelected(bool bAppend)
{
	if(!bAppend)
		m_aSelected.clear();

	if(m_highlighted != common::cInvalidID)
		m_aSelected.insert(m_highlighted);
}

common::entityId64 CSelectSystem::getHighlighted() const
{
	return m_highlighted;
}

std::vector<common::entityId64> CSelectSystem::getSelected() const
{
	std::vector<common::entityId64> result;
	for(auto id : m_aSelected)
		result.push_back(id);
	return  result;
}

bool CSelectSystem::isSelected(common::entityId64 id) const
{
	auto it = std::find(m_aSelected.begin(), m_aSelected.end(), id);
	return it != m_aSelected.end();
}

bool CSelectSystem::isHighlighted(common::entityId64 id) const
{
	return id == m_highlighted;
}

}