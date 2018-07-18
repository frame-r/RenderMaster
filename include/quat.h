#pragma once
#include "vector_math.h"
#include <cmath> // sqrt


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
		const float DEGTORAD = 3.1415926f / 180;

		float a = DEGTORAD * xAngle / 2;
		float b = DEGTORAD * yAngle / 2;
		float g = DEGTORAD * zAngle / 2;

		//float cy = cos(DEGTORAD * zAngle * 0.5f);
		//float sy = sin(DEGTORAD * zAngle * 0.5f);
		//float cr = cos(DEGTORAD * yAngle * 0.5f);
		//float sr = sin(DEGTORAD * yAngle * 0.5f);
		//float cp = cos(DEGTORAD * xAngle * 0.5f);
		//float sp = sin(DEGTORAD * xAngle * 0.5f);

		//w = cy * cr * cp + sy * sr * sp;
		//y = cy * sr * cp - sy * cr * sp;
		//x = cy * cr * sp + sy * sr * cp;
		//z = sy * cr * cp - cy * sr * sp;

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
		constexpr float RADTODEG = 180 / 3.1415926f;

		//const float HALF_PI = 3.1415926f / 2;
		//const float RADTODEG = 180 / 3.1415926f;
		//
		//// roll (x-axis rotation)
		//float sinr = +2.0f * (w * x + y * z);
		//float cosr = +1.0f - 2.0f * (x * x + y * y);
		//float roll = atan2(sinr, cosr);
		//
		//// pitch (y-axis rotation)
		//float sinp = +2.0f * (w * y - z * x);
		//float pitch;
		//if (fabs(sinp) >= 1)
		//	pitch = copysign(HALF_PI, sinp); // use 90 degrees if out of range
		//else
		//	pitch = asin(sinp);
		//
		//// yaw (z-axis rotation)
		//float siny = +2.0f * (w * z + x * y);
		//float cosy = +1.0f - 2.0f * (y * y + z * z);
		//float yaw = atan2(siny, cosy);
		//
		//return vec3(roll * RADTODEG, pitch * RADTODEG, yaw * RADTODEG);

		mat4 M = ToMatrix();

		float alpha_rad, beta_rad, gamma_rad;

		float cy = sqrt(M.el_2D[2][2] * M.el_2D[2][2] + M.el_2D[1][2] * M.el_2D[1][2]);
		
		if (cy > 16 * 1.19e-07)
		{
			gamma_rad = -atan2(M.el_2D[0][1], M.el_2D[0][0]);
			beta_rad = -atan2(-M.el_2D[0][2], cy);
			alpha_rad = -atan2(M.el_2D[1][2], M.el_2D[2][2]);
		}else
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
		//mat4 ret;
		//
		//quat q = *this;
		//q.Normalize();
		//
		//float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
		//x2 = q.x + q.x;
		//y2 = q.y + q.y;
		//z2 = q.z + q.z;
		//xx = q.x * x2;   xy = q.x * y2;   xz = q.x * z2;
		//yy = q.y * y2;   yz = q.y * z2;   zz = q.z * z2;
		//wx = q.w * x2;   wy = q.w * y2;   wz = q.w * z2;
		//
		//ret.el_2D[0][0] = 1.0f - (yy + zz);	ret.el_2D[0][1] = xy - wz;			ret.el_2D[0][2] = xz + wy;
		//ret.el_2D[1][0] = xy + wz;			ret.el_2D[1][1] = 1.0f - (xx + zz); ret.el_2D[1][2] = yz - wx;
		//ret.el_2D[2][0] = xz - wy;			ret.el_2D[2][1] = yz + wx;			ret.el_2D[2][2] = 1.0f - (xx + yy);
		//ret.el_2D[3][3] = 1.0f;
		//
		//return ret;

		float Nq = x*x + y*y + z*z + w*w;
		float s;
		if (Nq > 0.0f)
			s = 2.0f / Nq;
		else
			s = 0.0f;

		float xs = x*s;  float ys = y*s;  float zs = z*s;
		float wx = w*xs; float wy = w*ys; float wz = w*zs;
		float xx = x*xs; float xy = x*ys; float xz = x*zs;
		float yy = y*ys; float yz = y*zs; float zz = z*zs;

		mat4 ret;
		ret.el_2D[0][0] = 1.0f - (yy + zz);  ret.el_2D[0][1] = xy - wz;          ret.el_2D[0][2] = xz + wy;           ret.el_2D[0][3] = 0.0f;
		ret.el_2D[1][0] = xy + wz;          ret.el_2D[1][1] = 1.0f - (xx + zz);  ret.el_2D[1][2] = yz - wx;           ret.el_2D[1][3] = 0.0f;
		ret.el_2D[2][0] = xz - wy;          ret.el_2D[2][1] = yz + wx;          ret.el_2D[2][2] = 1.0f - (xx + yy);     ret.el_2D[2][3] = 0.0f;
		ret.el_2D[3][0] = 0.0f;                ret.el_2D[3][1] = 0.0f;                ret.el_2D[3][2] = 0.0f;                 ret.el_2D[3][3] = 1.0f;

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
		constexpr float eps = 0.000001f;
		return std::abs(r.x - x) < eps && std::abs(r.y - y) < eps && std::abs(r.z - z) < eps && std::abs(r.w - w) < eps;
	}

};
