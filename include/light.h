#pragma once
#include "gameobject.h"


class Light : public GameObject
{
	float intensity_{1.0f};

protected:
	virtual void Copy(GameObject *original) override;
	virtual void SaveYAML(void *yaml) override;
	virtual void LoadYAML(void *yaml) override;

public:
	Light();

	auto DLLEXPORT virtual GetIntensity() -> float { return intensity_; }
	auto DLLEXPORT virtual SetIntensity(float v) -> void { intensity_ = v; }

	// GameObject
	auto DLLEXPORT virtual Clone() -> GameObject* override;
};
