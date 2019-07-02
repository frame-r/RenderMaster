#pragma once
#include "common.h"


enum class OBJECT_TYPE
{
	GAMEOBJECT,
	MODEL,
	LIGHT,
	CAMERA
};

const char *getNameByType(OBJECT_TYPE type);
OBJECT_TYPE getTypeByName(const std::string& name);

class GameObject
{
protected:
	int id_;
	std::string name_{"GameObject"};
	bool enabled_{true};
	OBJECT_TYPE type_{OBJECT_TYPE::GAMEOBJECT};

	// Local (relative parent)
	vec3 pos_;
	quat rot_;
	vec3 scale_{1.0f, 1.0f, 1.0f};
	mat4 localTransform_; // transforms Local -> Parent coordinates

	// World (relative world)
	vec3 worldPos_;
	quat worldRot_;
	vec3 worldScale_{1.0f, 1.0f, 1.0f};
	mat4 worldTransform_; // transforms Local -> World coordinates

	mat4 worldTransformPrev_;

	GameObject *parent_{nullptr};
	std::vector<GameObject*> childs_;

	virtual void Copy(GameObject *original);

public:
	GameObject();
	virtual ~GameObject();

	// Interanl API
	virtual void Update(float dt);
	virtual void Serialize(void *yaml);
	virtual void Deserialize(void *yaml);

public:
	auto DLLEXPORT GetName() -> const char* { return name_.c_str(); }
	auto DLLEXPORT SetName(const char *name) -> void { name_ = name; }
	auto DLLEXPORT GetId() -> int { return id_; }
	auto DLLEXPORT SetId(int id) -> void { id_ = id; }
	auto DLLEXPORT SetEnabled(bool v) -> void { enabled_ = v; }
	auto DLLEXPORT IsEnabled() -> bool { return enabled_; }
	auto DLLEXPORT GetType() -> OBJECT_TYPE { return type_; }

	// Transformation
	auto DLLEXPORT SetLocalPosition(vec3 pos) -> void;
	auto DLLEXPORT GetLocalPosition() -> vec3;
	auto DLLEXPORT SetLocalRotation(quat pos) -> void;
	auto DLLEXPORT GetLocalRotation() -> quat;
	auto DLLEXPORT SetLocalScale(vec3 pos) -> void;
	auto DLLEXPORT GetLocalScale() -> vec3;
	auto DLLEXPORT SetLocalTransform(const mat4& m) -> void;
	auto DLLEXPORT GetLocalTransform() -> mat4;
	auto DLLEXPORT SetWorldPosition(vec3 pos) -> void;
	auto DLLEXPORT GetWorldPosition() -> vec3;
	auto DLLEXPORT SetWorldRotation(const quat& r) -> void;
	auto DLLEXPORT GetWorldRotation() -> quat;
	auto DLLEXPORT SetWorldScale(vec3 s) -> void;
	auto DLLEXPORT GetWorldScale() -> vec3;
	auto DLLEXPORT SetWorldTransform(const mat4& m) -> void;
	auto DLLEXPORT GetWorldTransform() -> mat4;
	auto DLLEXPORT GetInvWorldTransform() -> mat4;
	auto DLLEXPORT GetWorldTransformPrev()->mat4;

	// Hierarchy
	auto DLLEXPORT GetParent() -> GameObject* { return parent_; }
	auto DLLEXPORT GetNumChilds() -> size_t { return childs_.size(); }
	auto DLLEXPORT GetChild(size_t i) -> GameObject*;
	auto DLLEXPORT RemoveChild(GameObject *obj) -> void;
	auto DLLEXPORT InsertChild(GameObject *obj, int row = -1) -> void;
	auto DLLEXPORT virtual Clone() -> GameObject*;

	// debug
	void DLLEXPORT print_local();
	void DLLEXPORT print_global();
};
