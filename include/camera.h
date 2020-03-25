#pragma once
#include "gameobject.h"


class Camera : public GameObject
{
	float zNear_{0.1f};
	float zFar_{1000.0f};
	float fovAngle_{60.0f};

protected:
	virtual void Copy(GameObject *original) override;
	virtual void SaveYAML(void *yaml) override;
	virtual void LoadYAML(void *yaml) override;

public:
	Camera();

	virtual void Update(float dt);

	auto DLLEXPORT GetViewMatrix() -> mat4;
	auto DLLEXPORT GetViewProjectionMatrix(float aspect) -> mat4;
	auto DLLEXPORT GetProjectionMatrix(float aspect) -> mat4;
	auto DLLEXPORT GetFovAngle() -> float;

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
