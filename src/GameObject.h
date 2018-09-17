#pragma once
#include "Common.h"
#include "Events.h"
#include "Serialization.h"

template <typename T>
class GameObjectBase : public T, public Serializable<T>
{
protected:

	int _id;
	std::string _name{"GameObject"};
	vec3 _pos;
	quat _rot;
	vec3 _scale{1.0f, 1.0f, 1.0f};
	
	std::unique_ptr<PositionEvent> _positionEvent{new PositionEvent};
	std::unique_ptr<RotationEvent> _rotationEvent{new RotationEvent};
	std::unique_ptr<StringEvent> _nameEvent{new StringEvent};

public:

	GameObjectBase()
	{
		add_entry("id", &GameObjectBase::_id);
		add_entry("name", &GameObjectBase::_name);
		add_entry("positon", &GameObjectBase::_pos);
		add_entry("rotation", &GameObjectBase::_rot);
		add_entry("scale", &GameObjectBase::_scale);

		_id = getRandomInt();
	}

	API GetID(OUT int *id) override;
	API GetName(OUT const char **pName) override;
	API SetName(const char *pName) override;
	API SetPosition(const vec3 *pos) override;
	API SetRotation(const quat *rot) override;
	API SetScale(const vec3 *scale) override;
	API GetPosition(OUT vec3 *pos) override;
	API GetRotation(OUT quat *rot) override;
	API GetScale(OUT vec3 *scale) override;

	//
	// Model Matrix
	// Matrix transforms coordinates local -> world
	// p' (world) = mat * p (local)
	// last column - translate in world space
	// columns - (Right, Forward, Up) vectors in world space
	//
	API GetModelMatrix(OUT mat4 *mat) override;

	//
	// Inverse of Model Matrix
	// Matrix transforms coordinates world -> local
	// p' (local) = mat * p (world)
	//
	API GetInvModelMatrix(OUT mat4 *mat) override;


	API GetNameEv(OUT IStringEvent **pEvent) override;
	API GetPositionEv(OUT IPositionEvent **pEvent) override;
	API GetRotationEv(OUT IRotationEvent **pEvent) override;
};

class GameObject : public GameObjectBase<IGameObject> {};



// implementation

template<typename T>
inline API GameObjectBase<T>::GetID(OUT int *id)
{
	*id = _id;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetName(OUT const char ** pName)
{
	*pName = _name.c_str();
	return S_OK;
}

template <typename T>
inline API GameObjectBase<T>::SetName(const char* pName)
{
	std::string name = std::string(pName);
	if (name != _name)
	{
		_name = name;
		_nameEvent->Fire(name.c_str());
	}
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::SetPosition(const vec3 * pos)
{
	if (!_pos.Aproximately(*pos))
	{
		_pos = *pos;
		_positionEvent->Fire(&_pos);
	}
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::SetRotation(const quat *rot)
{
    if (!_rot.IsSameRotation(*rot))
	{
		_rot = *rot;
		_rotationEvent->Fire(&_rot);
	}
	return S_OK;
}

template <typename T>
inline API GameObjectBase<T>::SetScale(const vec3* scale)
{
	_scale = *scale;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetPosition(OUT vec3 * pos)
{
	*pos = _pos;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetRotation(OUT quat *rot)
{
	*rot = _rot;
	return S_OK;
}

template <typename T>
inline API GameObjectBase<T>::GetScale(vec3* scale)
{
	*scale = _scale;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetModelMatrix(OUT mat4 *mat)
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
inline API GameObjectBase<T>::GetInvModelMatrix(OUT mat4 *mat)
{
	mat4 M;

	GetModelMatrix(&M);

	*mat = M.Inverse();

	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetNameEv(OUT IStringEvent **pEvent)
{
	*pEvent = _nameEvent.get();
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetPositionEv(OUT IPositionEvent **pEvent)
{
	*pEvent = _positionEvent.get();
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetRotationEv(OUT IRotationEvent **pEvent)
{
	*pEvent = _rotationEvent.get();
	return S_OK;
}
