#include "scenewidget.h"
#include "ui_scenewidget.h"
#include "editorcore.h"
#include "treemodel.h"
#include "gameobject.h"
#include "treenode.h"
#include <QDebug>

SceneWidget::SceneWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SceneWidget)
{
	ui->setupUi(this);

	view = ui->view;

	connect(view, &QTreeView::pressed, view, &QTreeView::expandAll);
	connect(view, &QTreeView::pressed, view, &QTreeView::expandAll);

	view->setDragEnabled(true);
	view->setAcceptDrops(true);
	view->resizeColumnToContents(0);
	view->resize(400, 500);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->expandAll();
	view->header()->setVisible(false);
	view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	view->setIndentation(20);
	view->setEditTriggers(QAbstractItemView::EditTriggers());
	view->setSelectionBehavior (QAbstractItemView::SelectRows );
	view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	//dbg
	//QStringList s = {"aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg", "hhh"};
	QStringList s;

	model = new TreeModel(s, this);
	view->setModel(model);

	selectionModel = view->selectionModel();

	connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChangedFromClick(const QItemSelection&,const QItemSelection&)));
	connect(editor, &EditorCore::OnSelectionChanged, this, &SceneWidget::OnSelectionChanged, Qt::DirectConnection);
	connect(editor, &EditorCore::OnObjectAdded, this, &SceneWidget::onObjectAdded);
	connect(editor, &EditorCore::OnObjectRemoved, this, &SceneWidget::onObjectRemoved);

}

SceneWidget::~SceneWidget()
{
	delete ui;
}

void SceneWidget::RemoveSelectedObjects()
{
	on_remove_clicked();
}

void SceneWidget::on_remove_clicked()
{
	auto idxList = selectionModel->selectedIndexes();

	QHash<TreeNode*, bool> nodesHash; // node -> need delete
	nodesHash.reserve(idxList.count());

	for (auto &idx : idxList)
	{
		TreeNode *n = model->nodeForIndex(idx);
		nodesHash[n] = true;
	}

	// mark nodes that don't need delete
	for (auto i = nodesHash.begin(); i != nodesHash.end(); ++i)
	{
		bool needDelete = true;
		TreeNode *next = i.key();

		while (next != editor->RootNode()) // go up hierarchy
		{
			if (nodesHash.find(next->parentNode()) != nodesHash.end())
			{
				needDelete = false;
				break;
			}
			next = next->parentNode();
		}

		*i = needDelete;
	}

	for (auto i = nodesHash.begin(); i != nodesHash.end(); ++i)
	{
		bool needDelete = i.value();

		if (!needDelete)
			continue;

		TreeNode *n = i.key();
		//qDebug() << n->data(0) << needDelete;

		editor->DestroyGameObject(n->obj());
	}

}

void SceneWidget::on_add_clicked()
{
	editor->CreateGameObject();
}

// Selection clllback
//
void SceneWidget::selectionChangedFromClick(const QItemSelection&, const QItemSelection&)
{
	auto idxList = selectionModel->selectedIndexes();

	if (idxList.size())
	{
		//QString dbg;
		//for (auto &idx : idxList)
		//{
		//	TreeNode *n = model->nodeForIndex(idx);
		//	dbg += n->obj()->GetName() + QString(" ");
		//}
		//qDebug() << "SceneWidget::selectionChangedFromClick(): selected:" << dbg;
		QSet<GameObject*> slected;
		for (auto &idx : idxList)
		{
			TreeNode *n = model->nodeForIndex(idx);
			slected.insert(n->obj());
		}
		editor->SelectObjects(slected);
	}
	else
	{
		//editor->ChangeSelection(vector<GameObjectPtr>());
		//auto *selectionModel = ui->treeView->selectionModel();
		//qDebug() << "SceneWidget::selectionChangedFromClick(): invalid";
		editor->SelectObjects(QSet<GameObject*>());
		selectionModel->clear();
	}
}

void SceneWidget::onObjectAdded(TreeNode *obj)
{
	model->onObjectAdded(obj);

	//QSet<GameObject*> slected = {obj->obj()};
	//editor->SelectObjects(slected);
	//
	//QModelIndex idx = model->indexForNode(obj);
	//selectionModel->select(idx, QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

void SceneWidget::onObjectRemoved(TreeNode *obj)
{
	model->onObjectRemoved(obj);
}

void SceneWidget::OnSelectionChanged(QSet<GameObject *> &objects)
{
	if (objects.size())
	{
		TreeNode *n = editor->TreNodeForObject(*objects.begin());
		auto idx = model->indexForNode(n);

		if (idx.isValid())
		{
			selectionModel->reset();
			selectionModel->select(idx, QItemSelectionModel::SelectionFlag::Select);
			view->repaint();
		}
		else
			qDebug() << "Can not find object";
	} else
		selectionModel->clear();

}

