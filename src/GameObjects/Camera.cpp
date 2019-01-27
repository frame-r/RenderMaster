#include "Pch.h"
#include "Common.h"
#include "Camera.h"
#include "Core.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(Camera, _pCore, RemoveRuntimeGameObject)

void Camera::_update()
{
	int left_pressd{};
	int right_pressd{};
	int forward_pressd{};
	int back_pressd{};
	int down_pressed{};
	int up_pressed{};
	int mouse_pressed{};
	vec2 dM;

	float dt = _pCore->deltaTime();

	_pInput->IsKeyPressed(&left_pressd, KEYBOARD_KEY_CODES::KEY_A);
	_pInput->IsKeyPressed(&right_pressd, KEYBOARD_KEY_CODES::KEY_D);
	_pInput->IsKeyPressed(&forward_pressd, KEYBOARD_KEY_CODES::KEY_W);
	_pInput->IsKeyPressed(&back_pressd, KEYBOARD_KEY_CODES::KEY_S);
	_pInput->IsKeyPressed(&down_pressed, KEYBOARD_KEY_CODES::KEY_Q);
	_pInput->IsKeyPressed(&up_pressed, KEYBOARD_KEY_CODES::KEY_E);
	_pInput->IsMoisePressed(&mouse_pressed, MOUSE_BUTTON::LEFT);
	_pInput->GetMouseDeltaPos(&dM);

	mat4 M;
	GetModelMatrix(&M);

	{
		// directions in world space
		vec3 orth_direction = GetRightDirection(M); // X local
		vec3 forward_direction = GetBackDirection(M); // -Z local
		vec3 up_direction = vec3(0.0f, 0.0f, 1.0f); // Z world

		const float moveSpeed = 60.0f;

		vec3 pos = _pos;

		if (left_pressd)
			pos -= orth_direction * moveSpeed* dt;

		if (right_pressd)
			pos += orth_direction * moveSpeed* dt;

		if (forward_pressd)
			pos += forward_direction * moveSpeed* dt;

		if (back_pressd)
			pos -= forward_direction * moveSpeed* dt;

		if (down_pressed)
			pos -= up_direction * moveSpeed* dt;

		if (up_pressed)
			pos += up_direction * moveSpeed * dt;
		
		SetPosition(&pos);
	}

	if (mouse_pressed)
	{
		//LOG_FORMATTED("dt=%f dm=(%f, %f)\n",_pCore->getDt(),dM.x,dM.y);
		const float rotSpeed = 13.0f;
		quat rot;
		GetRotation(&rot);
		quat dxRot = quat(-dM.y * dt * rotSpeed, 0.0f, 0.0f);
		quat dyRot = quat(0.0f, 0.0f,-dM.x * dt * rotSpeed);
		rot = dyRot * rot * dxRot;
		SetRotation(&rot);
	}
}

Camera::Camera()
{
	_name = "Camera";

	_pCore->AddUpdateCallback(std::bind(&Camera::_update, this));
	_pCore->GetSubSystem((ISubSystem**)&_pInput, SUBSYSTEM_TYPE::INPUT);

	//_rot = vec3(25.0f, -22.4f, 0.0f);
	//_pos = vec3(9.37f, 9.61f, -14.27f);
	_pos = vec3(10.0f, -25.0f, 5.0f);
	_rot = quat(90.0f, 0.0f, 0.0f);

	vec3 euler = _rot.ToEuler();
	//LOG_FORMATTED("_rot(euler) = (%f, %f, %f)", euler.x, euler.y, euler.z);

}

Camera::~Camera()
{
}

API Camera::GetViewMatrix(OUT mat4 *mat)
{
	GetInvModelMatrix(mat);
	return S_OK;
}

API Camera::GetViewProjectionMatrix(OUT mat4 *mat, float aspect)
{
	mat4 P;

	// OpenGL projection  matrix
	//const float DEGTORAD = 3.1415926f / 180.0f;
	//float const tanHalfFovy = tan(DEGTORAD* _fovAngle / 2);
	//P.el_2D[0][0] = 1.0f / (aspect * tanHalfFovy);
	//P.el_2D[1][1] = 1.0f / (tanHalfFovy);
	//P.el_2D[2][2] = -(_zFar + _zNear) / (_zNear - _zFar);
	//P.el_2D[3][2] = 1.0f;
	//P.el_2D[2][3] = (2.0f * _zFar * _zNear) / (_zNear - _zFar);
	//P.el_2D[3][3] = 0.0f;

	P = perspectiveRH_ZO(_fovAngle * DEGTORAD, aspect, _zNear, _zFar);

	mat4 V;
	GetInvModelMatrix(&V);

	
	vec4 zf(1.0f, 0.0f, -_zFar, 1.0f);
	vec4 zn(1.0f, 0.0f, -_zNear, 1.0f);
	//vec4 zm(1.0f, 0.0f, (_zNear + _zFar) * 0.5f, 1.0f);

	vec4 rf = P * zf;
	rf /= rf.w; // -> 1

	vec4 rn = P * zn;
	rn /= rn.w; // -> 0
/*
	vec4 rm = P * zm;
	rm /= rm.w;

	vec4 or(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 ox (1.0f, 0.0f, 0.0f, 1.0f);
	
	vec4 ze = mat * or;
	ze /= ze.w;

	vec4 z1 = mat * ox;
	z1 /= z1.w;
	*/

	*mat = P * V;

	return S_OK;
}

API Camera::GetProjectionMatrix(OUT mat4 *mat, float aspect)
{
	*mat = perspectiveRH_ZO(_fovAngle * DEGTORAD, aspect, _zNear, _zFar);
	return S_OK;
}

API Camera::GetFovAngle(OUT float *fovInDegrees)
{
	*fovInDegrees = _fovAngle;
	return S_OK;
}

API Camera::Copy(OUT ICamera *copy)
{
	GameObjectBase<ICamera>::Copy(copy);

	Camera *copyCamera = static_cast<Camera*>(copy);
	copyCamera->_zNear = _zNear;
	copyCamera->_zFar = _zFar;
	copyCamera->_fovAngle = _fovAngle;

	return S_OK;
}

