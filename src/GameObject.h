#pragma once
#include "Common.h"
#include "Events.h"

template <typename T>
class GameObjectBase : public T
{
protected:

	std::string _name{"GameObject"};

	vec3 _pos{0.0f, 0.0f, 0.0f};
	vec3 _rot{0.0f, 0.0f, 0.0f};
	vec3 _scale{1.0f, 1.0f, 1.0f};
	
	std::unique_ptr<PositionEvent> _positionEvent{new PositionEvent};

public:

	API GetName(OUT const char **pName) override;
	API SetPosition(const vec3 *pos) override;
	API SetRotation(const vec3 *rot) override;
	API GetPosition(OUT vec3 *pos) override;
	API GetRotation(OUT vec3 *rot) override;
	API GetModelMatrix(OUT mat4 *mat) override;
	API GetPositionEv(OUT IPositionEvent **pEvent) override;
};

class GameObject : public GameObjectBase<IGameObject> {};



// implementation

template<typename T>
inline API GameObjectBase<T>::GetName(OUT const char ** pName)
{
	*pName = _name.c_str();
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
inline API GameObjectBase<T>::SetRotation(const vec3 * rot)
{
	_rot = *rot;
	_rot.x = fmod(_rot.x, 360.0f);
	_rot.y = fmod(_rot.y, 360.0f);
	_rot.z = fmod(_rot.z, 360.0f);
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetPosition(OUT vec3 * pos)
{
	*pos = _pos;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetRotation(OUT vec3 * rot)
{
	*rot = _rot;
	return S_OK;
}

template<typename T>
inline API GameObjectBase<T>::GetModelMatrix(OUT mat4 * mat)
{
	mat4 R;
	mat4 T;
	mat4 S;

	const float DEG2RAD = 3.141593f / 180;
	float sx, sy, sz, cx, cy, cz, theta;

	theta = _rot.x * DEG2RAD;
	sx = sinf(theta);
	cx = cosf(theta);

	theta = _rot.y * DEG2RAD;
	sy = sinf(theta);
	cy = cosf(theta);

	theta = _rot.z * DEG2RAD;
	sz = sinf(theta);
	cz = cosf(theta);

	mat4 rx;
	rx.el_2D[1][1] = cx;
	rx.el_2D[1][2] = -sx;
	rx.el_2D[2][1] = sx;
	rx.el_2D[2][2] = cx;

	mat4 ry;
	ry.el_2D[0][0] = cy;
	ry.el_2D[0][2] = sy;
	ry.el_2D[2][0] = -sy;
	ry.el_2D[2][2] = cy;

	R = ry * rx;

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
inline API GameObjectBase<T>::GetPositionEv(OUT IPositionEvent ** pEvent)
{
	*pEvent = _positionEvent.get();
	return S_OK;
}
