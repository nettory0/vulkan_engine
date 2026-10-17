#include "GUI_Render.h"
#include <QStandardItemModel>
#include "en_node_scene.h"

CMainWindow::CMainWindow(QWidget* parent, CVulkanWindow* w, QPlainTextEdit* logWidget): QMainWindow(parent), m_window(w)
{

	ui.setupUi(this);
	QWidget* wrapper = QWidget::createWindowContainer(w);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(wrapper, 5);
	ui.widget1->setLayout(layout);

	QObject::connect(w, &CVulkanWindow::vulkanInfoReceived, this, &CMainWindow::onVulkanInfoReceived);
	QObject::connect(w, &CVulkanWindow::frameQueued, this, &CMainWindow::onFrameQueued);
	QObject::connect(ui.pushButton, &QPushButton::clicked, this, &CMainWindow::onSimulationButtonClicked);

}

void CMainWindow::onVulkanInfoReceived(const QString& text)
{

}

void CMainWindow::onSimulationButtonClicked(bool bWork)
{
	if(m_window)
	{
		m_window->m_pVulkan->setSimulationWork(m_bSimulationClicked);
		m_bSimulationClicked = !m_bSimulationClicked;
	}
}

void CMainWindow::onFrameQueued(int colorValue)
{

}

void CMainWindow::onGrabRequested()
{}

void traversceScene(QStandardItem* parent, uint64_t pNode, const en_scene::CScene& scene)
{
	auto& aChildren = scene.getChildren(pNode);

	for(auto child : aChildren)
	{
		QStandardItem* pChildItem = new QStandardItem(scene.tag(child).c_str());
		traversceScene(pChildItem, child, scene);
		parent->appendRow(pChildItem);
	}
}

void CMainWindow::loadTree()
{

	const auto& scene = m_window->m_pVulkan->getScene();
	QStandardItemModel* model = new QStandardItemModel(this);
	model->setHorizontalHeaderLabels({"Header", "Value"});

	QStandardItem* rootItem = model->invisibleRootItem();

	traversceScene(rootItem, scene.root(), scene);

	ui.treeView->setModel(model);
	ui.treeView->expandAll();


}
