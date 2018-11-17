#include "Pch.h"
#include "Common.h"
#include "Serialization.h"
#include "Model.h"
#include "Camera.h"
#include "SceneManager.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

using namespace YAML;
/*
Emitter& operator<<(Emitter& out, const vec3& v)
{
	out << Flow;
	out << BeginSeq << v.x << v.y << v.z << EndSeq;
	return out;
}

Emitter& operator<<(Emitter& out, const quat& q)
{
	out << Flow;
	out << BeginSeq << q.x << q.y << q.z << q.w << EndSeq;
	return out;
}

void saveGameObjectBase(Emitter& out, IGameObject* go)
{
	out << BeginMap;

	int id;
	go->GetID(&id);
	out << Key  << "id" << Value<< id;

	const char *name;
	go->GetName(&name);
	out << Key  << "name" << Value<< DoubleQuoted << name;

	vec3 pos;
	go->GetPosition(&pos);
	out << Key  << "pos" << Value<< pos;

	quat rot;
	go->GetRotation(&rot);
	out << Key  << "rot" << Value<< rot;

	vec3 scale;
	go->GetScale(&scale);
	out << Key  << "scale" << Value<< scale;
}

inline Emitter& operator<<(Emitter& out, const vector<IResource*>& seq)
{
	out << BeginSeq;
	for (typename vector<IResource*>::const_iterator it = seq.begin(); it != seq.end(); ++it)
	  out << *it;
	out << EndSeq;
	return out;
}

Emitter& operator<<(Emitter& out, IResource* res)
{
	IGameObject *go = nullptr;
	IModel *m = nullptr;
	ICamera *c = nullptr;

	RES_TYPE type;
	res->GetType(&type);

	if (type == RES_TYPE::CORE_MESH)
	{
		const char *path;
		res->GetFileID(&path);

		out << LocalTag("Mesh");
		out << BeginMap;
		out << Key  << "path" << Value<< DoubleQuoted << path;
		out << EndMap;
		return out;

	}

	if (go)
		saveGameObjectBase(out, go);

	else if (m)
	{
		saveGameObjectBase(out, m);

		Model *ml = dynamic_cast<Model*>(m);
		out << Key << "meshes" << Value << ml->_meshes;
	}
	
	else if (c)
	{
		saveGameObjectBase(out, c);

		Camera *cm = dynamic_cast<Camera*>(c);
		out << Key << "zNear" << Value << cm->_zNear;
		out << Key << "zFar" << Value << cm->_zFar;
		out << Key << "fovAngle" << Value << cm->_fovAngle;
	}

	out << EndMap;

	return out;
}

Emitter& operator<<(Emitter& out, const tree<IGameObject*>& _tree) 
{
	out << LocalTag("Tree");
	out << BeginMap;

	// print tree topology
	out << Key << "tree_topology" << Value;
	out << Block;
	out << BeginSeq;
	
	for (tree<IGameObject*>::pre_order_iterator it = _tree.begin(); it != _tree.end(); ++it)
		{
			out << Flow;
			out << BeginMap;

			IGameObject *go= *it;

			int id;
			go->GetID(&id);
			out << Key << "child" << Value << id;

			if (_tree.depth(it) <= 0)
				out << Key << "parent" << Value << 0;
			else
			{
				int parent_id;
				IGameObject *parent_go = *_tree.parent(it);
				parent_go->GetID(&parent_id);
				out << Key << "parent" << Value << parent_id;
			}

			out << EndMap;
		}

	out << EndSeq;

	// print items
	out << Key << "tree_items" << Value;	
	out << BeginSeq;
	
	for (auto it = _tree.begin(); it != _tree.end(); ++it)
		{
			out << *it;
		}

	out << EndSeq;
	out << EndMap;

	return out;
}

Emitter& operator<<(Emitter& out, const SceneManager& sm) 
{
	out << LocalTag("SceneManager");
	out << BeginMap;
	out << Key << "gameobjects" << Value << sm._gameobjects;
	out << EndMap;
	return out;
}

IGameObject *createSceneObject(const string& name)
{
	IResourceManager *resMan;
	_pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	IGameObject *ret = nullptr;

	if (name == "!GameObject") resMan->CreateGameObject(&ret);
	else if (name == "!Model") resMan->CreateGameObject(&ret);
	else if (name == "!Camera") resMan->CreateGameObject(&ret);

	return ret;
}

tree<IGameObject*>::iterator find(tree<IGameObject*>& tree, int idIn)
{
	for (auto it = tree.begin(); it != tree.end(); ++it)
	{
		int id;
		IGameObject *g = *it;
		g->GetID(&id);
		if (id == idIn)
			return it;
	}
	return tree.end();
}

//void insertResourceToTree(SceneManager& sm, int parent_id, IGameObject *res)
//{
//	auto it = find(sm._gameobjects, parent_id);
//	if (it == sm._gameobjects.end()) // root
//	{
//		sm.AddRootGameObject(res);
//	}
//	else
//	{
//		// TODO: do through SceneManager API!!! else child object won't shown in tree hierarchy
//		sm._gameobjects.append_child(it, res);
//		res->AddRef();
//	}
//}

void loadSceneManager(Node& n, SceneManager &sm)
{
	if (!n["gameobjects"])
		return;

	Node gameobjects_yaml = n["gameobjects"];
	auto t1 = gameobjects_yaml.Type();

	std::map<int, IGameObject*> pool;

	{
		if (!gameobjects_yaml["tree_items"])
			return;

		Node tree_items_yaml = gameobjects_yaml["tree_items"];
		auto t3 = tree_items_yaml.Type();

		if (t3 == NodeType::Sequence)
		{
			for (std::size_t i = 0; i < tree_items_yaml.size(); i++) 
			{
				Node n = tree_items_yaml[i];
				auto t4 = n.Tag();

				IGameObject *go = createSceneObject(t4);

				loadResource(n, go);

				int id;
				go->GetID(&id);

				pool.emplace(id, go);
			}
		}
	}

	{
		if (!gameobjects_yaml["tree_topology"])
			return;

		Node tree_topology_yaml = gameobjects_yaml["tree_topology"];
		auto t2 = tree_topology_yaml.Type();
		if (t2 == NodeType::Sequence)
		{
			for (std::size_t i = 0; i < tree_topology_yaml.size(); i++)
			{
				Node n = tree_topology_yaml[i];
				auto t3 = n.Type();

				int parent_id = n["parent"].as<int>();
				int childs_id = n["child"].as<int>();

				IGameObject *res = pool[childs_id];

				//insertResourceToTree(sm, parent_id, res);
			}
		}
	}
}

void loadGameObjectBase(Node& n, IGameObject *go)
{
	if (n["id"])
	{
		int id = n["id"].as<int>();
		go->SetID(&id);
	}
	if (n["name"])
	{
		string name = n["name"].as<string>();
		go->SetName(name.c_str());
	}
	if (n["pos"])
	{
		vec3 v3;
		Node pos = n["pos"];
		for (std::size_t i = 0; i < 3; i++)
			v3.xyz[i] = pos[i].as<float>();
		go->SetPosition(&v3);
	}
	if (n["rot"])
	{
		quat q;
		Node rot = n["rot"];
		for (std::size_t i = 0; i < 4; i++)
			q.xyzw[i] = rot[i].as<float>();
		go->SetRotation(&q);
	}
	if (n["scale"])
	{
		vec3 s;
		Node scale = n["scale"];
		for (std::size_t i = 0; i < 3; i++)
			s.xyz[i] = scale[i].as<float>();
		go->SetScale(&s);
	}
}

void loadResource(Node& n, IGameObject *go)
{
	loadGameObjectBase(n, go);

	Model *ml = dynamic_cast<Model*>(go);
	if (ml)
	{
		if (n["meshes"])
		{
			IResourceManager *resMan;
			_pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			auto meshes_yaml = n["meshes"];
			for (std::size_t i = 0; i < meshes_yaml.size(); i++)
			{
				Node m_yaml = meshes_yaml[i];
				auto t = m_yaml.Tag();

				IResource *mesh;
				resMan->LoadMesh(&mesh, m_yaml["path"].as<string>().c_str());

				mesh->AddRef();

				ml->_meshes.push_back(mesh);
			}
		}
	}

	Camera *cm = dynamic_cast<Camera*>(go);
	if (cm)
	{
		if (n["zNear"])
			cm->_zNear = n["zNear"].as<float>();
		if (n["zFar"])
			cm->_zFar = n["zFar"].as<float>();
		if (n["fovAngle"])
			cm->_fovAngle = n["fovAngle"].as<float>();
	}
}

*/
