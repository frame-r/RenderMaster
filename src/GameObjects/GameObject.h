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
	
	std::unique_ptr<PositionEvent> _positionEvent{new PositionEvent};
	std::unique_ptr<RotationEvent> _rotationEvent{new RotationEvent};
	std::unique_ptr<StringEvent> _nameEvent{new StringEvent};

public:

	GameObjectBase(){ _id = getRandomInt(); }
	virtual ~GameObjectBase() {}

	API_RESULT GetID(OUT uint *id) override					{ *id = _id; return S_OK; }
	API_RESULT SetID(uint *id) override						{ _id = *id; return S_OK; }
	API_RESULT GetName(OUT const char **pName) override	{ *pName = _name.c_str(); return S_OK; }
	API_RESULT SetName(const char *pName) override;
	API_RESULT SetPosition(const vec3 *pos) override;
	API_RESULT SetRotation(const quat *rot) override;
	API_RESULT SetScale(const vec3 *scale) override		{ _scale = *scale; return S_OK; }
	API_RESULT GetPosition(OUT vec3 *pos) override			{ *pos = _pos; return S_OK; }
	API_RESULT GetRotation(OUT quat *rot) override			{ *rot = _rot; return S_OK; }
	API_RESULT GetScale(OUT vec3 *scale) override			{ *scale = _scale; return S_OK; }
	API_RESULT GetAABB(OUT AABB *aabb) override;
	API_RESULT Copy(IGameObject *copy) override;

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
	API_RESULT GetModelMatrix(OUT mat4 *mat) override;

	//
	// Inverse of Model Matrix
	//
	// Transforms world -> local coordinates
	// p' (local) = mat * p (world)
	//
	API_RESULT GetInvModelMatrix(OUT mat4 *mat) override;

	API_RESULT GetNameEv(OUT IStringEvent **pEvent) override			{ *pEvent = _nameEvent.get(); return S_OK; }
	API_RESULT GetPositionEv(OUT IPositionEvent **pEvent) override		{ *pEvent = _positionEvent.get(); return S_OK; }
	API_RESULT GetRotationEv(OUT IRotationEvent **pEvent) override		{ *pEvent = _rotationEvent.get(); return S_OK; }
};

class GameObject : public BaseResource<GameObjectBase<IGameObject>>
{
public:
	virtual ~GameObject() {}
};

// implementation

template <typename T>
inline API_RESULT GameObjectBase<T>::SetName(const char* pName)
{
	string name = string(pName);
	if (name != _name)
	{
		_name = name;
		_nameEvent->Fire(name.c_str());
	}
	return S_OK;
}

template<typename T>
inline API_RESULT GameObjectBase<T>::SetPosition(const vec3 * pos)
{
	if (!_pos.Aproximately(*pos))
	{
		_pos = *pos;
		_positionEvent->Fire(&_pos);
	}
	return S_OK;
}

template<typename T>
inline API_RESULT GameObjectBase<T>::SetRotation(const quat *rot)
{
    if (!_rot.IsSameRotation(*rot))
	{
		_rot = *rot;
		_rotationEvent->Fire(&_rot);
	}
	return S_OK;
}

template <typename T>
inline API_RESULT GameObjectBase<T>::GetAABB(OUT AABB* aabb)
{
	const static AABB _unitAABB = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f};
	*aabb = _unitAABB;
	return S_OK;
}

template<typename T>
inline API_RESULT GameObjectBase<T>::Copy(IGameObject *copy)
{
	copy->SetName(_name.c_str());
	copy->SetPosition(&_pos);
	copy->SetRotation(&_rot);
	copy->SetScale(&_scale);
	return S_OK;
}

template<typename T>
inline API_RESULT GameObjectBase<T>::GetModelMatrix(OUT mat4 *mat)
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

	return S_OK;
}

template<typename T>
inline API_RESULT GameObjectBase<T>::GetInvModelMatrix(OUT mat4 *mat)
{
	mat4 M;

	GetModelMatrix(&M);

	*mat = M.Inverse();

	return S_OK;
}

