#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTreeView>

class TreeNode;
class TreeModel;
class GameObject;


namespace Ui {
class SceneWidget;
}

class SceneWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SceneWidget(QWidget *parent = nullptr);
	~SceneWidget();

	void RemoveSelectedObjects();

private slots:
	void on_remove_clicked();
	void on_add_clicked();

	void selectionChangedFromClick(const QItemSelection &selected, const QItemSelection &deselected);

	void onObjectAdded(TreeNode *obj);
	void onObjectRemoved(TreeNode *obj);
	void OnSelectionChanged(QSet<GameObject*>& objects);

private:
	Ui::SceneWidget *ui;
	QTreeView *view;
	TreeModel *model;
	QItemSelectionModel *selectionModel;

};

#endif // SCENEWIDGET_H
