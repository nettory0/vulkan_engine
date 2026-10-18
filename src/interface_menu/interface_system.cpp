#include "interface_system.h"
#include "interface_menu.h"
#include <RmlUi/Core/Context.h>

namespace interface_menu
{
CInterfaceSystem::CInterfaceSystem(): m_interfaceManager{std::make_unique<CInterfaceManager>()}
{

}

void CInterfaceSystem::init()
{
	m_interfaceManager->init();
}

void CInterfaceSystem::update()
{
	m_interfaceManager->update();
}
void CInterfaceSystem::render()
{
	m_interfaceManager->render();
}

CInterfaceSystem::~CInterfaceSystem()
{}


const std::vector<SUiDrawItem>& CInterfaceSystem::getDrawQueue() const
{
	return m_interfaceManager->getDrawQueue();
}

const std::set<commons_struct::STexture*>& CInterfaceSystem::avalibelTextures() const
{
	return m_interfaceManager->avalibelTextures();
}

const std::set<commons_struct::SIndexedVx*>& CInterfaceSystem::avalibelGeometry() const
{
	return m_interfaceManager->avalibelGeometry();
}


}
