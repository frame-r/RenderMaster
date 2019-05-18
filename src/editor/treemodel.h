#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeNode;

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit TreeModel(const QStringList &modalities, QObject *parent = nullptr);
	~TreeModel() Q_DECL_OVERRIDE;

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE { Q_UNUSED(parent) return 1; }
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE {return Qt::MoveAction;}
	Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE {return Qt::MoveAction;}
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;
	bool insertRows(int position, int rows,	const QModelIndex &parent = QModelIndex()) override;
	bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

	TreeNode * nodeForIndex(const QModelIndex &index) const;
	QModelIndex indexForNode(TreeNode *node);
private:

	void removeNode(TreeNode *node);
	//void setupModelData(const QStringList &lines, TreeNode *parent);

public:
	void onObjectAdded(TreeNode *obj);
	void onObjectRemoved(TreeNode *obj);
};

#endif // TREEMODEL_H
