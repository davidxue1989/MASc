#pragma once
#include "stdafx.h"

#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)

namespace Algebra3D {
	class Algebra3D
	{
	public:
		Algebra3D() {};
		~Algebra3D() {};
	};



	class Vector
	{
	public:
		Vector() : x(0), y(0), z(0) {};
		Vector(float a, float b, float c) : x(a), y(b), z(c) {};
		Vector operator + (Vector v2) { return Vector(x + v2.x, y + v2.y, z + v2.z); };
		Vector operator - (Vector v2) { return Vector(x - v2.x, y - v2.y, z - v2.z); };
		Vector operator * (float s) { return Vector(s*x, s*y, s*z); };
		Vector operator / (float s) { return Vector(x / s, y / s, z / s); };
		void set(float a, float b, float c) { x = a; y = b; z = c; };

		float x, y, z;
	};

	class Matrix
	{
	public:
		Matrix(Vector C1, Vector C2, Vector C3) : c1(C1), c2(C2), c3(C3) {};
		Vector operator * (Vector v) { return c1*v.x + c2*v.y + c3*v.z; };
		void set(Vector C1, Vector C2, Vector C3) { c1 = C1; c2 = C2; c3 = C3; };

		Vector c1, c2, c3;
	};
}