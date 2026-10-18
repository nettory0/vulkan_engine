#include "GUI_Render.h"
#include <QtWidgets/QApplication>
#include <QPlainTextEdit>
#include <QVulkanInstance>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QPointer>
#include <QTimer>



Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")

static QPointer<QPlainTextEdit> messageLogWidget;
static QtMessageHandler oldMessageHandler = nullptr;

static void messageHandler(QtMsgType msgType, const QMessageLogContext &logContext, const QString &text)
{
    if (!messageLogWidget.isNull())
        messageLogWidget->appendPlainText(text);
    if (oldMessageHandler)
        oldMessageHandler(msgType, logContext, text);
}

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    messageLogWidget = new QPlainTextEdit(QLatin1String(QLibraryInfo::build()) + QLatin1Char('\n'));
    messageLogWidget->setReadOnly(true);

    oldMessageHandler = qInstallMessageHandler(messageHandler);

    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));



    CVulkanWindow *vulkanWindow = new CVulkanWindow;
    vulkanWindow->init();

    CMainWindow mainWindow(nullptr, vulkanWindow, messageLogWidget.data());

   // mainWindow.resize(1920, 1080);
    mainWindow.show();
    //QObject::connect(app, &QApplication::exec, vulkanWindow, &CVulkanWindow::updateFrame);
    vulkanWindow->initAfterShow();
    mainWindow.loadTree();
    
    return app.exec();
}
