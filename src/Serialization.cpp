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

YAML::Emitter& operator<<(YAML::Emitter& out, const vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const quat& q)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
	return out;
}

void saveGO(YAML::Emitter& out, IGameObject* go)
{
	out << YAML::BeginMap;

	int id;
	go->GetID(&id);
	out << YAML::Key  << "id" << YAML::Value<< id;

	const char *name;
	go->GetName(&name);
	out << YAML::Key  << "name" << YAML::Value<< YAML::DoubleQuoted << name;

	vec3 pos;
	go->GetPosition(&pos);
	out << YAML::Key  << "pos" << YAML::Value<< pos;

	quat rot;
	go->GetRotation(&rot);
	out << YAML::Key  << "rot" << YAML::Value<< rot;

	vec3 scale;
	go->GetScale(&scale);
	out << YAML::Key  << "scale" << YAML::Value<< scale;
}

inline YAML::Emitter& operator<<(YAML::Emitter& emitter, const vector<IResource*>& seq) {
  emitter << YAML::BeginSeq;
  for (typename vector<IResource*>::const_iterator it = seq.begin(); it != seq.end(); ++it)
    emitter << *it;
  emitter << YAML::EndSeq;
  return emitter;
}

YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g)
{
	IGameObject *go = nullptr;
	IModel *m = nullptr;
	ICamera *c = nullptr;

	RES_TYPE type;
	g->GetType(&type);

	if (type == RES_TYPE::CORE_MESH)
	{
		const char *path;
		g->GetFileID(&path);

		out << YAML::LocalTag("Mesh");
		out << YAML::BeginMap;
		out << YAML::Key  << "path" << YAML::Value<< YAML::DoubleQuoted << path;
		out << YAML::EndMap;
		return out;

	} else if (type == RES_TYPE::GAME_OBJECT)
	{
		out << YAML::LocalTag("GameObject");
		g->GetPointer((void**)&go);
	}
	else if (type == RES_TYPE::MODEL)
	{
		out << YAML::LocalTag("Model");
		g->GetPointer((void**)&m);
	}
	else if (type == RES_TYPE::CAMERA)
	{
		out << YAML::LocalTag("Camera");
		g->GetPointer((void**)&c);
	}

	if (go)
		saveGO(out, go);

	else if (m)
	{
		saveGO(out, m);

		Model *ml = dynamic_cast<Model*>(m);
		out << YAML::Key << "meshes" << YAML::Value << ml->_meshes;
	}
	
	else if (c)
	{
		saveGO(out, c);

		Camera *cm = dynamic_cast<Camera*>(c);
		out << YAML::Key << "zNear" << YAML::Value << cm->_zNear;
		out << YAML::Key << "zFar" << YAML::Value << cm->_zFar;
		out << YAML::Key << "fovAngle" << YAML::Value << cm->_fovAngle;
	}

	out << YAML::EndMap;

	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const tree<IResource*>& _tree) 
{
	out << YAML::LocalTag("Tree");
	out << YAML::BeginMap;

	// print tree topology
	out << YAML::Key << "tree_topology" << YAML::Value;
	out << YAML::Block;
	out << YAML::BeginSeq;
	
	for (tree<IResource*>::pre_order_iterator it = _tree.begin(); it != _tree.end(); ++it)
		{
			out << YAML::Flow;
			out << YAML::BeginMap;

			IResource *res = *it;
			IGameObject *go;

			RES_TYPE type;
			res->GetType(&type);
			if (type != RES_TYPE::GAME_OBJECT && type != RES_TYPE::MODEL && type != RES_TYPE::CAMERA)
				continue;

			res->GetPointer((void**)&go);
			int id;
			go->GetID(&id);
			out << YAML::Key << "child" << YAML::Value << id;

			if (_tree.depth(it) <= 0)
				out << YAML::Key << "parent" << YAML::Value << 0;
			else
			{
				int parent_id;
				GameObject *parent_go;
				(*_tree.parent(it))->GetPointer((void**)parent_go);
				parent_go->GetID(&parent_id);
				out << YAML::Key << "parent" << YAML::Value << parent_id;
			}

			out << YAML::EndMap;
		}

	out << YAML::EndSeq;

	// print items
	out << YAML::Key << "tree_items" << YAML::Value;	
	out << YAML::BeginSeq;
	
	for (auto it = _tree.begin(); it != _tree.end(); ++it)
		{
			out << *it;
		}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const SceneManager& sm) 
{
	out << YAML::LocalTag("SceneManager");
	out << YAML::BeginMap;
	out << YAML::Key << "gameobjects" << YAML::Value << sm._gameobjects;
	out << YAML::EndMap;
	return out;
}

IResource *createResource(const string& name)
{
	IResourceManager *resMan;
	_pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	IResource *ret;

	if (name == "!GameObject") resMan->CreateResource(&ret, RES_TYPE::GAME_OBJECT);
	else if (name == "!Model") resMan->CreateResource(&ret, RES_TYPE::MODEL);
	else if (name == "!Camera") resMan->CreateResource(&ret, RES_TYPE::CAMERA);
	//else if (name == "!Mesh") resMan->LoadModel

	return ret;
}

//GameObject *createGameObject(int id)
//{
//	return new GameObject(id);
//}

//Mesh *createMesh(const string& path)
//{
//	return new Mesh{path};
//}

tree<IResource*>::iterator find(tree<IResource*>& tree, int idIn)
{
	for (auto it = tree.begin(); it != tree.end(); ++it)
	{
		int id;
		IGameObject *g;
		(*it)->GetPointer((void**)&g);
		g->GetID(&id);
		if (id == idIn) return it;
	}
	return tree.end();
}

void appendToTree(tree<IResource*>& tree, int parent_id, IResource *res)
{
	auto it = find(tree, parent_id);
	if (it == tree.end()) // root
		tree.insert(tree.begin(), res);
	else
		tree.append_child(it, res);
}

void readSceneManager(YAML::Node& n, SceneManager &sm)
{
	if (!n["gameobjects"])
		return;

	YAML::Node gameobjects_yaml = n["gameobjects"];
	auto t1 = gameobjects_yaml.Type();

	std::map<int, IResource*> pool;

	{
		if (!gameobjects_yaml["tree_items"])
			return;

		YAML::Node tree_items_yaml = gameobjects_yaml["tree_items"];
		auto t3 = tree_items_yaml.Type();

		if (t3 == YAML::NodeType::Sequence)
		{
			for (std::size_t i = 0; i < tree_items_yaml.size(); i++) 
			{
				YAML::Node n = tree_items_yaml[i];
				auto t4 = n.Tag();

				IResource *res = createResource(t4);
				readResource(n, res);

				IGameObject *g;
				res->GetPointer((void**)&g);
				int id;
				g->GetID(&id);
				pool.emplace(id, res);
			}
		}
	}

	{
		if (!gameobjects_yaml["tree_topology"])
			return;

		YAML::Node tree_topology_yaml = gameobjects_yaml["tree_topology"];
		auto t2 = tree_topology_yaml.Type();
		if (t2 == YAML::NodeType::Sequence)
		{
			for (std::size_t i = 0; i < tree_topology_yaml.size(); i++)
			{
				YAML::Node n = tree_topology_yaml[i];
				auto t3 = n.Type();

				int parent_id = n["parent"].as<int>();
				int childs_id = n["child"].as<int>();

				IResource *g = pool[childs_id];

				appendToTree(sm._gameobjects, parent_id, g);
			}
		}
	}
}

void readResource(YAML::Node& n, IResource *go)
{
	RES_TYPE type;
	go->GetType(&type);

	if (type == RES_TYPE::GAME_OBJECT)
	{
		//if (n["id"])
		//	go->_fileID = n["id"].as<int>();
		//if (n["name"])
		//	go->_name = n["name"].as<string>();
		//if (n["pos"])
		//{
		//	YAML::Node pos = n["pos"];
		//	for (std::size_t i = 0; i < 3; i++)
		//		go->_pos.xyz[i] = pos[i].as<float>();
		//}
		//if (n["rot"])
		//{
		//	YAML::Node rot = n["rot"];
		//	for (std::size_t i = 0; i < 4; i++)
		//		go->_rot.xyzw[i] = rot[i].as<float>();
		//}
		//if (n["scale"])
		//{
		//	YAML::Node scale = n["scale"];
		//	for (std::size_t i = 0; i < 3; i++)
		//		go->_scale.xyz[i] = scale[i].as<float>();
		//}
	} else if (type == RES_TYPE::MODEL)
	{
		if (n["meshes"])
		{
			// Todo
			//auto meshes_yaml = n["meshes"];
			//for (std::size_t i = 0; i < meshes_yaml.size(); i++)
			//{
			//	YAML::Node m_yaml = meshes_yaml[i];
			//	auto t = m_yaml.Tag();
			//	Mesh *m = createMesh(m_yaml["path"].as<string>());
			//	ml->meshes.push_back(m);
			//}
		}
	} else if (type == RES_TYPE::CAMERA)
	{
		Camera *cm;
		go->GetPointer((void**)&cm);
		if (n["zNear"])
			cm->_zNear = n["zNear"].as<float>();
		if (n["zFar"])
			cm->_zFar = n["zFar"].as<float>();
		if (n["fovAngle"])
			cm->_fovAngle = n["fovAngle"].as<float>();
	}
}

