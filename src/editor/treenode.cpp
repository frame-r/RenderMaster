#include "treeNode.h"
#include "editor_common.h"
#include "gameobject.h"
#include <QStringList>
#include <QDebug>


TreeNode::TreeNode(GameObject* obj, TreeNode *parent) : obj_(obj), m_parentNode(parent)
{
	if (obj_)
		m_nodeData = QString(obj_->GetName());
	else
		m_nodeData = QString("<no name>");

	if (obj)
	{
		id = obj->GetId();
		qDebug() << "TreeNode() for " << id;
	}
}

TreeNode::~TreeNode()
{
	if (obj_)
		qDebug() << "delete TreeNode() for " << id;
	qDeleteAll(m_childNodes);
}

void TreeNode::appendChild(TreeNode *node)
{
	m_childNodes.append(node);
}

void TreeNode::removeChild(int row)
{
	m_childNodes.removeAt(row);
}

TreeNode *TreeNode::child(int row) const
{
	return m_childNodes.value(row);
}

int TreeNode::childCount() const
{
	return m_childNodes.count();
}

QVariant TreeNode::data() const
{
	return m_nodeData;
}

TreeNode *TreeNode::parentNode() const
{
	return m_parentNode;
}

int TreeNode::row() const
{
	if (m_parentNode)
		return m_parentNode->m_childNodes.indexOf(const_cast<TreeNode*>(this));

	return 0;
}

void TreeNode::insertChild(int pos, TreeNode *child)
{
	m_childNodes.insert(pos, child);
	child->m_parentNode = this;
}

