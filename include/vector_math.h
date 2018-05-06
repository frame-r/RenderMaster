#pragma once
#include <cmath> // sqrt

// Don't include this file directly.
// Instead include Engine.h

struct Vector3;
struct Vector4;
struct Matrix3x3;
struct Matrix4x4;

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

	static Vector3 one() { return Vector3(1.0f, 1.0f, 1.0f); }
	static Vector3 zero() { return Vector3(0.0f, 0.0f, 0.0f); }
	static Vector3 forward() { return Vector3(0.0f, 0.0f, 1.0f); }
	static Vector3 back() { return Vector3(0.0f, 0.0f, -1.0f); }
	static Vector3 right() { return Vector3(1.0f, 0.0f, 0.0f); }
	static Vector3 left() { return Vector3(-1.0f, 0.0f, 0.0f); }
	static Vector3 up() { return Vector3(0.0f, 1.0f, 0.0f); }
	static Vector3 down() { return Vector3(0.0f, -1.0f, 0.0f); }

	Vector3() = default;

	Vector3(float v)
	{
		x = y = z = v;
	}

	Vector3(float xIn, float yIn, float zIn)
	{
		x = xIn;
		y = yIn;
		z = zIn;
	}

	Vector3 &operator += (const Vector3 &point)
	{
		x += point.x;
		y += point.y;
		z += point.z;
		return *this;
	}

	Vector3 operator + (const Vector3 &point) const
	{
		return Vector3(*this) += point;
	}

	Vector3 &operator -= (const Vector3 &point)
	{
		x -= point.x;
		y -= point.y;
		z -= point.z;
		return *this;
	}

	Vector3 operator - (const Vector3 &point) const
	{
		return Vector3(*this) -= point;
	}

	Vector3 &operator *= (const Vector3 &point)
	{
		x *= point.x;
		y *= point.y;
		z *= point.z;
		return *this;
	}

	Vector3 operator * (const Vector3 &point) const
	{
		return Vector3(*this) *= point;
	}

	Vector3 &operator *= (float value)
	{
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	Vector3 operator * (float value) const
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

	Vector3 &Normalize()
	{
		float len = Lenght();
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}

	Vector3 Normalized() const
	{
		Vector3 ret(*this);
		float len = Lenght();
		ret.x /= len;
		ret.y /= len;
		ret.z /= len;
		return ret;
	}

	float Dot(const Vector3& point) const
	{
		return x * point.x + y*point.y + z*point.z;
	}

	float Lenght() const
	{
		return sqrt(x * x + y * y + z * z);
	}
};


struct Vector4
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		float xyzw[4];
	};

	Vector4()
	{
		x = y = z = 0;
		w = 1.0f;
	}

	Vector4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vector4(const Vector3& v3)
	{
		x = v3.x;
		y = v3.y;
		z = v3.z;
		w = 1.0f;
	}

	Vector4& operator/=(float value)
	{
		x /= value;
		y /= value;
		z /= value;
		w /= value;
		return *this;
	}

	Vector4& operator/(float value) const
	{
		return Vector4(*this) /= value;
	}
};

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

	Vector4 operator*(const Vector4& v) const
	{
		Vector4 ret(0.0f, 0.0f, 0.0f, 0.0f);
		for (auto i = 0; i < 4; i++)
		{
			ret.x += el_2D[i][0] * v.xyzw[i];
			ret.y += el_2D[i][1] * v.xyzw[i];
			ret.z += el_2D[i][2] * v.xyzw[i];
			ret.w += el_2D[i][3] * v.xyzw[i];
		}
		return ret;
	}

	Matrix4x4 operator* (const Matrix4x4& m) const
	{
		Matrix4x4 res;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				res.el_2D[i][j] = 0;
				for (int k = 0; k < 4; k++)
					res.el_2D[i][j] += el_2D[i][k] * m.el_2D[k][j];
			}
		}
		return res;
	}

	Matrix4x4 Inverse() const
	{
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
		Matrix3x3 cache(*this);
		float inv_det = 1.0f / Det();
		el_2D[0][0] = cache.el_2D[1][1] * cache.el_2D[2][2] - cache.el_2D[1][2] * cache.el_2D[2][1];
		el_2D[0][1] = cache.el_2D[0][2] * cache.el_2D[2][1] - cache.el_2D[0][1] * cache.el_2D[2][2];
		el_2D[0][2] = cache.el_2D[0][1] * cache.el_2D[1][2] - cache.el_2D[0][2] * cache.el_2D[1][1];
		el_2D[1][0] = cache.el_2D[1][2] * cache.el_2D[2][0] - cache.el_2D[1][0] * cache.el_2D[2][2];
		el_2D[1][1] = cache.el_2D[0][0] * cache.el_2D[2][2] - cache.el_2D[0][2] * cache.el_2D[2][0];
		el_2D[1][2] = cache.el_2D[0][2] * cache.el_2D[1][0] - cache.el_2D[0][0] * cache.el_2D[1][2];
		el_2D[2][0] = cache.el_2D[1][0] * cache.el_2D[2][1] - cache.el_2D[1][1] * cache.el_2D[2][0];
		el_2D[2][1] = cache.el_2D[0][0] * cache.el_2D[1][2] - cache.el_2D[1][0] * cache.el_2D[0][2];
		el_2D[2][2] = cache.el_2D[0][0] * cache.el_2D[1][1] - cache.el_2D[0][1] * cache.el_2D[1][0];
		*this *= inv_det;
		return *this;
	}

	Matrix3x3 MakeInverse()
	{
		Matrix3x3 ret(*this);
		ret.Inverse();
		return ret;
	}

	Matrix3x3& Transpose()
	{
		Matrix3x3 cache(*this);
		el_2D[0][1] = cache.el_2D[1][0];
		el_2D[0][2] = cache.el_2D[2][0];
		el_2D[1][0] = cache.el_2D[0][1];
		el_2D[1][2] = cache.el_2D[2][1];
		el_2D[2][0] = cache.el_2D[0][2];
		el_2D[2][1] = cache.el_2D[1][2];
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
			ret.x += el_2D[0][i] * v.xyz[i];
			ret.y += el_2D[1][i] * v.xyz[i];
			ret.z += el_2D[2][i] * v.xyz[i];
		}
		return ret;
	}
};

typedef Vector3 vec3;
typedef Vector4 vec4;
typedef Matrix3x3 mat3;
typedef Matrix4x4 mat4;
