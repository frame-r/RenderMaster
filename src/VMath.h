#pragma once

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
		float xyz[3];
	};

	static const Vector3 one;
	static const Vector3 zero;
	static const Vector3 forward;
	static const Vector3 back;
	static const Vector3 right;
	static const Vector3 left;
	static const Vector3 up;
	static const Vector3 down;

	Vector3();
	Vector3(float v);
	Vector3(float x_, float y_, float z_);

	Vector3 &operator += (const Vector3 &point);
	Vector3 operator + (const Vector3 &point) const;
	Vector3 &operator -= (const Vector3 &point);
	Vector3 operator - (const Vector3 &point) const;
	Vector3 &operator *= (const Vector3 &point);
	Vector3 operator * (const Vector3 &point) const;
	Vector3 &operator *= (float value);
	Vector3 operator * (float value) const;
	Vector3 &operator /= (float value);
	Vector3 &operator / (float value) const;
	Vector3 Cross(const Vector3 &point) const;
	Vector3 &Normalize();
	Vector3 Normalized() const;
	float Dot(const Vector3 &point) const;
	float Lenght() const;
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

	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(const Vector3& v3);

	Vector4 &operator/=(float value);
	Vector4 &operator/(float value) const;
};


struct Matrix3x3
{
	union
	{
		float el_1D[9];
		float el_2D[3][3];
	};

	Matrix3x3();
	Matrix3x3(float f);
	Matrix3x3(float f1, float f2, float f3);
	Matrix3x3(const Matrix4x4& mat);

	Matrix3x3& Inverse();
	Matrix3x3 MakeInverse();
	Matrix3x3& Transpose();
	float Det() const;

	Matrix3x3 &operator*=(float f);
	Matrix3x3 operator*(float f) const;
	Vector3 operator*(const Vector3& v) const;

};


struct Matrix4x4
{
	union
	{
		float el_1D[16];
		float el_2D[4][4];
	};

	Matrix4x4();
	Matrix4x4(float f);
	Matrix4x4(float f1, float f2, float f3);
	Matrix4x4(float (&values)[16]);

	Vector4 operator*(const Vector4& v) const;
	Matrix4x4 operator*(const Matrix4x4& m) const;
	Matrix4x4 Inverse() const;

};

typedef Vector3 vec3;
typedef Vector4 vec4;
typedef Matrix3x3 mat3;
typedef Matrix4x4 mat4;
