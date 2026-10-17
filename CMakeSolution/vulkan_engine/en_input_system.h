#pragma once
#include <array>
#include "en_vulkan_engine_export.h"
#include <glm/glm.hpp>

enum class EKey: int
{
    W,
    A,
    S,
    D,
    SPACE,
    LEFT_CTRL,
    ESCAPE,
    eMouseRight,
    eMouseLeft,
    eMouseMiddle,
    UNKNOWN
};


class VULKAN_ENGINE CInputSystem
{
public:
	void keyCallback(EKey key, int scancode, int action, int mods);
	bool isKeyPressed(EKey key) const;
	void reset();
    glm::vec3 mouseMove(double x, double y, glm::vec2& deltaMove);

private:
	std::array<bool, 1024> m_keys{};
    double m_mouseX{0};
    double m_mouseY{0};
    bool m_bFirstMouse{true};
};