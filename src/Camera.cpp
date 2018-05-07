#include "Camera.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Camera::Camera()
{
}

API Camera::GetViewProjectionMatrix(mat4& mat, float aspect)
{
	mat4 P;
	mat4 V;

	// OpenGL projection  matrix
	const float DEGTORAD = 3.1415926f / 180.0f;
	float const tanHalfFovy = tan(DEGTORAD* _fovAngle / 2);
	P.el_2D[0][0] = 1.0f / (aspect * tanHalfFovy);
	P.el_2D[1][1] = 1.0f / (tanHalfFovy);
	P.el_2D[2][2] = -(_zFar + _zNear) / (_zNear - _zFar);
	P.el_2D[3][2] = 1.0f;
	P.el_2D[2][3] = (2.0f * _zFar * _zNear) / (_zNear - _zFar);
	P.el_2D[3][3] = 0.0f;

	vec3 position(10.5f, 2.0f, -12.5f);
	vec3 forward = position.Normalized() * -1.0f;

	look_at(V, position, position + forward);

	/*
	vec4 zf(1.0f, 0.0f, _zFar, 1.0f);
	vec4 zn(1.0f, 0.0f, _zNear, 1.0f);
	vec4 zm(1.0f, 0.0f, (_zNear + _zFar) * 0.5f, 1.0f);

	vec4 rf = P * zf;
	rf /= rf.w;

	vec4 rn = P * zn;
	rn /= rn.w;

	vec4 rm = P * zm;
	rm /= rm.w;

	vec4 or(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 ox (1.0f, 0.0f, 0.0f, 1.0f);
	
	vec4 ze = mat * or;
	ze /= ze.w;

	vec4 z1 = mat * ox;
	z1 /= z1.w;
	*/

	mat = P * V;

	return S_OK;
}

API Camera::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
		pResMan->RemoveFromList(this);
	else if (refNum > 1)
		pResMan->DecrementRef(this);
	else
		LOG_WARNING("Camera::Free(): refNum == 0");

	delete this;

	return S_OK;
}

API Camera::GetType(RES_TYPE & type)
{
	type = RES_TYPE::CAMERA;
	return S_OK;
}
