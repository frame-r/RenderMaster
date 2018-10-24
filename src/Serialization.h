#pragma once
#include "yaml-cpp/yaml.h"

class SceneManager;

void readSceneManager(YAML::Node& n, SceneManager &sm);
YAML::Emitter& operator<<(YAML::Emitter& out, const SceneManager& sm);

YAML::Emitter& operator<<(YAML::Emitter& out, IResource* g);
