#pragma once
#include "Common.h"
#include "Events.h"

#define IGAMEOBJECT_IMPLEMENTATION \
private: \
\
std::unique_ptr<PositionEvent> _posEv{new PositionEvent}; \
\
std::string _name{"GameObject"}; \
\
vec3 _pos{0.0f, 0.0f, 0.0f}; \
vec3 _rot{0.0f, 0.0f, 0.0f}; \
vec3 _scale{1.0f, 1.0f, 1.0f}; \
\
public: \
\
API GetName(OUT const char **pName) override \
{ \
	*pName = _name.c_str(); \
	return S_OK; \
} \
\
API SetPosition(const vec3 *pos) override \
{ \
	if (!_pos.Aproximately(*pos)) \
	{ \
		_pos = *pos; \
		_posEv->Fire(&_pos); \
	} \
	return S_OK; \
} \
API SetRotation(const vec3 *rot) override \
{ \
	_rot = *rot; \
	_rot.x = fmod(_rot.x, 360.0f); \
	_rot.y = fmod(_rot.y, 360.0f); \
	_rot.z = fmod(_rot.z, 360.0f); \
	return S_OK; \
} \
\
API GetPosition(OUT vec3 *pos) override \
{ \
	*pos = _pos; \
	return S_OK; \
} \
\
API GetRotation(OUT vec3 *rot) override \
{ \
	*rot = _rot; \
	return S_OK; \
} \
\
API GetModelMatrix(OUT mat4 *mat) override \
{ \
	mat4 R; \
	mat4 T; \
	mat4 S; \
\
	const float DEG2RAD = 3.141593f / 180; \
	float sx, sy, sz, cx, cy, cz, theta; \
\
	theta = _rot.x * DEG2RAD; \
	sx = sinf(theta); \
	cx = cosf(theta); \
\
	theta = _rot.y * DEG2RAD; \
	sy = sinf(theta); \
	cy = cosf(theta); \
\
	theta = _rot.z * DEG2RAD; \
	sz = sinf(theta); \
	cz = cosf(theta); \
\
	mat4 rx; \
	rx.el_2D[1][1] = cx; \
	rx.el_2D[1][2] = -sx; \
	rx.el_2D[2][1] = sx; \
	rx.el_2D[2][2] = cx; \
\
	mat4 ry; \
	ry.el_2D[0][0] = cy; \
	ry.el_2D[0][2] = sy; \
	ry.el_2D[2][0] = -sy; \
	ry.el_2D[2][2] = cy; \
\
	R = ry * rx; \
\
	T.el_2D[0][3] = _pos.x; \
	T.el_2D[1][3] = _pos.y; \
	T.el_2D[2][3] = _pos.z; \
\
	S.el_2D[0][0] = _scale.x; \
	S.el_2D[1][1] = _scale.y; \
	S.el_2D[2][2] = _scale.z; \
\
	*mat =  T * R * S; \
\
	return S_OK; \
} \
\
API GetPositionEv(OUT IPositionEvent **pEvent) override \
{ \
		*pEvent = _posEv.get(); \
		return S_OK; \
}

