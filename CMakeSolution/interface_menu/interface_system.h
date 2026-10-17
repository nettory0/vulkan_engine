#pragma once
#include <memory>
#include <vector>
#include <set>
#include "interface_menu_export.h"

namespace commons_struct
{
struct STexture;
struct SIndexedVx;
}

namespace interface_menu
{
struct SUiDrawItem;
class CInterfaceManager;
class INTERFACE_MENU CInterfaceSystem
{
public:
	CInterfaceSystem();
	void init();
	void update();
	void render();
	~CInterfaceSystem();
	const std::vector<SUiDrawItem>& getDrawQueue() const;
	const std::set<commons_struct::STexture*>& avalibelTextures() const;
	const std::set<commons_struct::SIndexedVx*>& avalibelGeometry() const;

private:
	std::unique_ptr<CInterfaceManager> m_interfaceManager;
};
}