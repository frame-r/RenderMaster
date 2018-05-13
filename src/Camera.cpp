#include "Camera.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

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

	vec3 orth_direction = vec3(M * vec4(1.0f, 0.0f, 0.0f, 0.0f));
	vec3 forward_direction = vec3(M * vec4(0.0f, 0.0f, 1.0f, 0.0f));
	vec3 up_direction = vec3(0.0f, 1.0f, 0.0f);

	const float speed = 15.3f;

	if (left_pressd)
		_pos -= orth_direction * speed;

	if (right_pressd)
		_pos += orth_direction * speed;

	if (forward_pressd)
		_pos += forward_direction * speed;

	if (back_pressd)
		_pos -= forward_direction * speed;

	if (down_pressed)
		_pos -= up_direction * speed;

	if (up_pressed)
		_pos += up_direction * speed;

	if (mouse_pressed)
	{
		//LOG_FORMATTED("_rot=(%f, %f)", _rot.x, _rot.y);
		_rot.y = fmod(_rot.y + 0.1f * dM.x, 360.0f);
		_rot.x = fmod(_rot.x + 0.1f * dM.y, 360.0f);
	}

}

Camera::Camera()
{
	_pCore->AddUpdateCallback(std::bind(&Camera::_update, this));
	_pCore->GetSubSystem((ISubSystem**)&_pInput, SUBSYSTEM_TYPE::INPUT);

	_rot = vec3(25.0f, -22.4f, 0.0f);
	_pos = vec3(9.37f, 9.61f, -14.27f);
}

API Camera::GetViewProjectionMatrix(OUT mat4 *mat, float aspect)
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

	mat4 M;
	GetModelMatrix(&M);

	vec3 forward = vec3(M * vec4(0.0f, 0.0f, 1.0f, 0.0f));

	look_at(V, _pos, _pos + forward);

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

	*mat = P * V;

	return S_OK;
}

API Camera::Free()
{
	standard_free_and_delete(this, std::function<void()>(), _pCore);

	return S_OK;
}

API Camera::GetType(OUT RES_TYPE *type)
{
	*type = RES_TYPE::CAMERA;
	return S_OK;
}
