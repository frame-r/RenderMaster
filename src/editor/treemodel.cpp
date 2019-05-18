#include "treeNode.h"
#include "treeModel.h"
#include "editorcore.h"
#include "gameobject.h"

#include <QCoreApplication>
#include <QStringList>
#include <QMimeData>
#include <QIODevice>
#include <QDataStream>
#include <QIcon>

#define MESH_ICON ":/icons/cube_g.png"
#define CAMERA_ICON ":/icons/camera.png"
#define LIGHT_ICON ":/icons/light.png"

static const char s_treeNodeMimeType[] = "application/x-treenode";


TreeModel::TreeModel(const QStringList &modalities, QObject *parent)
	: QAbstractItemModel(parent)
{

	QStringList data = modalities;

//    for (int i = 1 ; i < modalities.size() ; ++i)
//        data.push_back(QString::fromStdString(modalities.at(i)));


//	setupModelData(data, editor->RootNode());

//	connect(editor, &EditorCore::OnObjectAdded, this, &TreeModel::onObjectAdded);
//	connect(editor, &EditorCore::OnObjectRemoved, this, &TreeModel::onObjectRemoved);
}

TreeModel::~TreeModel()
{
}

//returns the mime type
QStringList TreeModel::mimeTypes() const
{
	return QStringList() << s_treeNodeMimeType;
}

//receives a list of model indexes list
QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData;
	QByteArray data; //a kind of RAW format for datas

	//QDataStream is independant on the OS or proc architecture
	//serialization of C++'s basic data types, like char, short, int, char *, etc.
	//Serialization of more complex data is accomplished
	//by breaking up the data into primitive units.
	QDataStream stream(&data, QIODevice::WriteOnly);
	QList<TreeNode *> nodes;

	//
	foreach (const QModelIndex &index, indexes) {
		TreeNode *node = nodeForIndex(index);
		if (!nodes.contains(node))
			nodes << node;
	}
	stream << QCoreApplication::applicationPid();
	stream << nodes.count();
	foreach (TreeNode *node, nodes) {
		stream << reinterpret_cast<qlonglong>(node);
	}
	mimeData->setData(s_treeNodeMimeType, data);
	return mimeData;
}

bool TreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_ASSERT(action == Qt::MoveAction);
	Q_UNUSED(column);
	//test if the data type is the good one
	if (!mimeData->hasFormat(s_treeNodeMimeType)) {
		return false;
	}
	QByteArray data = mimeData->data(s_treeNodeMimeType);
	QDataStream stream(&data, QIODevice::ReadOnly);
	qint64 senderPid;
	stream >> senderPid;
	if (senderPid != QCoreApplication::applicationPid()) {
		// Let's not cast pointers that come from another process...
		return false;
	}

	TreeNode *parentNode = nodeForIndex(parent);	
	Q_ASSERT(parentNode);

	GameObject *parentObj = parentNode->obj();

	int count;
	stream >> count;

	if (row == -1) {
		// valid index means: drop onto item. I chose that this should insert
		// a child item, because this is the only way to create the first child of an item...
		// This explains why Qt calls it parent: unless you just support replacing, this
		// is really the future parent of the dropped items.
		if (parent.isValid())
			row = 0;
		else
			// invalid index means: append at bottom, after last toplevel
			row = rowCount(parent);
	}
	for (int i = 0; i < count; ++i) {
		// Decode data from the QMimeData
		qlonglong nodePtr;
		stream >> nodePtr;
		TreeNode *node = reinterpret_cast<TreeNode *>(nodePtr);

		// Adjust destination row for the case of moving an item
		// within the same parent, to a position further down.
		// Its own removal will reduce the final row number by one.
		if (node->row() < row && parentNode == node->parentNode())
			--row;

		GameObject *obj = node->obj();

		// Remove from old position
		{
			bool root = obj->GetParent() == nullptr;

			// dbg
			//obj->print_local();

			if (root)
				editor->RemoveRootGameObject(obj);
			else
				obj->GetParent()->RemoveChild(obj);
			removeNode(node);
		}

		// Insert at new position
		//qDebug() << "Inserting into" << parent << row;
		beginInsertRows(parent, row, row);
		{
			if (parentObj == nullptr)
				editor->InsertRootGameObject(row, obj);
			else
				parentObj->InsertChild(obj, row);

			// dbg
			//obj->print_local();

			parentNode->insertChild(row, node);
		}
		endInsertRows();
		++row;
	}
	return true;
}
static int counter;
bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	assert(false); // should never call
	//TreeNode *parentItem = nodeForIndex(parent);
	//
	//beginInsertRows(parent, position, position + rows - 1);
	//for(int i = position; i< position + rows; ++i)
	//{
	//	auto data = QVariant(QString("new %1").arg(counter++));
	//	//TreeNode *newNode = new TreeNode(data, parentItem);
	//	TreeNode *newNode = parentItem->child(i);
	//	parentItem->insertChild(position, newNode);
	//}
	//endInsertRows();

	return true;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
	TreeNode *parentItem = nodeForIndex(parent);

	beginRemoveRows(parent, position, position + rows - 1);
	for(int i = position; i<position + rows; ++i)
	{
		TreeNode *n = parentItem->child(position);
		parentItem->removeChild(position);
	}
	endRemoveRows();

	return true;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode *node = nodeForIndex(index);

	if (role == Qt::DisplayRole)
	{
		return node->data();
	}
	else if (role == Qt::DecorationRole)
	{
		GameObject *obj = node->obj();
		if (obj->GetType() == OBJECT_TYPE::CAMERA)
			return QIcon(CAMERA_ICON);
		else if (obj->GetType() == OBJECT_TYPE::LIGHT)
			return QIcon(LIGHT_ICON);

		return QIcon(MESH_ICON);
	}
	return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant TreeModel::headerData(int , Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return editor->RootNode()->data();

	return QVariant();
}

TreeNode * TreeModel::nodeForIndex(const QModelIndex &index) const
{
	if (!index.isValid())
		return editor->RootNode();
	else
		return static_cast<TreeNode*>(index.internalPointer());
}

QModelIndex TreeModel::indexForNode(TreeNode *node)
{
	return createIndex(node->row(), 0, node);
}

void TreeModel::removeNode(TreeNode *node)
{
	const int row = node->row();
	QModelIndex idx = createIndex(row, 0, node);
	beginRemoveRows(idx.parent(), row, row);
	node->parentNode()->removeChild(row);
	endRemoveRows();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{	
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	TreeNode *parentNode = nodeForIndex(parent);
	TreeNode *childNode = parentNode->child(row);

	if (childNode)
		return createIndex(row, column, childNode);
	else
		return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	TreeNode *childNode = nodeForIndex(index);

	if (!index.isValid())
		return QModelIndex();

	if (childNode == editor->RootNode())
		return QModelIndex();

	TreeNode *parentNode = childNode->parentNode();
	if (parentNode == editor->RootNode())
		return QModelIndex();

	return createIndex(parentNode->row(), 0, parentNode);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	TreeNode *parentNode = nodeForIndex(parent);
	return parentNode->childCount();
}

//void TreeModel::setupModelData(const QStringList &lines, TreeNode *parent)
//{
//	QList<TreeNode*> parents;
//	QList<int> indentations;
//	parents << parent;
//	indentations << 0;

//	int number = 0;

//	while (number < lines.count()) {
//		int position = 0;
//		while (position < lines[number].length()) {
//			if (lines[number].mid(position, 1) != " ")
//				break;
//			position++;
//		}

//		QString lineData = lines[number].mid(position).trimmed();

//		if (!lineData.isEmpty()) {
//			// Read the column data from the rest of the line.
//			QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
//			QList<QVariant> columnData;
//			for (int column = 0; column < columnStrings.count(); ++column)
//				columnData << columnStrings[column];

//			if (position > indentations.last()) {
//				// The last child of the current parent is now the new parent
//				// unless the current parent has no children.

//				if (parents.last()->childCount() > 0) {
//					parents << parents.last()->child(parents.last()->childCount()-1);
//					indentations << position;
//				}
//			} else {
//				while (position < indentations.last() && parents.count() > 0) {
//					parents.pop_back();
//					indentations.pop_back();
//				}
//			}

//			// Append a new node to the current parent's list of children.
//			parents.last()->appendChild(new TreeNode(columnData[0], parents.last()));
//		}

//		++number;
//	}
//}

void TreeModel::onObjectAdded(TreeNode *node)
{
	//const int row = node->row();
	//QModelIndex idx = createIndex(row, 0, node);
	//insertRow(row, idx);
	emit layoutChanged();
}

void TreeModel::onObjectRemoved(TreeNode *n)
{
	if (n->parentNode() == editor->RootNode())
		removeRow(n->row(), QModelIndex());
	else
		removeRow(n->row(), indexForNode(n->parentNode()));
}
