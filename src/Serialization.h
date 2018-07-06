#pragma once
#include <tree.h>
#include <sstream>
#include "Filesystem.h"

struct IField
{
	virtual void print(void *, IFile *file, int depth) const = 0;
};

// base class
template <typename C, typename T>
struct entry_base : public IField
{
	T C::* member{nullptr};
	std::string filed_name;

	entry_base(const std::string& nameIn, T C::*memberIn) :
		filed_name(nameIn), member(memberIn) {}

};

inline void print_tabs(IFile *file, int depth)
{
	std::string tabs(depth, ' ');
	file->WriteStr(tabs.c_str());
}

// default
template <typename C, typename T>
struct entry : entry_base<C, T>
{
	using entry_base<C, T>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		std::ostringstream oss;
		oss << this->filed_name << " : " << c->*(this->member) << std::endl;
		print_tabs(file, depth);
		file->WriteStr(oss.str().c_str());
	}
};

// float
template <typename C>
struct entry<C, float> : entry_base<C, float>
{
	using entry_base<C, float>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		float& f = c->*(this->member);
		char buf[60];
		sprintf(buf, "%s : %.3f\n", this->filed_name.c_str(), f);
		print_tabs(file, depth);
		file->WriteStr(buf);
	}
};

// string 
template <typename C>
struct entry<C, std::string> : entry_base<C, std::string>
{
	using entry_base<C, std::string>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		std::string& str = c->*(this->member);
		std::ostringstream oss;
		oss << this->filed_name << " : \"" << str << "\"" << std::endl;
		print_tabs(file, depth);
		file->WriteStr(oss.str().c_str());
	}
};

// vec3
template <typename C>
struct entry<C, vec3> : entry_base<C, vec3>
{
	using entry_base<C, vec3>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		vec3& v3 = c->*(this->member);
		char buf[60];
		sprintf(buf, "%s : {x: %.3f, y: %.3f, z: %.3f}\n", this->filed_name.c_str(), v3.x, v3.y, v3.z);
		print_tabs(file, depth);
		file->WriteStr(buf);
	}
};

// quat
template <typename C>
struct entry<C, quat> : entry_base<C, quat>
{
	using entry_base<C, quat>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		quat& q = c->*(this->member);
		char buf[60];
		sprintf(buf, "%s : {x: %.3f, y: %.3f, z: %.3f, w: %.3f}\n", this->filed_name.c_str(), q.x, q.y, q.z, q.w);
		print_tabs(file, depth);
		file->WriteStr(buf);
	}
};

template <typename C, typename T>
struct sub_entry : public IField
{
	T C::* member;
	std::string filed_name;

	sub_entry(const std::string& nameIn, T C::*member) : filed_name(nameIn), member(member) {}

	void print(void *p, IFile *file, int depth) const override
	{
		p->print(file);
	}
};

class SerializableBase
{
public:
	virtual void serialize(IFile *file, int depth) = 0;
};

template <class Y>
class Serializable : public SerializableBase
{
	std::vector<std::shared_ptr<const IField>> _fields;

public:
	
	virtual void serialize(IFile *file, int depth) override
	{
		std::ostringstream oss;
		std::string str(typeid(*this).name());
		str = str.substr(str.find_first_of(" ") + 1); // remove "class" word
		oss << str << ":" << std::endl;
		file->WriteStr(oss.str().c_str());

		for (auto& entry_base : _fields)
			entry_base->print(dynamic_cast<Y*>(this), file, depth + 1);

		print_tabs(file, depth);
		file->WriteStr("---\n");
	}

	template <typename C, typename T>
	typename std::enable_if<!std::is_base_of<Serializable, T>::value>::type
		add_entry(const std::string& nameIn, T C::* member)
	{
		auto e = new entry<C, T>(nameIn, member);
		_fields.push_back(std::shared_ptr<IField>(e));
	}

	template <typename C, typename T>
	typename std::enable_if<std::is_base_of<Serializable, T>::value>::type
		add_entry(const std::string& nameIn, T C::* member)
	{
		auto e = new sub_entry<C, T>(nameIn, member);
		_fields.push_back(std::shared_ptr<IField>(e));
	}
};


// tree


// T - standard type
template <typename C, typename T>
typename std::enable_if<!std::is_base_of<Serializable<C>, T>::value, void>::type
serialize_collection_node(T& t, IFile *file, std::ostringstream& oss, int depth, bool creturn = false)
{
	SerializableBase *s = dynamic_cast<SerializableBase*>(t);
	if (s)
		s->serialize(file, depth);
	else
	{
		oss << t;
		if (creturn) oss << std::endl;
	}
}

// T - Serializable
//template <typename C, typename T>
//typename std::enable_if<std::is_base_of<Serializable<C>, T>::value, void>::type
//print_node(IFile *file, std::ostringstream& oss, typename tree<T>::pre_order_iterator& it, int depth)
//{
//	T& t = *it;
//	t.serialize(file, depth);
//}

// T is pointer, where T - Serializable
//template <typename C, typename T>
//typename std::enable_if<
//	std::is_pointer<T>::value &&
//	std::is_base_of<Serializable<C>, typename std::remove_pointer<T>::type>::value,
//	void>::type
//	print_(IFile *file, std::ostringstream& oss, typename tree<T>::pre_order_iterator& it, int depth)
//{
//	T* t = &(*it);
//	(*t)->print(file, depth);
//}


template <typename C, typename T>
struct entry<C, tree<T>> : entry_base<C, tree<T>>
{
	using entry_base<C, tree<T>>::entry_base;


	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		tree<T>& tr = c->*(this->member);

		std::ostringstream oss_header;
		oss_header << this->filed_name << " : " << tr.size() << "\n";
		print_tabs(file, depth);
		file->WriteStr(oss_header.str().c_str());


		std::ostringstream oss;
		typename tree<T>::pre_order_iterator it = tr.begin();

		while (it != tr.end())
		{
			print_tabs(file, tr.depth(it) + depth + 1);
			serialize_collection_node<C, T>(*it, file, oss, tr.depth(it) + depth + 1, true);
			it++;
		}		

		file->WriteStr(oss.str().c_str());
	}
};


// std::vector
template <typename C, typename T>
struct entry<C, std::vector<T>> : entry_base<C, std::vector<T>>
{
	using entry_base<C, std::vector<T>>::entry_base;

	void print(void *p, IFile *file, int depth) const override
	{
		C *c = (C*)p;
		std::vector<T>& vec = c->*(this->member);

		std::ostringstream oss;
		oss << this->filed_name << " : " << " [ ";	

		//std::ostringstream oss;
		//typename tree<T>::pre_order_iterator it = tr.begin();

		for (auto& v : vec)
		{
			serialize_collection_node<C, T>(v, file, oss, 0);
		}

		oss << " ]\n";

		print_tabs(file, depth);
		file->WriteStr(oss.str().c_str());
	}
};
