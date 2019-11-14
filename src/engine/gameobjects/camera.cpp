#include "pch.h"
#include "camera.h"
#include "console.h"
#include "core.h"
#include "input.h"
#include "yaml.inl"

#define MOVE_SPEED 30.f
#define ROTATE_SPEED 0.2f

void Camera::Update(float dt)
{
	GameObject::Update(dt);

	int left_pressd = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_A);
	int right_pressd = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_D);
	int forward_pressd = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_W);
	int back_pressd = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_S);
	int down_pressed = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_Q);
	int up_pressed = INPUT->IsKeyPressed(KEYBOARD_KEY_CODES::KEY_E);
	int mouse_pressed = INPUT->IsMoisePressed(MOUSE_BUTTON::LEFT);
	vec2 dM = INPUT->GetMouseDeltaPos();

	mat4 M = GetWorldTransform();

	{
		// directions in world space
		vec3 orth_direction = GetRightDirection(M); // X local
		vec3 forward_direction = GetBackDirection(M); // -Z local
		vec3 up_direction = vec3(0.0f, 0.0f, 1.0f); // Z world
		
		vec3 pos = GetWorldPosition();
		vec3 new_pos = pos;

		if (left_pressd)
			new_pos -= orth_direction * MOVE_SPEED * dt;

		if (right_pressd)
			new_pos += orth_direction * MOVE_SPEED * dt;

		if (forward_pressd)
			new_pos += forward_direction * MOVE_SPEED * dt;

		if (back_pressd)
			new_pos -= forward_direction * MOVE_SPEED * dt;

		if (down_pressed)
			new_pos -= up_direction * MOVE_SPEED * dt;

		if (up_pressed)
			new_pos += up_direction * MOVE_SPEED  * dt;

		if (new_pos != pos)
			SetWorldPosition(new_pos);
	}

	if (mouse_pressed)
	{
		//Log("dt=%f dm=(%f, %f)", LOG_TYPE::NORMAL, dt, dM.x, dM.y);

		quat rot = GetLocalRotation();

		quat dxRot = quat(-dM.y * ROTATE_SPEED, 0.0f, 0.0f);
		quat dyRot = quat(0.0f, 0.0f,-dM.x  * ROTATE_SPEED);

		rot = dyRot * rot * dxRot;

		// TODO: SetWorldRotation
		SetLocalRotation(rot);
	}
}

void Camera::SaveYAML(void *yaml)
{
	GameObject::SaveYAML(yaml);

	YAML::Emitter *_n = static_cast<YAML::Emitter*>(yaml);
	YAML::Emitter& n = *_n;

	n << YAML::Key << "zNear" << YAML::Value << zNear_;
	n << YAML::Key << "zFar" << YAML::Value << zFar_;
	n << YAML::Key << "fovAngle" << YAML::Value << fovAngle_;
}

void Camera::LoadYAML(void *yaml)
{
	GameObject::LoadYAML(yaml);

	YAML::Node *_n = static_cast<YAML::Node*>(yaml);
	YAML::Node& n = *_n;

	if (n["zNear"]) zNear_ = n["zNear"].as<float>();
	if (n["zFar"]) zFar_ = n["zFar"].as<float>();
	if (n["fovAngle"]) fovAngle_ = n["fovAngle"].as<float>();
}

void Camera::Copy(GameObject * original)
{
	GameObject::Copy(original);
	Camera *original_cam = static_cast<Camera*>(original);
	fovAngle_ = original_cam->fovAngle_;
	zFar_ = original_cam->zFar_;
	zNear_ = original_cam->zNear_;
}

Camera::Camera()
{
	type_ = OBJECT_TYPE::CAMERA;
	SetWorldPosition(vec3(0.0f, -15.0f, 5.0f));
	SetLocalRotation(quat(80.0f, 0.0f, 0.0f));
}

auto DLLEXPORT Camera::Clone() -> GameObject *
{
	Camera *l = new Camera;
	l->Copy(this);
	return l;
}

auto DLLEXPORT Camera::GetViewMatrix() -> mat4
{
	mat4 mat = GetInvWorldTransform();
	return mat;
}

auto DLLEXPORT Camera::GetViewProjectionMatrix(float aspect) -> mat4
{
	mat4 P = perspectiveRH_ZO(fovAngle_ * DEGTORAD, aspect, zNear_, zFar_);
	mat4 V = GetInvWorldTransform();
	return P * V;
}

auto DLLEXPORT Camera::GetProjectionMatrix(float aspect) -> mat4
{
	return perspectiveRH_ZO(fovAngle_ * DEGTORAD, aspect, zNear_, zFar_);
}

auto DLLEXPORT Camera::GetFovAngle() -> float
{
	return fovAngle_;
}

