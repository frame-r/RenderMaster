#ifndef TREENODE_H
#define TREENODE_H

#include <QList>
#include <QVariant>

class GameObject;

class TreeNode
{
public:
	explicit TreeNode(GameObject* obj, TreeNode *parentNode);
	~TreeNode();

	void appendChild(TreeNode *child);
	void removeChild(int row);
	TreeNode *child(int row) const;
	int childCount() const;
	QVariant data() const;
	int row() const;
	TreeNode *parentNode() const;
	void insertChild(int pos, TreeNode *child);

	GameObject *obj() { return obj_; }

private:

	GameObject *obj_{nullptr};
	QVariant m_nodeData;
	QList<TreeNode*> m_childNodes;
	TreeNode *m_parentNode;

	// dbg
	int id;
};

#endif // TREENODE_H
