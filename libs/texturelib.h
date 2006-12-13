/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined (INCLUDED_TEXTURELIB_H)
#define INCLUDED_TEXTURELIB_H

#include "debugging/debugging.h"
#include "math/Vector3.h"
#include "math/matrix.h"
#include "math/Plane3.h"
typedef Vector3 Colour3;
typedef unsigned int GLuint;
class LoadImageCallback;

enum ProjectionAxis {
	eProjectionAxisX = 0,
	eProjectionAxisY = 1,
	eProjectionAxisZ = 2,
};

// describes a GL texture
struct qtexture_t
{
  qtexture_t(const LoadImageCallback& load, const char* name) : load(load), name(name)
  {
  }
  const LoadImageCallback& load;
  const char* name;
  std::size_t width, height;
  GLuint texture_number; // gl bind number
  Colour3 color; // for flat shade mode
  int surfaceFlags, contentFlags, value;
};

inline Matrix4 matrix4_rotation_for_vector3(const Vector3& x, const Vector3& y, const Vector3& z) {
	return Matrix4(
		x.x(), x.y(), x.z(), 0,
		y.x(), y.y(), y.z(), 0,
		z.x(), z.y(), z.z(), 0,
		0, 0, 0, 1
	);
}

inline Matrix4 matrix4_swap_axes(const Vector3& from, const Vector3& to) {
	if(from.x() != 0 && to.y() != 0) {
		return matrix4_rotation_for_vector3(to, from, g_vector3_axis_z);
	}

	if(from.x() != 0 && to.z() != 0) {
		return matrix4_rotation_for_vector3(to, g_vector3_axis_y, from);
	}

	if(from.y() != 0 && to.z() != 0) {
		return matrix4_rotation_for_vector3(g_vector3_axis_x, to, from);
	}

	if(from.y() != 0 && to.x() != 0) {
		return matrix4_rotation_for_vector3(from, to, g_vector3_axis_z);
	}

	if(from.z() != 0 && to.x() != 0) {
		return matrix4_rotation_for_vector3(from, g_vector3_axis_y, to);
	}

	if(from.z() != 0 && to.y() != 0) {
		return matrix4_rotation_for_vector3(g_vector3_axis_x, from, to);
	}

	ERROR_MESSAGE("unhandled axis swap case");

	return g_matrix4_identity;
}

inline Matrix4 matrix4_reflection_for_plane(const Plane3& plane) {
	return Matrix4(
		static_cast<float>(1 - (2 * plane.a * plane.a)),
		static_cast<float>(-2 * plane.a * plane.b),
		static_cast<float>(-2 * plane.a * plane.c),
		0,
		static_cast<float>(-2 * plane.b * plane.a),
		static_cast<float>(1 - (2 * plane.b * plane.b)),
		static_cast<float>(-2 * plane.b * plane.c),
		0,
		static_cast<float>(-2 * plane.c * plane.a),
		static_cast<float>(-2 * plane.c * plane.b),
		static_cast<float>(1 - (2 * plane.c * plane.c)),
		0,
		static_cast<float>(-2 * plane.d * plane.a),
		static_cast<float>(-2 * plane.d * plane.b),
		static_cast<float>(-2 * plane.d * plane.c),
		1
	);
}

inline Matrix4 matrix4_reflection_for_plane45(const Plane3& plane, const Vector3& from, const Vector3& to) {
	Vector3 first = from;
	Vector3 second = to;

	if (from.dot(plane.normal()) > 0 == to.dot(plane.normal()) > 0) {
		first = -first;
		second = -second;
	}

#if 0
  globalOutputStream() << "normal: ";
  print_vector3(plane.normal());

  globalOutputStream() << "from: ";
  print_vector3(first);

  globalOutputStream() << "to: ";
  print_vector3(second);
#endif

	Matrix4 swap = matrix4_swap_axes(first, second);
	
	Matrix4 tmp = matrix4_reflection_for_plane(plane);
	
	swap.tx() = -static_cast<float>(-2 * plane.a * plane.d);
	swap.ty() = -static_cast<float>(-2 * plane.b * plane.d);
	swap.tz() = -static_cast<float>(-2 * plane.c * plane.d);
	
	return swap;
}

const float ProjectionAxisEpsilon = static_cast<float>(0.0001);

inline bool projectionaxis_better(float axis, float other) {
	return fabs(axis) > fabs(other) + ProjectionAxisEpsilon;
}

/// \brief Texture axis precedence: Z > X > Y
inline ProjectionAxis projectionaxis_for_normal(const Vector3& normal) {
	return (projectionaxis_better(normal[eProjectionAxisY], normal[eProjectionAxisX]))
		? (projectionaxis_better(normal[eProjectionAxisY], normal[eProjectionAxisZ]))
			? eProjectionAxisY
			: eProjectionAxisZ
		: (projectionaxis_better(normal[eProjectionAxisX], normal[eProjectionAxisZ]))
			? eProjectionAxisX
			: eProjectionAxisZ;
}

/*!
\brief Construct a transform from XYZ space to ST space (3d to 2d).
This will be one of three axis-aligned spaces, depending on the surface normal.
NOTE: could also be done by swapping values.
*/
inline void Normal_GetTransform(const Vector3& normal, Matrix4& transform) {
	switch (projectionaxis_for_normal(normal)) {
		case eProjectionAxisZ:
			transform[0]  =  1;
			transform[1]  =  0;
			transform[2]  =  0;
			    
			transform[4]  =  0;
			transform[5]  =  1;
			transform[6]  =  0;
			    
			transform[8]  =  0;
			transform[9]  =  0;
			transform[10] =  1;
		break;
		case eProjectionAxisY:
			transform[0]  =  1;
			transform[1]  =  0;
			transform[2]  =  0;
			    
			transform[4]  =  0;
			transform[5]  =  0;
			transform[6]  = -1;
			    
			transform[8]  =  0;
			transform[9]  =  1;
			transform[10] =  0;
		break;
		case eProjectionAxisX:
			transform[0]  =  0;
			transform[1]  =  0;
			transform[2]  =  1;
			    
			transform[4]  =  1;
			transform[5]  =  0;
			transform[6]  =  0;
			    
			transform[8]  =  0;
			transform[9]  =  1;
			transform[10] =  0;
		break;
	}
	transform[3] = transform[7] = transform[11] = transform[12] = transform[13] = transform[14] = 0;
	transform[15] = 1;
}

//++timo replace everywhere texX by texS etc. ( ----> and in q3map !) 
// NOTE : ComputeAxisBase here and in q3map code must always BE THE SAME !
// WARNING : special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere when x == 0
// rotation by (0,RotY,RotZ) assigns X to normal
inline void ComputeAxisBase(const Vector3& normal, Vector3& texS, Vector3& texT) {
	const Vector3 up(0, 0, 1);
	const Vector3 down(0, 0, -1);

	if(vector3_equal_epsilon(normal, up, float(1e-6))) {
		texS = Vector3(0, 1, 0);
		texT = Vector3(1, 0, 0);
	}
	else if(vector3_equal_epsilon(normal, down, float(1e-6))) {
		texS = Vector3(0, 1, 0);
		texT = Vector3(-1, 0, 0);
	}
	else {
		texS = normal.crossProduct(up).getNormalised();
		texT = normal.crossProduct(texS).getNormalised();
		texS = -texS;
	}
}

// handles degenerate cases, just in case library atan2 doesn't
inline double arctangent_yx(double y, double x) {
	if (fabs(x) > 1.0E-6) {
		return atan2(y, x);
	}
	else if (y > 0) {
		return c_half_pi;
	}
	else {
		return -c_half_pi;
	}
}

#endif
