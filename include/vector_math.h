#pragma once
#include <cmath> // sqrt

#pragma warning(disable : 4201) // ignore non standard unnamed struct in union

inline constexpr float EPSILON = 1e-5f;
inline constexpr float DEGTORAD = 3.141592654f / 180.0f;
inline constexpr float RADTODEG = 180.0f / 3.141592654f;

struct vec2;
struct vec3;
struct vec4;
struct mat3;
struct mat4;

struct vec4
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		float xyzw[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	vec4() = default;
	vec4(float xIn, float yIn, float zIn, float wIn);
	vec4(const vec3& v3);

	vec4 &operator+=(const vec4 &point);
	vec4 operator+(const vec4 &point) const;

	vec4& operator/=(float value);
	vec4& operator/(float value) const;

	vec4 &operator*=(float value)
	{
		x *= value;
		y *= value;
		z *= value;
		w *= value;
		return *this;
	}

	vec4 operator*(float value) const
	{
		return vec4(*this) *= value;
	}

	float Lenght() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	vec4 Normalized() const
	{
		float len = Lenght();
		return vec4(x / len, y / len, z / len, w / len);
	}

	bool Aproximately(const vec4& r) const
	{
		return std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON && std::abs(r.z - z) < EPSILON && std::abs(r.w - w) < EPSILON;
	}

	operator vec3() const;
	operator vec2() const;
};

struct vec3
{
	union
	{
		struct
		{
			float x, y, z;
		};
		float xyz[3] = {0.0f, 0.0f, 0.0f};
	};

	vec3() = default;
	vec3(float v) : x(v), y(v), z(v) {}
	vec3(float xIn, float yIn, float zIn) : x(xIn), y(yIn), z(zIn) {}
	vec3(const vec4& v4) : x(v4.x), y(v4.y), z(v4.z) {}

	vec3 &operator+=(const vec3 &point)
	{
		x += point.x;
		y += point.y;
		z += point.z;
		return *this;
	}

	vec3 operator+(const vec3 &point) const
	{
		return vec3(*this) += point;
	}

	vec3 &operator-=(const vec3 &point)
	{
		x -= point.x;
		y -= point.y;
		z -= point.z;
		return *this;
	}

	vec3 operator-(const vec3 &point) const
	{
		return vec3(*this) -= point;
	}

	vec3 operator-() const
	{
		return vec3(-x, -y, -z);
	}

	vec3 &operator*=(const vec3 &point)
	{
		x *= point.x;
		y *= point.y;
		z *= point.z;
		return *this;
	}

	vec3 operator*(const vec3 &point) const
	{
		return vec3(*this) *= point;
	}

	vec3 &operator*=(float value)
	{
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	vec3 operator*(float value) const
	{
		return vec3(*this) *= value;
	}

	vec3& operator/=(float value)
	{
		x /= value;
		y /= value;
		z /= value;
		return *this;
	}

	bool operator==(const vec3& v)
	{
		return x == v.x && y == v.y && z == v.z;
	}
	bool operator!=(const vec3& v)
	{
		return x != v.x || y != v.y || z != v.z;
	}

	vec3& operator/(float value) const
	{
		return vec3(*this) /= value;
	}

	vec3 Cross(const vec3 &point) const
	{
		return vec3(y * point.z - z * point.y, z * point.x - x * point.z, x * point.y - y * point.x);
	}

	vec3& Normalize()
	{
		float len = Lenght();
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}

	vec3 Normalized() const
	{
		float len = Lenght();
		return vec3(x / len, y / len, z / len);
	}

	float Dot(const vec3& point) const
	{
		return x * point.x + y*point.y + z*point.z;
	}

	float Lenght() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	bool Aproximately(const vec3& r) const
	{
		return std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON && std::abs(r.z - z) < EPSILON;
	}
};

struct vec2
{
	union
	{
		struct
		{
			float x, y;
		};
		float xy[2] = {0.0f, 0.0f};
	};

	vec2() = default;
	vec2(float v) : x(v), y(v) {}
	vec2(float xIn, float yIn) : x(xIn), y(yIn) {}
	vec2(const vec3& v3) : x(v3.x), y(v3.y) {}

	vec2 &operator+=(const vec2 &point)
	{
		x += point.x;
		y += point.y;
		return *this;
	}

	vec2 operator+(const vec2 &point) const
	{
		return vec2(*this) += point;
	}

	vec2 &operator-=(const vec2 &point)
	{
		x -= point.x;
		y -= point.y;
		return *this;
	}

	vec2 operator-(const vec2 &point) const
	{
		return vec2(*this) -= point;
	}

	vec2 operator-() const
	{
		return vec2(-x, -y);
	}

	vec2 &operator*=(const vec2 &point)
	{
		x *= point.x;
		y *= point.y;
		return *this;
	}

	vec2 operator*(const vec2 &point) const
	{
		return vec2(*this) *= point;
	}

	vec2 &operator*=(float value)
	{
		x *= value;
		y *= value;
		return *this;
	}

	vec2 operator*(float value) const
	{
		return vec2(*this) *= value;
	}

	vec2& operator/=(float value)
	{
		x /= value;
		y /= value;
		return *this;
	}

	vec2& operator/(float value) const
	{
		return vec2(*this) /= value;
	}
	
	vec2& Normalize()
	{
		float len = Lenght();
		x /= len;
		y /= len;
		return *this;
	}

	vec2 Normalized() const
	{
		float len = Lenght();
		return vec2(x / len, y / len);
	}

	float Dot(const vec2& point) const
	{
		return x * point.x + y*point.y;
	}

	float Lenght() const
	{
		return sqrt(x * x + y * y);
	}

	bool Aproximately(const vec2& r) const
	{
		return std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON;
	}
};

inline vec4::vec4(float xIn, float yIn, float zIn, float wIn) : x(xIn), y(yIn), z(zIn), w(wIn) {}

inline vec4::vec4(const vec3& v3) : x(v3.x), y(v3.y), z(v3.z), w(1.0f) {}

inline vec4& vec4::operator/=(float value)
{
	x /= value;
	y /= value;
	z /= value;
	w /= value;
	return *this;
}

inline vec4& vec4::operator/(float value) const
{
	return vec4(*this) /= value;
}

inline vec4& vec4::operator+=(const vec4 &point)
{
	x += point.x;
	y += point.y;
	z += point.z;
	w += point.w;
	return *this;
}

inline vec4 vec4::operator+(const vec4 &point) const
{
	return vec4(*this) += point;
}

inline vec4::operator vec3() const
{
	return vec3(x, y, z);
}

inline vec4::operator vec2() const
{
	return vec2(x, y);
}

struct mat4
{
	union
	{
		float el_1D[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		float el_2D[4][4];
	};

	//
	// Identitty matrix
	//
	mat4() = default;

	//
	// Diagonal matrix
	//
	mat4(float f) : mat4()
	{
		el_2D[0][0] = el_2D[1][1] = el_2D[2][2] = f;
		el_2D[3][3] = 1.0f;
	}

	mat4(float(&values)[16])
	{
		for (int i = 0; i < 16; i++)
			el_1D[i] = values[i];
	}

	mat4(float f1, float f2, float f3) : mat4()
	{
		el_2D[0][0] = 1.0f;
		el_2D[1][1] = 1.0f;
		el_2D[2][2] = 1.0f;
		el_2D[3][0] = f1;
		el_2D[3][1] = f2;
		el_2D[3][2] = f3;
	}

	vec4 Column(int i) const
	{
		return vec4(el_2D[0][i], el_2D[1][i], el_2D[2][i], el_2D[3][i]);
	}

	vec3 Column3(int i) const
	{
		return vec3(el_2D[0][i], el_2D[1][i], el_2D[2][i]);
	}

	void SetColumn3(int i, const vec3& vec)
	{
		el_2D[0][i] = vec.x;
		el_2D[1][i] = vec.y;
		el_2D[2][i] = vec.y;
	}

	vec4 operator*(const vec4& v) const
	{
		vec4 ret(0.0f, 0.0f, 0.0f, 0.0f);

		for (auto i = 0; i < 4; i++)
		{
			ret.xyzw[i] = v.x * el_2D[i][0] + v.y * el_2D[i][1] + v.z * el_2D[i][2] + v.w * el_2D[i][3];
		}

		return ret;
	}

	mat4 operator*(const mat4& m) const
	{
		mat4 res;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				res.el_2D[i][j] =
					el_2D[i][0] * m.el_2D[0][j] +
					el_2D[i][1] * m.el_2D[1][j] +
					el_2D[i][2] * m.el_2D[2][j] +
					el_2D[i][3] * m.el_2D[3][j];
			}
		}
		return res;
	}

	mat4 Inverse() const
	{
		// TODO: rewrite

		float inv[16], invOut[16], det;

		inv[0] = el_1D[5] * el_1D[10] * el_1D[15] -
			el_1D[5] * el_1D[11] * el_1D[14] -
			el_1D[9] * el_1D[6] * el_1D[15] +
			el_1D[9] * el_1D[7] * el_1D[14] +
			el_1D[13] * el_1D[6] * el_1D[11] -
			el_1D[13] * el_1D[7] * el_1D[10];

		inv[4] = -el_1D[4] * el_1D[10] * el_1D[15] +
			el_1D[4] * el_1D[11] * el_1D[14] +
			el_1D[8] * el_1D[6] * el_1D[15] -
			el_1D[8] * el_1D[7] * el_1D[14] -
			el_1D[12] * el_1D[6] * el_1D[11] +
			el_1D[12] * el_1D[7] * el_1D[10];

		inv[8] = el_1D[4] * el_1D[9] * el_1D[15] -
			el_1D[4] * el_1D[11] * el_1D[13] -
			el_1D[8] * el_1D[5] * el_1D[15] +
			el_1D[8] * el_1D[7] * el_1D[13] +
			el_1D[12] * el_1D[5] * el_1D[11] -
			el_1D[12] * el_1D[7] * el_1D[9];

		inv[12] = -el_1D[4] * el_1D[9] * el_1D[14] +
			el_1D[4] * el_1D[10] * el_1D[13] +
			el_1D[8] * el_1D[5] * el_1D[14] -
			el_1D[8] * el_1D[6] * el_1D[13] -
			el_1D[12] * el_1D[5] * el_1D[10] +
			el_1D[12] * el_1D[6] * el_1D[9];

		inv[1] = -el_1D[1] * el_1D[10] * el_1D[15] +
			el_1D[1] * el_1D[11] * el_1D[14] +
			el_1D[9] * el_1D[2] * el_1D[15] -
			el_1D[9] * el_1D[3] * el_1D[14] -
			el_1D[13] * el_1D[2] * el_1D[11] +
			el_1D[13] * el_1D[3] * el_1D[10];

		inv[5] = el_1D[0] * el_1D[10] * el_1D[15] -
			el_1D[0] * el_1D[11] * el_1D[14] -
			el_1D[8] * el_1D[2] * el_1D[15] +
			el_1D[8] * el_1D[3] * el_1D[14] +
			el_1D[12] * el_1D[2] * el_1D[11] -
			el_1D[12] * el_1D[3] * el_1D[10];

		inv[9] = -el_1D[0] * el_1D[9] * el_1D[15] +
			el_1D[0] * el_1D[11] * el_1D[13] +
			el_1D[8] * el_1D[1] * el_1D[15] -
			el_1D[8] * el_1D[3] * el_1D[13] -
			el_1D[12] * el_1D[1] * el_1D[11] +
			el_1D[12] * el_1D[3] * el_1D[9];

		inv[13] = el_1D[0] * el_1D[9] * el_1D[14] -
			el_1D[0] * el_1D[10] * el_1D[13] -
			el_1D[8] * el_1D[1] * el_1D[14] +
			el_1D[8] * el_1D[2] * el_1D[13] +
			el_1D[12] * el_1D[1] * el_1D[10] -
			el_1D[12] * el_1D[2] * el_1D[9];

		inv[2] = el_1D[1] * el_1D[6] * el_1D[15] -
			el_1D[1] * el_1D[7] * el_1D[14] -
			el_1D[5] * el_1D[2] * el_1D[15] +
			el_1D[5] * el_1D[3] * el_1D[14] +
			el_1D[13] * el_1D[2] * el_1D[7] -
			el_1D[13] * el_1D[3] * el_1D[6];

		inv[6] = -el_1D[0] * el_1D[6] * el_1D[15] +
			el_1D[0] * el_1D[7] * el_1D[14] +
			el_1D[4] * el_1D[2] * el_1D[15] -
			el_1D[4] * el_1D[3] * el_1D[14] -
			el_1D[12] * el_1D[2] * el_1D[7] +
			el_1D[12] * el_1D[3] * el_1D[6];

		inv[10] = el_1D[0] * el_1D[5] * el_1D[15] -
			el_1D[0] * el_1D[7] * el_1D[13] -
			el_1D[4] * el_1D[1] * el_1D[15] +
			el_1D[4] * el_1D[3] * el_1D[13] +
			el_1D[12] * el_1D[1] * el_1D[7] -
			el_1D[12] * el_1D[3] * el_1D[5];

		inv[14] = -el_1D[0] * el_1D[5] * el_1D[14] +
			el_1D[0] * el_1D[6] * el_1D[13] +
			el_1D[4] * el_1D[1] * el_1D[14] -
			el_1D[4] * el_1D[2] * el_1D[13] -
			el_1D[12] * el_1D[1] * el_1D[6] +
			el_1D[12] * el_1D[2] * el_1D[5];

		inv[3] = -el_1D[1] * el_1D[6] * el_1D[11] +
			el_1D[1] * el_1D[7] * el_1D[10] +
			el_1D[5] * el_1D[2] * el_1D[11] -
			el_1D[5] * el_1D[3] * el_1D[10] -
			el_1D[9] * el_1D[2] * el_1D[7] +
			el_1D[9] * el_1D[3] * el_1D[6];

		inv[7] = el_1D[0] * el_1D[6] * el_1D[11] -
			el_1D[0] * el_1D[7] * el_1D[10] -
			el_1D[4] * el_1D[2] * el_1D[11] +
			el_1D[4] * el_1D[3] * el_1D[10] +
			el_1D[8] * el_1D[2] * el_1D[7] -
			el_1D[8] * el_1D[3] * el_1D[6];

		inv[11] = -el_1D[0] * el_1D[5] * el_1D[11] +
			el_1D[0] * el_1D[7] * el_1D[9] +
			el_1D[4] * el_1D[1] * el_1D[11] -
			el_1D[4] * el_1D[3] * el_1D[9] -
			el_1D[8] * el_1D[1] * el_1D[7] +
			el_1D[8] * el_1D[3] * el_1D[5];

		inv[15] = el_1D[0] * el_1D[5] * el_1D[10] -
			el_1D[0] * el_1D[6] * el_1D[9] -
			el_1D[4] * el_1D[1] * el_1D[10] +
			el_1D[4] * el_1D[2] * el_1D[9] +
			el_1D[8] * el_1D[1] * el_1D[6] -
			el_1D[8] * el_1D[2] * el_1D[5];

		det = el_1D[0] * inv[0] + el_1D[1] * inv[4] + el_1D[2] * inv[8] + el_1D[3] * inv[12];

		det = 1.0f / det;

		for (int i = 0; i < 16; i++)
			invOut[i] = inv[i] * det;

		return mat4(invOut);
	}

	mat4 Transpose()
	{
		mat4 ret(*this);

		el_2D[0][1] = ret.el_2D[1][0];
		el_2D[0][2] = ret.el_2D[2][0];
		el_2D[0][3] = ret.el_2D[3][0];
		el_2D[1][0] = ret.el_2D[0][1];
		el_2D[1][2] = ret.el_2D[2][1];
		el_2D[1][3] = ret.el_2D[3][1];
		el_2D[2][0] = ret.el_2D[0][2];
		el_2D[2][1] = ret.el_2D[1][2];
		el_2D[2][3] = ret.el_2D[3][2];
		el_2D[3][0] = ret.el_2D[0][3];
		el_2D[3][1] = ret.el_2D[1][3];
		el_2D[3][2] = ret.el_2D[2][3];

		return *this;
	}
};


struct mat3
{
	union
	{
		float el_1D[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		float el_2D[3][3];
	};

	//
	// Identity matrix
	//
	mat3() = default;

	//
	// Diagonal matrix
	//
	mat3(float f)
	{
		el_2D[0][0] = el_2D[0][1] = el_2D[0][2] = 0.0f;
		el_2D[1][0] = el_2D[1][1] = el_2D[1][2] = 0.0f;
		el_2D[2][0] = el_2D[2][1] = el_2D[2][2] = 0.0f;
		el_2D[0][0] = el_2D[1][1] = el_2D[2][2] = f;
	}

	mat3(float f1, float f2, float f3)
	{
		el_2D[0][0] = el_2D[0][1] = el_2D[0][2] = 0.0f;
		el_2D[1][0] = el_2D[1][1] = el_2D[1][2] = 0.0f;
		el_2D[2][0] = el_2D[2][1] = el_2D[2][2] = 0.0f;
		el_2D[0][0] = f1;
		el_2D[1][1] = f2;
		el_2D[2][2] = f3;
	}

	mat3(const mat4& m4)
	{
		el_2D[0][0] = m4.el_2D[0][0];
		el_2D[0][1] = m4.el_2D[0][1];
		el_2D[0][2] = m4.el_2D[0][2];
		el_2D[1][0] = m4.el_2D[1][0];
		el_2D[1][1] = m4.el_2D[1][1];
		el_2D[1][2] = m4.el_2D[1][2];
		el_2D[2][0] = m4.el_2D[2][0];
		el_2D[2][1] = m4.el_2D[2][1];
		el_2D[2][2] = m4.el_2D[2][2];
	}

	mat3& Inverse()
	{
		mat3 tmp(*this);
		float inv_det = 1.0f / Det();
		el_2D[0][0] = tmp.el_2D[1][1] * tmp.el_2D[2][2] - tmp.el_2D[1][2] * tmp.el_2D[2][1];
		el_2D[0][1] = tmp.el_2D[0][2] * tmp.el_2D[2][1] - tmp.el_2D[0][1] * tmp.el_2D[2][2];
		el_2D[0][2] = tmp.el_2D[0][1] * tmp.el_2D[1][2] - tmp.el_2D[0][2] * tmp.el_2D[1][1];
		el_2D[1][0] = tmp.el_2D[1][2] * tmp.el_2D[2][0] - tmp.el_2D[1][0] * tmp.el_2D[2][2];
		el_2D[1][1] = tmp.el_2D[0][0] * tmp.el_2D[2][2] - tmp.el_2D[0][2] * tmp.el_2D[2][0];
		el_2D[1][2] = tmp.el_2D[0][2] * tmp.el_2D[1][0] - tmp.el_2D[0][0] * tmp.el_2D[1][2];
		el_2D[2][0] = tmp.el_2D[1][0] * tmp.el_2D[2][1] - tmp.el_2D[1][1] * tmp.el_2D[2][0];
		el_2D[2][1] = tmp.el_2D[0][0] * tmp.el_2D[1][2] - tmp.el_2D[1][0] * tmp.el_2D[0][2];
		el_2D[2][2] = tmp.el_2D[0][0] * tmp.el_2D[1][1] - tmp.el_2D[0][1] * tmp.el_2D[1][0];
		*this *= inv_det;
		return *this;
	}

	mat3 Inversed() const
	{
		mat3 ret(*this);
		ret.Inverse();
		return ret;
	}

	mat3& Transpose()
	{
		mat3 tmp(*this);
		el_2D[0][1] = tmp.el_2D[1][0];
		el_2D[0][2] = tmp.el_2D[2][0];
		el_2D[1][0] = tmp.el_2D[0][1];
		el_2D[1][2] = tmp.el_2D[2][1];
		el_2D[2][0] = tmp.el_2D[0][2];
		el_2D[2][1] = tmp.el_2D[1][2];
		return *this;
	}

	mat3 Transposed() const
	{
		mat3 ret(*this);
		ret.Transpose();
		return ret;
	}

	float Det() const
	{
		return
			el_2D[0][0] * el_2D[1][1] * el_2D[2][2] -
			el_2D[0][0] * el_2D[1][2] * el_2D[2][1] -
			el_2D[0][1] * el_2D[1][0] * el_2D[2][2] +
			el_2D[0][1] * el_2D[1][2] * el_2D[2][0] +
			el_2D[0][2] * el_2D[1][0] * el_2D[2][1] -
			el_2D[0][2] * el_2D[1][1] * el_2D[2][0];
	}

	mat3& operator*=(float f)
	{
		for (auto& e : el_1D) e *= f;
		return *this;
	}

	mat3 operator*(float f) const
	{
		return (mat3(*this) *= f);
	}

	vec3 operator*(const vec3& v) const
	{
		vec3 ret;
		for (auto i = 0; i < 3; i++)
		{
			ret.xyz[i] = el_2D[i][0] * v.x + el_2D[i][1] * v.y + el_2D[i][2] * v.z;
		}
		return ret;
	}
};


////////////////////////
// Quaternion
////////////////////////

struct quat
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct
		{
			vec3 v;
			float _w;
		};
		float xyzw[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	//
	// Identity rotation
	//
	quat()
		: x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

	//
	// Rotation form x, y, z, w
	//
	quat(float xIn, float yIn, float zIn, float wIn)
		: x(xIn), y(yIn), z(zIn), w(wIn) {}

	//
	// Creates rotation from Euler Angles
	// Rotation Order:	X -> Y -> Z around local axes, i.e
	//					Z -> Y -> X around global axes
	//
	quat(float xAngle, float yAngle, float zAngle)
	{
		float a = DEGTORAD * xAngle / 2;
		float b = DEGTORAD * yAngle / 2;
		float g = DEGTORAD * zAngle / 2;

		z = cos(-b)*sin(g)*cos(a) - sin(-b)*cos(g)*sin(a);
		y = -cos(-b)*sin(g)*sin(a) - sin(-b)*cos(g)*cos(a);
		x = cos(-b)*cos(g)*sin(a) - sin(-b)*sin(g)*cos(a);
		w = cos(-b)*cos(g)*cos(a) + sin(-b)*sin(g)*sin(a);
	}

	quat(const vec3& euler) : quat(euler.x, euler.y, euler.z) {}

	//
	// Creates rotation from Rotation Matrix
	//
	quat(const mat3& rotMat)
	{
		float tr = rotMat.el_2D[0][0] + rotMat.el_2D[1][1] + rotMat.el_2D[2][2];

		if (tr > 0.0f)
		{
			float S = sqrt(tr+1.0f) * 2.f;
			w = 0.25f * S;
			x = (rotMat.el_2D[2][1] - rotMat.el_2D[1][2]) / S;
			y = (rotMat.el_2D[0][2] - rotMat.el_2D[2][0]) / S; 
			z = (rotMat.el_2D[1][0] - rotMat.el_2D[0][1]) / S; 
		} else if ((rotMat.el_2D[0][0] > rotMat.el_2D[1][1])&&(rotMat.el_2D[0][0] > rotMat.el_2D[2][2]))
		{
			float S = sqrt(1.0f + rotMat.el_2D[0][0] - rotMat.el_2D[1][1] - rotMat.el_2D[2][2]) * 2.f;
			w = (rotMat.el_2D[2][1] - rotMat.el_2D[1][2]) / S;
			x = 0.25f * S;
			y = (rotMat.el_2D[0][1] + rotMat.el_2D[1][0]) / S; 
			z = (rotMat.el_2D[0][2] + rotMat.el_2D[2][0]) / S; 
		} else if (rotMat.el_2D[1][1] > rotMat.el_2D[2][2])
		{
			float S = sqrt(1.0f + rotMat.el_2D[1][1] - rotMat.el_2D[0][0] - rotMat.el_2D[2][2]) * 2.f;
			w = (rotMat.el_2D[0][2] - rotMat.el_2D[2][0]) / S;
			x = (rotMat.el_2D[0][1] + rotMat.el_2D[1][0]) / S; 
			y = 0.25f * S;
			z = (rotMat.el_2D[1][2] + rotMat.el_2D[2][1]) / S; 
		} else
		{
			float S = sqrt(1.0f + rotMat.el_2D[2][2] - rotMat.el_2D[0][0] - rotMat.el_2D[1][1]) * 2.f;
			w = (rotMat.el_2D[1][0] - rotMat.el_2D[0][1]) / S;
			x = (rotMat.el_2D[0][2] + rotMat.el_2D[2][0]) / S;
			y = (rotMat.el_2D[1][2] + rotMat.el_2D[2][1]) / S;
			z = 0.25f * S;
		}
	}

	//
	// Creates rotation from Rotation Matrix
	//
	quat(const mat4& rotMat) : quat(mat3(rotMat)) {}

	//
	// Converts to Euler Angles
	// Rotation Order:	X -> Y -> Z around local axes, i.e
	//					Z -> Y -> X around global axes
	//
	vec3 ToEuler() const
	{
		mat4 M = ToMatrix();

		float alpha_rad, beta_rad, gamma_rad;

		float cy = sqrt(M.el_2D[2][2] * M.el_2D[2][2] + M.el_2D[1][2] * M.el_2D[1][2]);

		if (cy > 16 * 1.19e-07)
		{
			gamma_rad = -atan2(M.el_2D[0][1], M.el_2D[0][0]);
			beta_rad = -atan2(-M.el_2D[0][2], cy);
			alpha_rad = -atan2(M.el_2D[1][2], M.el_2D[2][2]);
		}
		else
		{
			gamma_rad = -atan2(-M.el_2D[1][0], M.el_2D[1][1]);
			beta_rad = -atan2(-M.el_2D[0][2], cy);
			alpha_rad = 0;
		}

		return vec3(alpha_rad * RADTODEG, beta_rad * RADTODEG, gamma_rad *RADTODEG);
	}

	//
	// Converts to Rotation Matrix 4x4
	//
	mat4 ToMatrix() const
	{
		float Nq = x * x + y * y + z * z + w * w;
		float s;

		if (Nq > 0.0f)
			s = 2.0f / Nq;
		else
			s = 0.0f;

		float xs = x * s;  float ys = y * s;  float zs = z * s;
		float wx = w * xs; float wy = w * ys; float wz = w * zs;
		float xx = x * xs; float xy = x * ys; float xz = x * zs;
		float yy = y * ys; float yz = y * zs; float zz = z * zs;

		mat4 ret;
		ret.el_2D[0][0] = 1.0f - (yy + zz); ret.el_2D[0][1] = xy - wz;          ret.el_2D[0][2] = xz + wy;           ret.el_2D[0][3] = 0.0f;
		ret.el_2D[1][0] = xy + wz;          ret.el_2D[1][1] = 1.0f - (xx + zz); ret.el_2D[1][2] = yz - wx;           ret.el_2D[1][3] = 0.0f;
		ret.el_2D[2][0] = xz - wy;          ret.el_2D[2][1] = yz + wx;          ret.el_2D[2][2] = 1.0f - (xx + yy);  ret.el_2D[2][3] = 0.0f;
		ret.el_2D[3][0] = 0.0f;             ret.el_2D[3][1] = 0.0f;             ret.el_2D[3][2] = 0.0f;              ret.el_2D[3][3] = 1.0f;

		return ret;
	}

	quat& operator=(const quat& q)
	{
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
		return *this;
	}

	quat operator+(const quat& q) const
	{
		return quat(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	quat operator*(const quat& q) const
	{
		quat p;
		p.w = w * q.w - x * q.x - y * q.y - z * q.z;
		p.x = w * q.x + x * q.w + y * q.z - z * q.y;
		p.y = w * q.y + y * q.w + z * q.x - x * q.z;
		p.z = w * q.z + z * q.w + x * q.y - y * q.x;
		return p;
	}

	quat operator-() const
	{
		quat p;
		p.x = -x;
		p.y = -y;
		p.z = -z;
		p.w = -w;
		return p;
	}

	quat Conjugated() const
	{
		return quat(-v.x, -v.y, -v.z, w);
	}

	float Norm() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	void Normalize()
	{
		float n = Norm();
		x /= n;
		y /= n;
		z /= n;
		w /= n;
	}

	bool Aproximately(const quat& r) const
	{
		return std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON && std::abs(r.z - z) < EPSILON && std::abs(r.w - w) < EPSILON;
	}

	bool IsSameRotation(const quat& r) const
	{
		bool f1 = std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON && std::abs(r.z - z) < EPSILON && std::abs(r.w - w) < EPSILON;
		if (f1) return true;
		quat q = -r;
		return std::abs(q.x - x) < EPSILON && std::abs(q.y - y) < EPSILON && std::abs(q.z - z) < EPSILON && std::abs(q.w - w) < EPSILON;
	}
};

inline float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T>
inline T max(T a, T b)
{
	return a > b ? a : b;
}

template <class T>
inline T min(T a, T b)
{
	return a > b ? b : a;
}

inline vec3 normalize(const vec3& v)
{
	float len = v.Lenght();
	return v / len;
}

inline vec3 cross(const vec3& point1 ,const vec3& point)
{
	return vec3(point1.y * point.z - point1.z * point.y, point1.z * point.x - point1.x * point.z, point1.x * point.y - point1.y * point.x);
}

inline float saturate(float a)
{
	return max(min(a, 1.0f), 0.0f);
}

#pragma warning(default : 4201)
