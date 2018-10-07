#pragma once

using std::string;
using std::vector;
using std::enable_if;
using std::is_pointer;
using std::is_base_of;
using std::remove_pointer;
using std::ostringstream;
using std::istringstream;
using std::is_integral;


template<typename T>
void f(ostringstream& stream, const string& name, const T& value, int depth, bool tabs = true);

template<typename T>
void r(istringstream& stream, T& value);

inline void print_tabs(std::ostringstream& stream, int depth)
{
	string tabs(depth * 2, ' ');
	stream.write(tabs.c_str(), strlen(tabs.c_str()));
}

class SerializableBase
{
public:
	virtual void serialize(std::ostringstream& stream, int depth) = 0;
	virtual void deserialize(std::istringstream& stream) = 0;
};


// default
template <typename T, typename Enable = void>
class TypeSerializator
{
public:
	static void serilaize(ostringstream& stream, const string& name, const T& value, int depth)
	{
		stream << name << " : " << value << "\n";
	}
	static void deserialize(istringstream& stream, T& value)
	{
		string line;
		std::getline(stream, line);

		istringstream in(line);

		string name;
		char c;

		in >> name >> c >> value;
	}
};

// float
template<>
class TypeSerializator<float>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const float& f, int depth)
	{
		char buf[60];
		sprintf(buf, "%s : %.3f\n", name.c_str(), f);
		stream << buf;
	}
	static void deserialize(istringstream& stream, float& value)
	{
		string line;
		std::getline(stream, line);

		istringstream in(line);

		string name;
		char c;

		in >> name >> c >> value;
	}
};

// vec3
template<>
class TypeSerializator<vec3>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const vec3& v3, int depth)
	{
		char buf[60];
		sprintf(buf, "%s : {x: %.3f, y: %.3f, z: %.3f}\n", name.c_str(), v3.x, v3.y, v3.z);
		stream << buf;
	}
	static void deserialize(istringstream& stream, vec3& v3)
	{
		string line;
		std::getline(stream, line);
		char name[10];
		sscanf(line.c_str(), "%s : {x: %f, y: %f, z: %f}\n", name, &v3.x, &v3.y, &v3.z);
	}
};

// quat
template<>
class TypeSerializator<quat>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const quat& q, int depth)
	{
		char buf[60];
		sprintf(buf, "%s : {x: %.3f, y: %.3f, z: %.3f, w: %.3f}\n", name.c_str(), q.x, q.y, q.z, q.w);
		stream << buf;
	}
	static void deserialize(istringstream& stream, quat& q)
	{
		string line;
		std::getline(stream, line);
		char name[10];
		sscanf(line.c_str(), "%s : {x: %f, y: %f, z: %f, w: %f}\n", name, &q.x, &q.y, &q.z, &q.w);
	}
};

// string
template<>
class TypeSerializator<string>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const string& value, int depth)
	{
		stream << name << " : \"" << value << "\"\n";
	}
	static void deserialize(istringstream& stream, string& s)
	{
		string line;
		std::getline(stream, line);
		char name[10];
		char val[260];
		sscanf(line.c_str(), "%s : \"%[^\"]\"\n", name, val);
		s = string(val);
	}
};

// IResource*
template<>
class TypeSerializator<IResource*>
{
public:
	static void serilaize(ostringstream& stream, const string& name, IResource* value, int depth)
	{
		RES_TYPE type;
		value->GetType(&type);
	
		IGameObject *go;
		value->GetPointer((void**)&go);
		SerializableBase* gosb = dynamic_cast<SerializableBase*>(go);
		if (gosb)
		{
			f(stream, name, gosb, depth);
		} else
		{
			assert(type == RES_TYPE::CORE_MESH);
			const char *mesh_path;
			value->GetFileID(&mesh_path);
			stream << "\"" << mesh_path << "\"\n";
		}
	}
	static void deserialize(istringstream& stream, string& s)
	{
		//string line;
		//std::getline(stream, line);
		//char name[10];
		//char val[260];
		//sscanf(line.c_str(), "%s : \"%[^\"]\"\n", name, val);
		//s = string(val);
		assert(false); // not impl
	}
};

// SerializableBase
template <typename T>
class TypeSerializator<T, typename enable_if<is_base_of<SerializableBase, T>::value>::type>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const T& value, int depth)
	{
		//stream << "inner serializable: \n";
		SerializableBase* s = (SerializableBase*)&value;
		s->serialize(stream, depth);
	}
};

// SerializableBase*
template <typename T>
class TypeSerializator<T, typename enable_if<is_pointer<T>::value && is_base_of<SerializableBase, typename remove_pointer<T>::type>::value>::type>
{
public:
	static void serilaize(ostringstream& stream, const string& name, const T& value, int depth)
	{
		SerializableBase* s = (SerializableBase*)value;
		s->serialize(stream, depth);
	}
	static void deserialize(istringstream& stream, T& value)
	{
		SerializableBase* s = (SerializableBase*)value;
		s->deserialize(stream);
	}
};



////////////////////////////
//
////////////////////////////

template<typename T>
void f(ostringstream& stream, const string& name, const T& value, int depth, bool tabs)
{
	if (tabs) print_tabs(stream, depth);
	TypeSerializator<T>::serilaize(stream, name, value, depth);
}

template<typename T>
void r(istringstream& stream, T& value)
{
	TypeSerializator<T>::deserialize(stream, value);
}




class Fabric
{
public:
	static SerializableBase* create(string className);
};

struct IField
{
	virtual void print(void *object, std::ostringstream& stream, int depth) const = 0;
	virtual void scan(void *object, std::istringstream& stream) const = 0;
};


//
// Base class that serializes one field (member) in class
// C - class that contains field
// T - type of field
//
template <typename C, typename T>
struct FieldBase : public IField
{
	T C::* member{nullptr};
	string field_name;

	FieldBase(const string& nameIn, T C::*memberIn) : field_name(nameIn), member(memberIn) {}

	void print(void *object, std::ostringstream& stream, int depth) const override
	{
		C *c = (C*)object;
		f<T>(stream, this->field_name, c->*(this->member), depth);
	}

	void scan(void *object, std::istringstream& stream) const override
	{
		C *c = (C*)object;
		T& field = c->*(this->member);
		r<T>(stream, field);
	}
};

//
// Specialization for vector
// C - class that contains field
// V - type in vector
//
template <typename C, typename V>
struct FieldBase<C, vector<V>> : public IField
{
	vector<V> C::*member;
	string field_name;

	FieldBase(const string& nameIn, vector<V> C::*memberIn) : field_name(nameIn), member(memberIn) {}

	void print(void *object, std::ostringstream& stream, int depth) const
	{
		C *c = (C*)object;
		vector<V>& vec = c->*(this->member);

		print_tabs(stream, depth);

		stream << this->field_name << " : {num: " << vec.size() << "}\n";

		for (auto& v : vec)
		{
			print_tabs(stream, depth + 1);
			stream << "- ";

			f(stream, "", v, depth + 1, false);
		}
	}

	void scan(void *object, std::istringstream& stream) const
	{
		assert(false); // not impl
	}
};


//
// Specialization for tree
// C - class that contains field
// V - type in tree
//
template <typename C, typename V>
struct FieldBase<C, tree<V>> : public IField
{
	tree<V> C::*member;
	string field_name;

	FieldBase(const string& nameIn, tree<V> C::*memberIn) : field_name(nameIn), member(memberIn) {}

	void print(void *object, std::ostringstream& stream, int depth) const
	{
		C *c = (C*)object;
		tree<V>& _tree = c->*(this->member);

		//print_tabs(stream, depth);

		//stream << this->field_name << " :\n";

		// topology		
		print_tabs(stream, depth);
		stream << "tree_topology : {num: " << _tree.size();

		typename tree<V>::pre_order_iterator it = _tree.begin();

		for (auto it = _tree.begin(); it != _tree.end(); ++it)
		{
			IGameObject *go;
			(*it)->GetPointer((void**)&go);

			int child_id;
			go->GetID(&child_id);

			if (_tree.depth(it) <= 0)
				stream << ", " << child_id << " : " << 0;
			else
			{
				int parent_id;
				IGameObject *parent_go;
				(*_tree.parent(it))->GetPointer((void**)&parent_go);
				parent_go->GetID(&parent_id);
				stream << ", " << child_id << " : " << parent_id;
			}
		}
		stream << "}\n";

		for (auto& v : _tree)
		{
			print_tabs(stream, depth);

			stream << "-\n";

			//SerializableBase *s = dynamic_cast<SerializableBase*>(v);
			f(stream, field_name, v, depth);
		}
	}

	void scan(void *object, istringstream& stream) const
	{
		//C *c = (C*)object;
		//tree<V>& _tree = c->*(this->member);

		//string line;
		//std::getline(stream, line);

		//char name[20];
		//int size;
		//int pos;

		//sscanf(line.c_str(), "%s : {num : %i%n", name, &size, &pos);

		//if (size == 0)
		//	return;

		//pos++; // skip ","

		//line = line.substr(pos);
		//istringstream in(line);

		//std::map<int, int> parent_map; // id -> parent id

		//for (int i = 0; i < size; i++)
		//{
		//	int id, parent_id;
		//	char c;
		//	in >> id >> c >> parent_id;

		//	parent_map[id] = parent_id;

		//	if (i != size - 1) in >> c; // ","
		//}

		//for (int i = 0; i < size; i++)
		//{
		//	string line;
		//	std::getline(stream, line); // "-"
		//	std::getline(stream, line); // class name

		//	char class_name[260];
		//	sscanf(line.c_str(), "%s :", class_name);

		//	SerializableBase *obj = Fabric::create(string(class_name));
		//	V v_obj = dynamic_cast<V>(obj);

		//	r(stream, obj);

		//	int parent_id;
		//	v_obj->GetID(&parent_id);
		//	parent_map[parent_id];

		//	auto get_item_by_id = [&](int id) -> typename tree<V>::iterator
		//	{
		//		for (auto it = _tree.begin(); it != _tree.end(); ++it)
		//		{
		//			int id_;
		//			(*it)->GetID(&id_);
		//			if (id_ == id) return it;
		//		}
		//		return _tree.end();
		//	};

		//	if (parent_id == 0) // => at 0 tree level
		//		_tree.insert(_tree.begin(), v_obj);
		//	else
		//	{ // some child
		//		auto it = get_item_by_id(parent_id);
		//		if (it != _tree.end())
		//			_tree.append_child(it, v_obj);
		//	}
		//}
	}
};


//
// Class that serializes one field in case when member is SerializableBase
// C - class that contains field
// T - type of field (always SerializableBase)
//
template <typename C, typename T>
struct SubField : public IField
{
	T C::* member;
	string field_name;

	SubField(const string& nameIn, T C::*member) : field_name(nameIn), member(member) {}

	void print(void *object, std::ostringstream& stream, int depth) const override
	{
		C *c = (C*)object;
		T *t = (T*)&(c->*(this->member));
		print_tabs(stream, depth);
		stream << this->field_name << " :\n";
		t->serialize(stream, depth + 1);
	}

	void scan(void *object, std::istringstream& stream) const override
	{
		assert(false); // not impl
	}
};

template <class Y>
class Serializable : public SerializableBase
{
	vector<std::shared_ptr<const IField>> _fields;

public:

	~Serializable() {}

	virtual void serialize(std::ostringstream& stream, int depth = 0) override
	{
		string str(typeid(*this).name());
		str = str.substr(str.find_first_of(" ") + 1); // remove "class" word

		print_tabs(stream, depth);

		stream << str << " :" << std::endl;

		for (auto& f : _fields)
			f->print(dynamic_cast<Y*>(this), stream, depth + 1);

		//print_tabs(stream, depth);
		//stream << "---\n";
	}

	virtual void deserialize(std::istringstream& stream) override
	{
		for (auto& f : _fields)
			f->scan(dynamic_cast<Y*>(this), stream);
	}

	// Add not serializable field
	template <typename C, typename T>
	typename std::enable_if<!std::is_base_of<SerializableBase, T>::value>::type	add_entry(const string& nameIn, T C::*member)
	{
		_fields.push_back(std::shared_ptr<IField>(new FieldBase<C, T>(nameIn, member)));
	}

	// Add serializable field
	template <typename C, typename T>
	typename std::enable_if<std::is_base_of<SerializableBase, T>::value>::type add_entry(const string& nameIn, T C::*member)
	{
		_fields.push_back(std::shared_ptr<IField>(new SubField<C, T>(nameIn, member)));
	}
};


