#pragma once
#include "Common.h"

template <typename T>
class GameObjectBase : public T
{
protected:

	uint _id;
	string _name{"GameObject"};
	vec3 _pos;
	quat _rot;
	vec3 _scale{1.0f, 1.0f, 1.0f};

	IGameObject *_parent = nullptr;
	vector<IGameObject*> _childs;
	
	std::unique_ptr<PositionEvent> _positionEvent{new PositionEvent};
	std::unique_ptr<RotationEvent> _rotationEvent{new RotationEvent};
	std::unique_ptr<StringEvent> _nameEvent{new StringEvent};

public:

	GameObjectBase(){ _id = getRandomInt(); }
	virtual ~GameObjectBase() {}

	API_VOID GetID(OUT uint *id) override					{ *id = _id; }
	API_VOID SetID(uint *id) override						{ _id = *id; }
	API_VOID GetName(OUT const char **pName) override		{ *pName = _name.c_str(); }
	API_VOID SetName(const char *pName) override;
	API_VOID SetPosition(const vec3 *pos) override;
	API_VOID SetRotation(const quat *rot) override;
	API_VOID SetScale(const vec3 *scale) override			{ _scale = *scale;; }
	API_VOID GetPosition(OUT vec3 *pos) override			{ *pos = _pos; }
	API_VOID GetRotation(OUT quat *rot) override			{ *rot = _rot; }
	API_VOID GetScale(OUT vec3 *scale) override				{ *scale = _scale; }
	API_VOID GetAABB(OUT AABB *aabb) override				{ *aabb = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f}; }

	//
	// Model Matrix
	//
	// Transforms local -> world coordinates
	// p' (world) = mat * p (local)
	//
	// Note:
	// * Translate in world space = last column 
	// * Columns = (Right, Forward, Up) vectors in world space
	// * View vector = -M.el_2d.Column3(2).Normalized();
	API_VOID GetModelMatrix(OUT mat4 *mat) override;

	//
	// Inverse of Model Matrix
	//
	// Transforms world -> local coordinates
	// p' (local) = mat * p (world)
	//
	API_VOID GetInvModelMatrix(OUT mat4 *mat) override;

	API_VOID Copy(IGameObject *copy) override;

	API_VOID GetNameEv(OUT IStringEvent **pEvent) override			{ *pEvent = _nameEvent.get(); }
	API_VOID GetPositionEv(OUT IPositionEvent **pEvent) override	{ *pEvent = _positionEvent.get(); }
	API_VOID GetRotationEv(OUT IRotationEvent **pEvent) override	{ *pEvent = _rotationEvent.get(); }
};

class GameObject : public BaseResource<GameObjectBase<IGameObject>>
{
public:
	GameObject(IGameObject *parent = nullptr) { _parent = parent; }
	virtual ~GameObject() {}
};

// implementation

template <typename T>
inline API_VOID GameObjectBase<T>::SetName(const char* pName)
{
	string name = string(pName);
	if (name != _name)
	{
		_name = name;
		_nameEvent->Fire(name.c_str());
	}
}

template<typename T>
inline API_VOID GameObjectBase<T>::SetPosition(const vec3 * pos)
{
	if (!_pos.Aproximately(*pos))
	{
		_pos = *pos;
		_positionEvent->Fire(&_pos);
	}
}

template<typename T>
inline API_VOID GameObjectBase<T>::SetRotation(const quat *rot)
{
    if (!_rot.IsSameRotation(*rot))
	{
		_rot = *rot;
		_rotationEvent->Fire(&_rot);
	}
}

template<typename T>
inline API_VOID GameObjectBase<T>::Copy(IGameObject *dst)
{
	dst->SetName(_name.c_str());
	dst->SetPosition(&_pos);
	dst->SetRotation(&_rot);
	dst->SetScale(&_scale);
}

template<typename T>
inline API_VOID GameObjectBase<T>::GetModelMatrix(OUT mat4 *mat)
{
	mat4 R;
	mat4 T;
	mat4 S;

	R = _rot.ToMatrix();

	T.el_2D[0][3] = _pos.x;
	T.el_2D[1][3] = _pos.y;
	T.el_2D[2][3] = _pos.z;

	S.el_2D[0][0] = _scale.x;
	S.el_2D[1][1] = _scale.y;
	S.el_2D[2][2] = _scale.z;

	*mat = T * R * S;
}

template<typename T>
inline API_VOID GameObjectBase<T>::GetInvModelMatrix(OUT mat4 *mat)
{
	mat4 M;
	GetModelMatrix(&M);

	*mat = M.Inverse();
}

