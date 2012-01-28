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

#if !defined(INCLUDED_RENDER_H)
#define INCLUDED_RENDER_H

/// \file
/// \brief High-level constructs for efficient OpenGL rendering.

#include "render/ArbitraryMeshVertex.h"
#include "render/Vertex3f.h"
#include "render/TexCoord2f.h"

#include "irender.h"
#include "igl.h"

#include "math/FloatTools.h"
#include "math/Vector2.h"
#include "math/pi.h"

#include <vector>

typedef unsigned int RenderIndex;
const GLenum RenderIndexTypeID = GL_UNSIGNED_INT;

/// Vector of indices for use with glDrawElements
typedef std::vector<RenderIndex> IndexBuffer;

namespace std {
/// \brief Swaps the values of \p self and \p other.
/// Overloads std::swap.
inline void swap(IndexBuffer& self, IndexBuffer& other) {
	self.swap(other);
}
}

/// \brief A wrapper around a std::vector which inserts only vertices which have not already been inserted.
/// \param Vertex The vertex data type. Must support operator<, operator== and operator!=.
/// For best performance, quantise vertices before inserting them.
template<typename Vertex>
class UniqueVertexBuffer {
	typedef std::vector<Vertex> Vertices;
	Vertices& m_data;

	struct bnode {
		bnode()
				: m_left(0), m_right(0) {}
		RenderIndex m_left;
		RenderIndex m_right;
	};

	std::vector<bnode> m_btree;
	RenderIndex m_prev0;
	RenderIndex m_prev1;
	RenderIndex m_prev2;

	const RenderIndex find_or_insert(const Vertex& vertex) {
		RenderIndex index = 0;

		while(1) {
			if(vertex < m_data[index]) {
				bnode& node = m_btree[index];
				if(node.m_left != 0) {
					index = node.m_left;
					continue;
				}
				else {
					node.m_left = RenderIndex(m_btree.size());
					m_btree.push_back(bnode());
					m_data.push_back(vertex);
					return RenderIndex(m_btree.size()-1);
				}
			}
			if(m_data[index] < vertex) {
				bnode& node = m_btree[index];
				if(node.m_right != 0) {
					index = node.m_right;
					continue;
				}
				else {
					node.m_right = RenderIndex(m_btree.size());
					m_btree.push_back(bnode());
					m_data.push_back(vertex);
					return RenderIndex(m_btree.size()-1);
				}
			}

			return index;
		}
	}
public:
	UniqueVertexBuffer(Vertices& data)
			: m_data(data), m_prev0(0), m_prev1(0), m_prev2(0) {}

	typedef typename Vertices::const_iterator iterator;

	iterator begin() const {
		return m_data.begin();
	}
	iterator end() const {
		return m_data.end();
	}

	std::size_t size() const {
		return m_data.size();
	}
	const Vertex* data() const {
		return &(*m_data.begin());
	}
	Vertex& operator[](std::size_t index) {
		return m_data[index];
	}
	const Vertex& operator[](std::size_t index) const {
		return m_data[index];
	}

	void clear() {
		m_prev0 = 0;
		m_prev1 = 0;
		m_prev2 = 0;
		m_data.clear();
		m_btree.clear();
	}
	void reserve(std::size_t max_vertices) {
		m_data.reserve(max_vertices);
		m_btree.reserve(max_vertices);
	}
	/// \brief Returns the index of the element equal to \p vertex.
	RenderIndex insert(const Vertex& vertex) {
		if(m_data.empty()) {
			m_data.push_back(vertex);
			m_btree.push_back(bnode());
			return 0;
		}

		if(m_data[m_prev0] == vertex)
			return m_prev0;
		if(m_prev1 != m_prev0 && m_data[m_prev1] == vertex)
			return m_prev1;
		if(m_prev2 != m_prev0 && m_prev2 != m_prev1 && m_data[m_prev2] == vertex)
			return m_prev2;

		m_prev2 = m_prev1;
		m_prev1 = m_prev0;
		m_prev0 = find_or_insert(vertex);

		return m_prev0;
	}
};


/// \brief A 4-byte colour.
struct Colour4b {
	unsigned char r, g, b, a;

	Colour4b() {}

	Colour4b(unsigned char _r, unsigned char _g,
			 unsigned char _b, unsigned char _a) :
		r(_r), g(_g), b(_b), a(_a)
	{}

	bool operator<(const Colour4b& other) const {
		if (r != other.r) {
			return r < other.r;
		}
		if (g != other.g) {
			return g < other.g;
		}
		if (b != other.b) {
			return b < other.b;
		}
		if (a != other.a) {
			return a < other.a;
		}
		return false;
	}

	bool operator==(const Colour4b& other) const {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	bool operator!=(const Colour4b& other) const {
		return !operator==(other);
	}
};

// A Normal3f is just another Vertex3f (Vector3)
typedef Vertex3f Normal3f;

/// \brief Returns \p normal rescaled to be unit-length.
/*inline Normal3f normal3f_normalised(const Normal3f& normal) {
	return Normal3f(normal.getNormalised());
}*/

enum UnitSphereOctant
{
    UNITSPHEREOCTANT_000 = 0 << 0 | 0 << 1 | 0 << 2,
    UNITSPHEREOCTANT_001 = 0 << 0 | 0 << 1 | 1 << 2,
    UNITSPHEREOCTANT_010 = 0 << 0 | 1 << 1 | 0 << 2,
    UNITSPHEREOCTANT_011 = 0 << 0 | 1 << 1 | 1 << 2,
    UNITSPHEREOCTANT_100 = 1 << 0 | 0 << 1 | 0 << 2,
    UNITSPHEREOCTANT_101 = 1 << 0 | 0 << 1 | 1 << 2,
    UNITSPHEREOCTANT_110 = 1 << 0 | 1 << 1 | 0 << 2,
    UNITSPHEREOCTANT_111 = 1 << 0 | 1 << 1 | 1 << 2,
};

/// \brief Returns the octant for \p normal indicating the sign of the region of unit-sphere space it lies within.
inline UnitSphereOctant normal3f_classify_octant(const Normal3f& normal) {
	return static_cast<UnitSphereOctant>(
	           ((normal.x() > 0) << 0) | ((normal.y() > 0) << 1) | ((normal.z() > 0) << 2)
	       );
}

/// \brief Returns \p normal with its components signs made positive based on \p octant.
inline Normal3f normal3f_fold_octant(const Normal3f& normal, UnitSphereOctant octant) {
	switch(octant) {
		case UNITSPHEREOCTANT_000:
			return Normal3f(-normal.x(), -normal.y(), -normal.z());
		case UNITSPHEREOCTANT_001:
			return Normal3f(normal.x(), -normal.y(), -normal.z());
		case UNITSPHEREOCTANT_010:
			return Normal3f(-normal.x(), normal.y(), -normal.z());
		case UNITSPHEREOCTANT_011:
			return Normal3f(normal.x(), normal.y(), -normal.z());
		case UNITSPHEREOCTANT_100:
			return Normal3f(-normal.x(), -normal.y(), normal.z());
		case UNITSPHEREOCTANT_101:
			return Normal3f(normal.x(), -normal.y(), normal.z());
		case UNITSPHEREOCTANT_110:
			return Normal3f(-normal.x(), normal.y(), normal.z());
		case UNITSPHEREOCTANT_111:
			return Normal3f(normal.x(), normal.y(), normal.z());
	}
	return Normal3f();
}

/// \brief Reverses the effect of normal3f_fold_octant() on \p normal with \p octant.
/// \p normal must have been obtained with normal3f_fold_octant().
/// \p octant must have been obtained with normal3f_classify_octant().
inline Normal3f normal3f_unfold_octant(const Normal3f& normal, UnitSphereOctant octant) {
	return normal3f_fold_octant(normal, octant);
}

enum UnitSphereSextant
{
    UNITSPHERESEXTANT_XYZ = 0,
    UNITSPHERESEXTANT_XZY = 1,
    UNITSPHERESEXTANT_YXZ = 2,
    UNITSPHERESEXTANT_YZX = 3,
    UNITSPHERESEXTANT_ZXY = 4,
    UNITSPHERESEXTANT_ZYX = 5,
};

/// \brief Returns the sextant for \p normal indicating how to sort its components so that x > y > z.
/// All components of \p normal must be positive.
/// \p normal must be normalised.
inline UnitSphereSextant normal3f_classify_sextant(const Normal3f& normal) {
	return
	    normal.x() >= normal.y()
	    ? normal.x() >= normal.z()
	    ? normal.y() >= normal.z()
	    ? UNITSPHERESEXTANT_XYZ
	    : UNITSPHERESEXTANT_XZY
    : UNITSPHERESEXTANT_ZXY
    : normal.y() >= normal.z()
	    ? normal.x() >= normal.z()
	    ? UNITSPHERESEXTANT_YXZ
	    : UNITSPHERESEXTANT_YZX
    : UNITSPHERESEXTANT_ZYX;
}

/// \brief Returns \p normal with its components sorted so that x > y > z based on \p sextant.
/// All components of \p normal must be positive.
/// \p normal must be normalised.
inline Normal3f normal3f_fold_sextant(const Normal3f& normal, UnitSphereSextant sextant) {
	switch(sextant) {
		case UNITSPHERESEXTANT_XYZ:
			return Normal3f(normal.x(), normal.y(), normal.z());
		case UNITSPHERESEXTANT_XZY:
			return Normal3f(normal.x(), normal.z(), normal.y());
		case UNITSPHERESEXTANT_YXZ:
			return Normal3f(normal.y(), normal.x(), normal.z());
		case UNITSPHERESEXTANT_YZX:
			return Normal3f(normal.y(), normal.z(), normal.x());
		case UNITSPHERESEXTANT_ZXY:
			return Normal3f(normal.z(), normal.x(), normal.y());
		case UNITSPHERESEXTANT_ZYX:
			return Normal3f(normal.z(), normal.y(), normal.x());
	}
	return Normal3f();
}

/// \brief Reverses the effect of normal3f_fold_sextant() on \p normal with \p sextant.
/// \p normal must have been obtained with normal3f_fold_sextant().
/// \p sextant must have been obtained with normal3f_classify_sextant().
inline Normal3f normal3f_unfold_sextant(const Normal3f& normal, UnitSphereSextant sextant) {
	return normal3f_fold_sextant(normal, sextant);
}

const unsigned int c_quantise_normal = 1 << 6;

/// \brief All the components of \p folded must be positive and sorted so that x > y > z.
inline Normal3f normal3f_folded_quantised(const Normal3f& folded) {
	// compress
	float scale = static_cast<float>(c_quantise_normal) / (folded.x() + folded.y() + folded.z());
	unsigned int zbits = static_cast<unsigned int>(folded.z() * scale);
	unsigned int ybits = static_cast<unsigned int>(folded.y() * scale);

	// decompress
	Normal3f normal(static_cast<float>(c_quantise_normal) - zbits - ybits, static_cast<float>(ybits), static_cast<float>(zbits));
	return Normal3f(normal.getNormalised());
}

/// \brief Returns \p normal quantised by compressing and then decompressing its representation.
inline Normal3f normal3f_quantised_custom(const Normal3f& normal) {
	UnitSphereOctant octant = normal3f_classify_octant(normal);
	Normal3f folded = normal3f_fold_octant(normal, octant);
	UnitSphereSextant sextant = normal3f_classify_sextant(folded);
	folded = normal3f_fold_sextant(folded, sextant);
	return normal3f_unfold_octant(normal3f_unfold_sextant(normal3f_folded_quantised(folded), sextant), octant);
}



struct spherical_t {
	float longditude, latitude;

	spherical_t(float _longditude, float _latitude)
			: longditude(_longditude), latitude(_latitude) {}
}
;

/*
{
  theta = 2pi * U;
  phi = acos((2 * V) - 1);

  U = theta / 2pi;
  V = (cos(phi) + 1) / 2;
}

longitude = atan(y / x);
latitude = acos(z);
*/
struct uniformspherical_t {
	float U, V;

	uniformspherical_t(float U_, float V_)
			: U(U_), V(V_) {}
}
;


inline spherical_t spherical_from_normal3f(const Normal3f& normal)
{
	return spherical_t(
		normal.x() == 0 ? static_cast<float>(c_half_pi) : 
			normal.x() > 0 ? atan(normal.y() / normal.x()) : 
				atan(normal.y() / normal.x()) + static_cast<float>(c_pi), acos(normal.z()));
}

inline Normal3f normal3f_from_spherical(const spherical_t& spherical)
{
	return Normal3f(
	    cos(spherical.longditude) * sin(spherical.latitude),
	    sin(spherical.longditude) * sin(spherical.latitude),
	    cos(spherical.latitude)
	);
}

inline uniformspherical_t uniformspherical_from_spherical(const spherical_t& spherical)
{
	return uniformspherical_t(spherical.longditude * static_cast<float>(c_inv_2pi), (cos(spherical.latitude) + 1) * 0.5f);
}

inline spherical_t spherical_from_uniformspherical(const uniformspherical_t& uniformspherical) {
	return spherical_t(static_cast<float>(c_2pi) * uniformspherical.U, acos((2 * uniformspherical.V) - 1));
}

inline uniformspherical_t uniformspherical_from_normal3f(const Normal3f& normal) {
	return uniformspherical_from_spherical(spherical_from_normal3f(normal));
	//return uniformspherical_t(atan2(normal.y / normal.x) * c_inv_2pi, (normal.z + 1) * 0.5);
}

inline Normal3f normal3f_from_uniformspherical(const uniformspherical_t& uniformspherical) {
	return normal3f_from_spherical(spherical_from_uniformspherical(uniformspherical));
}

/// \brief Returns a single-precision \p component quantised to \p precision.
inline float float_quantise(float component, float precision) {
	return float_snapped(component, precision);
}

/// \brief Returns a double-precision \p component quantised to \p precision.
inline double double_quantise(double component, double precision) {
	return float_snapped(component, precision);
}

inline spherical_t spherical_quantised(const spherical_t& spherical, float snap) {
	return spherical_t(float_quantise(spherical.longditude, snap), float_quantise(spherical.latitude, snap));
}

inline uniformspherical_t uniformspherical_quantised(const uniformspherical_t& uniformspherical, float snap) {
	return uniformspherical_t(float_quantise(uniformspherical.U, snap), float_quantise(uniformspherical.V, snap));
}

/// \brief Returns a \p vertex quantised to \p precision.
inline Vertex3f vertex3f_quantised(const Vertex3f& vertex, float precision) {
	return Vertex3f(float_quantise(vertex.x(), precision), float_quantise(vertex.y(), precision), float_quantise(vertex.z(), precision));
}

/// \brief Returns a \p normal quantised to a fixed precision.
inline Normal3f normal3f_quantised(const Normal3f& normal) {
	return normal3f_quantised_custom(normal);
	//return normal3f_from_spherical(spherical_quantised(spherical_from_normal3f(normal), snap));
	//return normal3f_from_uniformspherical(uniformspherical_quantised(uniformspherical_from_normal3f(normal), snap));
	//  float_quantise(normal.x, snap), float_quantise(normal.y, snap), float_quantise(normal.y, snap));
}

/// \brief Returns a \p texcoord quantised to \p precision.
inline TexCoord2f texcoord2f_quantised(const TexCoord2f& texcoord, float precision) {
	return TexCoord2f(float_quantise(texcoord.s(), precision), float_quantise(texcoord.t(), precision));
}

/// \brief Standard vertex type for lines and points.
// greebo: A PointVertex basically consists of a location and a colour
class PointVertex {
public:
	Colour4b colour;
	Vertex3f vertex;

	PointVertex()
	{}

	PointVertex(const Vertex3f& _vertex) :
		colour(Colour4b(255, 255, 255, 255)),
		vertex(_vertex)
	{}

	PointVertex(const Vertex3f& _vertex, const Colour4b& _colour) :
		colour(_colour),
		vertex(_vertex)
	{}

	PointVertex(const Vector3& vector) :
		colour(255, 255, 255, 255),
		vertex(vector)
	{}

	// greebo: Same as above, but with a Vector3 as <point> argument
	PointVertex(const Vector3& point, const Colour4b& _colour) :
		colour(_colour),
		vertex(point)
	{}

	bool operator< (const PointVertex& other) const {
		if (vertex != other.vertex) {
			return vertex < other.vertex;
		}

		if (colour != other.colour) {
			return colour < other.colour;
		}

		return false;
	}

	bool operator== (const PointVertex& other) const {
		return colour == other.colour && vertex == other.vertex;
	}

	bool operator!= (const PointVertex& other) const {
		return !(*this == other);
	}
};

// A flat-shaded vertex has a position, a colour and a normal vector
struct FlatShadedVertex
{
	Vertex3f vertex;
	Colour4b colour;
	Normal3f normal;
};

const float c_quantise_vertex = 1.f / static_cast<float>(1 << 3);

/// \brief Returns \p v with vertex quantised to a fixed precision.
inline PointVertex pointvertex_quantised(const PointVertex& v) {
	return PointVertex(vertex3f_quantised(v.vertex, c_quantise_vertex), v.colour);
}

const float c_quantise_texcoord = 1.f / static_cast<float>(1 << 8);

/// \brief Returns \p v with vertex, normal and texcoord quantised to a fixed precision.
inline ArbitraryMeshVertex arbitrarymeshvertex_quantised(const ArbitraryMeshVertex& v) {
	return ArbitraryMeshVertex(vertex3f_quantised(v.vertex, c_quantise_vertex), normal3f_quantised(v.normal), texcoord2f_quantised(v.texcoord, c_quantise_texcoord));
}


/// \brief Sets up the OpenGL colour and vertex arrays for \p array.
inline void pointvertex_gl_array(const PointVertex* array)
{
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &array->colour);
	glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &array->vertex);
}

/// A renderable collection of coloured vertices
class RenderablePointVector :
	public OpenGLRenderable
{
protected:
	typedef std::vector<PointVertex> PointVertexVector;
	PointVertexVector _vector;
	const GLenum _mode;

public:
	RenderablePointVector(GLenum mode) :
		_mode(mode)
	{}

	RenderablePointVector(GLenum mode, std::size_t initialSize) :
		_vector(initialSize),
		_mode(mode)
	{}

	void render(const RenderInfo& info) const
	{
		if (_vector.empty()) return;

        // Enable point colours if required
        if (info.checkFlag(RENDER_VERTEX_COLOUR)
            || (info.checkFlag(RENDER_POINT_COLOUR) && _mode == GL_POINTS))
        {
            glEnableClientState(GL_COLOR_ARRAY);
        }
		pointvertex_gl_array(&_vector.front());
		glDrawArrays(_mode, 0, static_cast<GLsizei>(_vector.size()));
	}

	// Convenience method to set the colour of the whole array
	void setColour(const Colour4b& colour)
	{
		for (PointVertexVector::iterator i = _vector.begin(); i != _vector.end(); ++i)
		{
			i->colour = colour;
		}
	}

	const PointVertex& operator[](std::size_t i) const
	{
		return _vector[i];
	}

	PointVertex& operator[](std::size_t i)
	{
		return _vector[i];
	}

	PointVertex& front()
	{
		return _vector.front();
	}

	const PointVertex& front() const
	{
		return _vector.front();
	}

	std::size_t size() const {
		return _vector.size();
	}

	bool empty() const {
		return _vector.empty();
	}

	void clear() {
		_vector.clear();
	}

	void resize(std::size_t size) {
		_vector.resize(size);
	}

	void reserve(std::size_t size) {
		_vector.reserve(size);
	}

	void push_back(const PointVertex& point) {
		_vector.push_back(point);
	}
};


/// A renderable wrapper for a collection of vertices stored elsewhere
class RenderableVertexBuffer : public OpenGLRenderable
{
	const GLenum _mode;
	const std::vector<PointVertex>& m_vertices;
public:
	RenderableVertexBuffer(GLenum mode, const std::vector<PointVertex>& vertices)
			: _mode(mode), m_vertices(vertices) {}

	void render(const RenderInfo& info) const
    {
        if (info.checkFlag(RENDER_VERTEX_COLOUR)
            || (info.checkFlag(RENDER_POINT_COLOUR) && _mode == GL_POINTS))
        {
            glEnableClientState(GL_COLOR_ARRAY);
        }
		pointvertex_gl_array(m_vertices.data());
		glDrawArrays(_mode, 0, static_cast<GLsizei>(m_vertices.size()));
	}
};

/// Renderable wrapper for a set of vertices and indices stored in other arrays
class RenderableIndexBuffer : public OpenGLRenderable
{
	const GLenum _mode;
	const IndexBuffer& m_indices;
	const std::vector<PointVertex>& m_vertices;
public:
	RenderableIndexBuffer(GLenum mode, const IndexBuffer& indices, const std::vector<PointVertex>& vertices)
			: _mode(mode), m_indices(indices), m_vertices(vertices) {}

	void render(const RenderInfo& info) const
    {
        if (info.checkFlag(RENDER_VERTEX_COLOUR)
            || (info.checkFlag(RENDER_POINT_COLOUR) && _mode == GL_POINTS))
        {
            glEnableClientState(GL_COLOR_ARRAY);
        }
		pointvertex_gl_array(m_vertices.data());
		glDrawElements(_mode, GLsizei(m_indices.size()), RenderIndexTypeID, m_indices.data());
	}
};


class RemapXYZ {
public:
	static void set(Vertex3f& vertex, float x, float y, float z) {
		vertex.x() = x;
		vertex.y() = y;
		vertex.z() = z;
	}
};

class RemapYZX {
public:
	static void set(Vertex3f& vertex, float x, float y, float z) {
		vertex.x() = z;
		vertex.y() = x;
		vertex.z() = y;
	}
};

class RemapZXY {
public:
	static void set(Vertex3f& vertex, float x, float y, float z) {
		vertex.x() = y;
		vertex.y() = z;
		vertex.z() = x;
	}
};

template<typename remap_policy>
inline void draw_circle(const std::size_t segments, const float radius, PointVertex* vertices, remap_policy remap) {
	const double increment = c_pi / double(segments << 2);

	std::size_t count = 0;
	float x = radius;
	float y = 0;
	while(count < segments) {
		PointVertex* i = vertices + count;
		PointVertex* j = vertices + ((segments << 1) - (count + 1));

		PointVertex* k = i + (segments << 1);
		PointVertex* l = j + (segments << 1);

		PointVertex* m = i + (segments << 2);
		PointVertex* n = j + (segments << 2);
		PointVertex* o = k + (segments << 2);
		PointVertex* p = l + (segments << 2);

		remap_policy::set(i->vertex, x,-y, 0);
		remap_policy::set(k->vertex,-y,-x, 0);
		remap_policy::set(m->vertex,-x, y, 0);
		remap_policy::set(o->vertex, y, x, 0);

		++count;

		{
			const double theta = increment * count;
			x = static_cast<float>(radius * cos(theta));
			y = static_cast<float>(radius * sin(theta));
		}

		remap_policy::set(j->vertex, y,-x, 0);
		remap_policy::set(l->vertex,-x,-y, 0);
		remap_policy::set(n->vertex,-y, x, 0);
		remap_policy::set(p->vertex, x, y, 0);
	}
}

#if 0
class PointVertexArrayIterator {
	PointVertex* m_point;
public:
	PointVertexArrayIterator(PointVertex* point)
			: m_point(point) {}
	PointVertexArrayIterator& operator++() {
		++m_point;
		return *this;
	}
	PointVertexArrayIterator operator++(int) {
		PointVertexArrayIterator tmp(*this);
		++m_point;
		return tmp;
	}
	Vertex3f& operator*() {
		return m_point.vertex;
	}
	Vertex3f* operator->() {
		return &(operator*());
	}
}

template<typename remap_policy, typename iterator_type
inline void draw_circle(const std::size_t segments, const float radius, iterator_type start, remap_policy remap) {
	const float increment = c_pi / (double)
	(segments << 2);

	std::size_t count = 0;
	iterator_type pxpy(start);
	iterator_type pypx(pxpy + (segments << 1));
	iterator_type pynx(pxpy + (segments << 1));
	iterator_type nxpy(pypx + (segments << 1));
	iterator_type nxny(pypx + (segments << 1));
	iterator_type nynx(nxpy + (segments << 1));
	iterator_type nypx(nxpy + (segments << 1));
	iterator_type pxny(start);
	while(count < segments) {
		const float theta = increment * count;
		const float x = radius * cos(theta);
		const float y = radius * sin(theta);

		remap_policy::set((*pxpy), x, y, 0);
		remap_policy::set((*pxny), x,-y, 0);
		remap_policy::set((*nxpy),-x, y, 0);
		remap_policy::set((*nxny),-x,-y, 0);

		remap_policy::set((*pypx), y, x, 0);
		remap_policy::set((*pynx), y,-x, 0);
		remap_policy::set((*nypx),-y, x, 0);
		remap_policy::set((*nynx),-y,-x, 0);
	}
}

template<typename remap_policy, typename iterator_type
inline void draw_semicircle(const std::size_t segments, const float radius, iterator_type start, remap_policy remap) {
	const float increment = c_pi / (double)
	(segments << 2);

	std::size_t count = 0;
	iterator_type pxpy(start);
	iterator_type pypx(pxpy + (segments << 1));
	iterator_type pynx(pxpy + (segments << 1));
	iterator_type nxpy(pypx + (segments << 1));
	iterator_type nxny(pypx + (segments << 1));
	iterator_type nynx(nxpy + (segments << 1));
	iterator_type nypx(nxpy + (segments << 1));
	iterator_type pxny(start);
	while(count < segments) {
		const float theta = increment * count;
		const float x = radius * cos(theta);
		const float y = radius * sin(theta);

		remap_policy::set((*pxpy), x, y, 0);
		remap_policy::set((*pxny), x,-y, 0);
		remap_policy::set((*nxpy),-x, y, 0);
		remap_policy::set((*nxny),-x,-y, 0);

		//remap_policy::set((*pypx), y, x, 0);
		//remap_policy::set((*pynx), y,-x, 0);
		//remap_policy::set((*nypx),-y, x, 0);
		//remap_policy::set((*nynx),-y,-x, 0);
	}
}


#endif

inline void draw_quad(const float radius, PointVertex* quad) {
	(*quad++).vertex = Vertex3f(-radius, radius, 0);
	(*quad++).vertex = Vertex3f(radius, radius, 0);
	(*quad++).vertex = Vertex3f(radius, -radius, 0);
	(*quad++).vertex = Vertex3f(-radius, -radius, 0);
}

inline void draw_cube(const float radius, PointVertex* cube) {
	(*cube++).vertex = Vertex3f(-radius, -radius, -radius);
	(*cube++).vertex = Vertex3f(radius, -radius, -radius);
	(*cube++).vertex = Vertex3f(-radius, radius, -radius);
	(*cube++).vertex = Vertex3f(radius, radius, -radius);
	(*cube++).vertex = Vertex3f(-radius, -radius, radius);
	(*cube++).vertex = Vertex3f(radius, -radius, radius);
	(*cube++).vertex = Vertex3f(-radius, radius, radius);
	(*cube++).vertex = Vertex3f(radius, radius, radius);
}

#endif
