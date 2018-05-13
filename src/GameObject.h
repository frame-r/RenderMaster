#pragma once
#include "Common.h"

#define IGAMEOBJECT_IMPLEMENTATION \
private: \
\
vec3 _pos{0.0f, 0.0f, 0.0f}; \
vec3 _rot{0.0f, 0.0f, 0.0f}; \
vec3 _scale{1.0f, 1.0f, 1.0f}; \
\
public: \
\
API SetPosition(const vec3 *pos) override \
{ \
	_pos = *pos; \
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
}

