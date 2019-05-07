#pragma once
#include "gameobject.h"

class Camera : public GameObject
{
	float zNear_{0.1f};
	float zFar_{1000.0f};
	float fovAngle_{60.0f};

protected:
	virtual void Copy(GameObject *original) override;

public:
	Camera();
	virtual ~Camera(){}

	virtual void Update(float dt);
	virtual void Serialize(void *yaml) override;
	virtual void Deserialize(void *yaml) override;

public:
	auto DLLEXPORT GetViewMatrix() -> mat4;
	auto DLLEXPORT GetViewProjectionMatrix(float aspect) -> mat4;
	auto DLLEXPORT GetProjectionMatrix(float aspect) -> mat4;
	auto DLLEXPORT GetFovAngle() -> float;

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
