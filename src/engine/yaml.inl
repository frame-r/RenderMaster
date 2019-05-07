#include "yaml-cpp/yaml.h"

inline YAML::Emitter& operator<<(YAML::Emitter& out, const vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const quat& q)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
	return out;
}
inline YAML::Emitter& operator<<(YAML::Emitter& out, const mat4& m)
{
	out << YAML::Flow;
	out << YAML::BeginSeq;
	for (int i = 0; i<16; i++)
		out << m.el_1D[i];
	out << YAML::EndSeq;
	return out;
}

inline void loadMat4(YAML::Node& n, mat4& m)
{
	for (int i = 0; i < 16; i++)
		m.el_1D[i] = n[i].as<float>();
}
inline void loadVec4(const YAML::Node& node, const char *name, vec4& m)
{
	if (node[name])
	{
		YAML::Node n = node[name];
		for (int i = 0; i < 4; i++)
			m.xyzw[i] = n[i].as<float>();
	}
}
