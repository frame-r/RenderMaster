#pragma once
#include <cmath> // sqrt

// Don't include this file directly.
// Instead include Engine.h

#pragma warning(disable : 4201) // ignore non standard unnamed struct in union

#define EPSILON 1e-6f
#define DEGTORAD (3.141592654f / 180.0f)
#define RADTODEG (180.0f / 3.141592654f)

struct Vector3;
struct Vector4;
struct Matrix3x3;
struct Matrix4x4;


struct Vector4
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		float xyzw[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	Vector4() = default;
	Vector4(float xIn, float yIn, float zIn, float wIn);
	Vector4(const Vector3& v3);

	Vector4& operator/=(float value);
	Vector4& operator/(float value) const;

	float Lenght() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}
	Vector4 Normalized() const
	{
		float len = Lenght();
		return Vector4(x / len, y / len, z / len, w / len);
	}
};

struct Vector3
{
	union
	{
		struct
		{
			float x, y, z;
		};
		float xyz[3] = {0.0f, 0.0f, 0.0f};
	};

	Vector3() = default;
	Vector3(float v) : x(v), y(v), z(v) {}
	Vector3(float xIn, float yIn, float zIn) : x(xIn), y(yIn), z(zIn) {}
	Vector3(const Vector4& v4) : x(v4.x), y(v4.y), z(v4.z) {}

	//static Vector3 one()		{ return Vector3(1.0f, 1.0f, 1.0f); }
	//static Vector3 zero()		{ return Vector3(0.0f, 0.0f, 0.0f); }
	//static Vector3 forward()	{ return Vector3(0.0f, 1.0f, 0.0f); }
	//static Vector3 back()		{ return Vector3(0.0f, -1.0f, 0.0f); }
	//static Vector3 right()		{ return Vector3(1.0f, 0.0f, 0.0f); }
	//static Vector3 left()		{ return Vector3(-1.0f, 0.0f, 0.0f); }
	//static Vector3 up()			{ return Vector3(0.0f, 0.0f, 1.0f); }
	//static Vector3 down()		{ return Vector3(0.0f, 0.0f, -1.0f); }

	Vector3 &operator+=(const Vector3 &point)
	{
		x += point.x;
		y += point.y;
		z += point.z;
		return *this;
	}

	Vector3 operator+(const Vector3 &point) const
	{
		return Vector3(*this) += point;
	}

	Vector3 &operator-=(const Vector3 &point)
	{
		x -= point.x;
		y -= point.y;
		z -= point.z;
		return *this;
	}

	Vector3 operator-(const Vector3 &point) const
	{
		return Vector3(*this) -= point;
	}

	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 &operator*=(const Vector3 &point)
	{
		x *= point.x;
		y *= point.y;
		z *= point.z;
		return *this;
	}

	Vector3 operator*(const Vector3 &point) const
	{
		return Vector3(*this) *= point;
	}

	Vector3 &operator*=(float value)
	{
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	Vector3 operator*(float value) const
	{
		return Vector3(*this) *= value;
	}

	Vector3& operator/=(float value)
	{
		x /= value;
		y /= value;
		z /= value;
		return *this;
	}

	Vector3& operator/(float value) const
	{
		return Vector3(*this) /= value;
	}

	Vector3 Cross(const Vector3 &point) const
	{
		return Vector3(y * point.z - z * point.y, z * point.x - x * point.z, x * point.y - y * point.x);
	}

	Vector3& Normalize()
	{
		float len = Lenght();
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}

	Vector3 Normalized() const
	{
		float len = Lenght();
		return Vector3(x / len, y / len, z / len);
	}

	float Dot(const Vector3& point) const
	{
		return x * point.x + y*point.y + z*point.z;
	}

	float Lenght() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	bool Aproximately(const Vector3& r) const
	{
		return std::abs(r.x - x) < EPSILON && std::abs(r.y - y) < EPSILON && std::abs(r.z - z) < EPSILON;
	}
};

struct Vector2
{
	union
	{
		struct
		{
			float x, y;
		};
		float xy[2] = {0.0f, 0.0f};
	};

	Vector2() = default;
	Vector2(float v) : x(v), y(v) {}
	Vector2(float xIn, float yIn) : x(xIn), y(yIn) {}
	Vector2(const Vector3& v3) : x(v3.x), y(v3.y) {}
};

inline Vector4::Vector4(float xIn, float yIn, float zIn, float wIn) : x(xIn), y(yIn), z(zIn), w(wIn) {}

inline Vector4::Vector4(const Vector3& v3) : x(v3.x), y(v3.y), z(v3.z), w(1.0f) {}

inline Vector4& Vector4::operator/=(float value)
{
	x /= value;
	y /= value;
	z /= value;
	w /= value;
	return *this;
}

inline Vector4& Vector4::operator/(float value) const
{
	return Vector4(*this) /= value;
}

struct Matrix4x4
{
	union
	{
		float el_1D[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		float el_2D[4][4];
	};

	Matrix4x4() = default;

	Matrix4x4(float f) : Matrix4x4()
	{
		el_2D[0][0] = el_2D[1][1] = el_2D[2][2] = f;
		el_2D[3][3] = 1.0f;
	}

	Matrix4x4(float(&values)[16])
	{
		for (int i = 0; i < 16; i++)
			el_1D[i] = values[i];
	}

	Matrix4x4(float f1, float f2, float f3) : Matrix4x4()
	{
		el_2D[0][0] = 1.0f;
		el_2D[1][1] = 1.0f;
		el_2D[2][2] = 1.0f;
		el_2D[3][0] = f1;
		el_2D[3][1] = f2;
		el_2D[3][2] = f3;
	}

	Vector4 Column(int i) const
	{
		return Vector4(el_2D[0][i], el_2D[1][i], el_2D[2][i], el_2D[3][i]);
	}

	Vector3 Column3(int i) const
	{
		return Vector3(el_2D[0][i], el_2D[1][i], el_2D[2][i]);
	}

	Vector4 operator*(const Vector4& v) const
	{
		Vector4 ret(0.0f, 0.0f, 0.0f, 0.0f);

		for (auto i = 0; i < 4; i++)
		{
			ret.xyzw[i] = v.x * el_2D[i][0] + v.y * el_2D[i][1] + v.z * el_2D[i][2] + v.w * el_2D[i][3];
		}

		return ret;
	}

	Matrix4x4 operator*(const Matrix4x4& m) const
	{
		Matrix4x4 res;
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

	Matrix4x4 Inverse() const
	{
		float inv[16], invOut[16], det;
		// TODO: rewrite
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

		return Matrix4x4(invOut);
	}
};


struct Matrix3x3
{
	union
	{
		float el_1D[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		float el_2D[3][3];
	};

	Matrix3x3() = default;

	Matrix3x3(float f)
	{
		el_2D[0][0] = el_2D[0][1] = el_2D[0][2] = 0.0f;
		el_2D[1][0] = el_2D[1][1] = el_2D[1][2] = 0.0f;
		el_2D[2][0] = el_2D[2][1] = el_2D[2][2] = 0.0f;
		el_2D[0][0] = el_2D[1][1] = el_2D[2][2] = f;
	}

	Matrix3x3(float f1, float f2, float f3)
	{
		el_2D[0][0] = el_2D[0][1] = el_2D[0][2] = 0.0f;
		el_2D[1][0] = el_2D[1][1] = el_2D[1][2] = 0.0f;
		el_2D[2][0] = el_2D[2][1] = el_2D[2][2] = 0.0f;
		el_2D[0][0] = f1;
		el_2D[1][1] = f2;
		el_2D[2][2] = f3;
	}

	Matrix3x3(const Matrix4x4& m4)
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

	Matrix3x3& Inverse()
	{
		Matrix3x3 tmp(*this);
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

	Matrix3x3 Inversed()
	{
		Matrix3x3 ret(*this);
		ret.Inverse();
		return ret;
	}

	Matrix3x3& Transpose()
	{
		Matrix3x3 tmp(*this);
		el_2D[0][1] = tmp.el_2D[1][0];
		el_2D[0][2] = tmp.el_2D[2][0];
		el_2D[1][0] = tmp.el_2D[0][1];
		el_2D[1][2] = tmp.el_2D[2][1];
		el_2D[2][0] = tmp.el_2D[0][2];
		el_2D[2][1] = tmp.el_2D[1][2];
		return *this;
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

	Matrix3x3& operator*=(float f)
	{
		for (auto& e : el_1D) e *= f;
		return *this;
	}

	Matrix3x3 operator*(float f) const
	{
		return (Matrix3x3(*this) *= f);
	}

	Vector3 operator*(const Vector3& v) const
	{
		Vector3 ret;
		for (auto i = 0; i < 3; i++)
		{
			ret.xyz[i] = el_2D[i][0] * v.x + el_2D[i][1] * v.y + el_2D[i][2] * v.z;
		}
		return ret;
	}
};

typedef Vector2 vec2;
typedef Vector3 vec3;
typedef Vector4 vec4;
typedef Matrix3x3 mat3;
typedef Matrix4x4 mat4;


////////////////////////
// Quaternions
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


#pragma warning(default : 4201)
