#include "GUI_Render.h"
#include <QResizeEvent>


void CVulkanWindow::updateFrame()
{
	if(!m_pVulkan || m_closing)
		return;
	if(m_resizePending)
	{
		m_pVulkan->resizeSwapchain(m_windowSize.width(), m_windowSize.height());
		m_resizePending = false;
	}

	auto tmpTime = std::chrono::steady_clock::now();
	auto timeElapst = std::chrono::duration_cast<std::chrono::milliseconds>(tmpTime - m_timePoint);
	m_timePoint = tmpTime;
	auto deltatTime = timeElapst.count() / 1000.;
	m_pVulkan->handleKeyInput(EKey::UNKNOWN, -1, -1, -1);
	m_pVulkan->update(deltatTime);
	m_pVulkan->renderSceneFrame(true, glm::uvec2(m_mouseLastX, m_mouseLastY));
	requestUpdate();
}

void CVulkanWindow::init()
{
	m_pVulkan = std::make_unique<CEngine>();

	vk_structs::SInstance::initVolkExternal();

#ifndef NDEBUG
	m_instance.setLayers({"VK_LAYER_KHRONOS_validation"});
#endif
	m_instance.setApiVersion(QVersionNumber(1, 4, 0));
	if(!m_instance.create())
		qFatal("Failed to create Vulkan instance");


	vk_structs::SInstance::initVolkLoadInstanceOnlyExternal(m_instance.vkInstance());
	setVulkanInstance(&m_instance);
	setSurfaceType(QSurface::VulkanSurface);
}

void CVulkanWindow::initAfterShow()
{
	auto surface = m_instance.surfaceForWindow(this);
	if(!surface)
		qFatal("Failed to create Vulkan surface");
	if(!m_pVulkan)
		return;
	m_pVulkan->setWindowResolution(960, 540);
	if(m_instance.vkInstance())
	{
		m_pVulkan->initialize({m_instance.vkInstance(), surface, false});
	}
	QObject::connect(&m_timer, &QTimer::timeout, this, &CVulkanWindow::updateFrame);
	m_timePoint = std::chrono::steady_clock::now();
	m_timer.start(1000/60);
}

inline EKey QtMouseToKey(int qtKey)
{
	switch(qtKey)
	{
	case Qt::LeftButton: return EKey::eMouseLeft;
	case Qt::RightButton: return EKey::eMouseRight;
	case Qt::MiddleButton: return EKey::eMouseMiddle;

	default: return EKey::UNKNOWN;
	}
}

void CVulkanWindow::wheelEvent(QWheelEvent* pEvent)
{
	QWindow::wheelEvent(pEvent);

	if(m_pVulkan)
	{
		auto delta = pEvent->angleDelta();
		m_pVulkan->handleScroll(delta.x(), delta.y());
	}
}

void CVulkanWindow::mouseMoveEvent(QMouseEvent* pEvent)
{
	QWindow::mouseMoveEvent(pEvent);
	if(m_pVulkan)
	{
		m_mouseLastX = pEvent->position().x();
		m_mouseLastY = pEvent->position().y();
		m_pVulkan->handleMouseMove(pEvent->x(), pEvent->position().y());
	}
}

void CVulkanWindow::mousePressEvent(QMouseEvent* pEvent)
{
	QWindow::mousePressEvent(pEvent);
	if(m_pVulkan)
	{
		m_pVulkan->handleKeyInput(QtMouseToKey(pEvent->button()), -1, true, -1);
	}
}

void CVulkanWindow::mouseReleaseEvent(QMouseEvent* pEvent)
{
	QWindow::mouseReleaseEvent(pEvent);
	if(m_pVulkan)
	{
		m_pVulkan->handleKeyInput(QtMouseToKey(pEvent->button()), -1, false, -1);
	}
}


inline EKey QtKeyToEKey(int qtKey)
{
	switch(qtKey)
	{
	case Qt::Key_W: return EKey::W;
	case Qt::Key_A: return EKey::A;
	case Qt::Key_S: return EKey::S;
	case Qt::Key_D: return EKey::D;
	case Qt::Key_Space: return EKey::SPACE;
	case Qt::Key_Control: return EKey::LEFT_CTRL;
	case Qt::Key_Shift: return EKey::LEFT_SHIFT;
	case Qt::Key_Escape: return EKey::ESCAPE;

	default: return EKey::UNKNOWN;
	}
}

void CVulkanWindow::keyPressEvent(QKeyEvent* pEvent)
{
	QWindow::keyPressEvent(pEvent);
	if(m_pVulkan)
		m_pVulkan->handleKeyInput(QtKeyToEKey(pEvent->key()), -1, true, -1);
}

void CVulkanWindow::keyReleaseEvent(QKeyEvent* pEvent)
{
	QWindow::keyReleaseEvent(pEvent);
	// Qt auto-repeat sends release+press pairs while a key is held - the key is not really released.
	if(pEvent->isAutoRepeat())
		return;
	if(m_pVulkan)
		m_pVulkan->handleKeyInput(QtKeyToEKey(pEvent->key()), -1, false, -1);
}

void CVulkanWindow::closeEvent(QCloseEvent*)
{
	m_closing = true;
	m_timer.stop();
	m_timer.disconnect(this);
	if(m_pVulkan)
	{
		m_pVulkan->destroy();
		m_pVulkan.reset();
	}
}

void CVulkanWindow::resizeEvent(QResizeEvent* pEvent)
{
	QWindow::resizeEvent(pEvent);

	if(m_closing)
		return;
	if(m_windowSize != pEvent->size())
	{
		m_windowSize = pEvent->size();
		m_resizePending = true;
	}
}

bool CVulkanWindow::event(QEvent* pEvent)
{
	if(pEvent->type() == QEvent::Close)
	{
		m_timer.stop();
		m_timer.disconnect(this);
		m_closing = true;
	}

	return QWindow::event(pEvent);
}