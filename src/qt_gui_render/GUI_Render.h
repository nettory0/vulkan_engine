
#ifndef HELLOVULKANWIDGET_H
#define HELLOVULKANWIDGET_H

#include "ui_GUI_Render.h"
#include <QWidget>
#include <QWindow>
#include <QVulkanInstance>
#include "en_core.h"
#include "en_camera.h"
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QTabWidget)
QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(QLCDNumber)

class CVulkanWindow;

class CMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit CMainWindow(QWidget* parent, CVulkanWindow* w, QPlainTextEdit* logWidget);
    void loadTree();
public slots:
    void onVulkanInfoReceived(const QString& text);
    void onFrameQueued(int colorValue);
    void onGrabRequested();
    void onSimulationButtonClicked(bool bWork);
private:
    CVulkanWindow* m_window;

    Ui::GUI_RenderClass ui;
    bool m_bSimulationClicked{false};
};

class CVulkanWindow: public QWindow
{
    Q_OBJECT

public:
    void init();
    void initAfterShow();
    
    QVulkanInstance m_instance;
    std::unique_ptr<CEngine> m_pVulkan;
    QTimer m_timer;
    std::chrono::steady_clock::time_point m_timePoint;
private:
    void resizeEvent(QResizeEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void closeEvent(QCloseEvent*) override;
    bool event(QEvent* pEvent) override;
signals:
    void vulkanInfoReceived(const QString& text);
    void frameQueued(int colorValue);
public slots:
    void updateFrame();

private:
    QSize m_windowSize;
    bool m_closing = false;
    bool m_resizePending = false;
    double m_mouseLastX;
    double m_mouseLastY;
};

#endif // HELLOVULKANWIDGET_H
