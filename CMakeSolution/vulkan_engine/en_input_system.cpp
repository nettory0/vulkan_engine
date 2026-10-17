#include "en_input_system.h"

constexpr int press = 1;
constexpr int release = 0;

void CInputSystem::keyCallback(EKey key, int scancode, int action, int mods)
{
	if((int)key < 0 || (int)key >= 1024)
		return;

	if(action == press)
		m_keys[(int)key] = true;
	else if(action == release)
		m_keys[(int)key] = false;
}

bool CInputSystem::isKeyPressed(EKey key) const
{
	return (int)key >= 0 && (int)key < 1024 && m_keys[(int)key];
}

void CInputSystem::reset()
{
	m_keys.fill(false);
}

glm::vec3 CInputSystem::mouseMove(double x, double y, glm::vec2& deltaMove)
{
	if(m_bFirstMouse)
	{
		m_mouseX = x;
		m_mouseY = y;
		m_bFirstMouse = false;
		deltaMove = {0,0};
		return {};
	}

	glm::vec3 orientation{0,0,0};
	if(m_mouseX != x || m_mouseY != y)
	{
		deltaMove = {x - m_mouseX, y - m_mouseY};
		float sensitivity = 0.10;
		orientation.x += (x - m_mouseX) * sensitivity;
		orientation.y += (m_mouseY - y) * sensitivity;
		m_mouseX = x;
		m_mouseY = y;
	}
	return orientation;
}