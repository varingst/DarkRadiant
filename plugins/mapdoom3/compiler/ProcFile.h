#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>
#include "ibrush.h"
#include "ipatch.h"
#include "ishaders.h"

#include "math/AABB.h"
#include "render/ArbitraryMeshVertex.h"

#include "PlaneSet.h"
#include "ProcWinding.h"
#include "ProcLight.h"
#include "ProcBrush.h"
#include "BspTree.h"

namespace model { class IModelSurface; }
class IPatch;

namespace map
{

struct HashVertex;
struct OptEdge;
typedef boost::shared_ptr<OptEdge> OptEdgePtr;
struct HashVert;

struct OptVertex
{
	ArbitraryMeshVertex	v;
	Vector3				pv;					// projected against planar axis, third value is 0

	OptEdge*			edges;
	struct OptVertex*	islandLink;
	bool				addedToIsland;
	bool				emitted;			// when regenerating triangles

	OptVertex() :
		edges(NULL),
		islandLink(NULL),
		addedToIsland(false),
		emitted(false)
	{}
};

struct OptEdge
{
	OptVertex*		v1;
	OptVertex*		v2;
	struct OptEdge*	islandLink;
	bool			addedToIsland;
	bool			created;		// not one of the original edges
	bool			combined;		// combined from two or more colinear edges
	struct OptTri*	frontTri;
	struct OptTri*	backTri;
	struct OptEdge*	v1link;
	struct OptEdge*	v2link;

	void linkToVertices()
	{
		v1link = v1->edges;
		v1->edges = this;

		v2link = v2->edges;
		v2->edges = this;
	}
};

struct OriginalEdge
{
	OptVertex*	v1;
	OptVertex*	v2;

	OriginalEdge()
	{}

	OriginalEdge(OptVertex* v1_, OptVertex* v2_) :
		v1(v1_),
		v2(v2_)
	{}
};

struct EdgeCrossing
{
	OptVertex*	ov;

	EdgeCrossing() :
		ov(NULL)
	{}

	EdgeCrossing(OptVertex* ov_) :
		ov(ov_)
	{}
};
typedef std::vector<EdgeCrossing> EdgeCrossings;
typedef std::vector<EdgeCrossings> EdgeCrossingsList;

struct OptTri
{
	Vector3		midpoint;
	OptVertex*	v[3];
	bool		filled;

	OptTri() :
		filled(false)
	{
		v[0] = v[1] = v[2] = NULL;
	}
};
typedef boost::shared_ptr<OptTri> OptTriPtr;

// chains of ProcTri are the general unit of processing
struct ProcTri
{
	MaterialPtr					material;
	const ProcFace*				mergeGroup;		// we want to avoid merging triangles
	const model::IModelSurface* mergeSurf;		// from different fixed groups, like guiSurfs and mirrors
	const IPatch*				mergePatch;
	std::size_t					planeNum;

	ArbitraryMeshVertex			v[3];

	const HashVert*				hashVert[3];	// for T-junction pass
	OptVertex*					optVert[3];		// for optimization

	ProcTri() :
		mergeGroup(NULL),
		mergeSurf(NULL),
		mergePatch(NULL)
	{
		optVert[0] = optVert[1] = optVert[2] = NULL;
		hashVert[0] = hashVert[1] = hashVert[2] = NULL;
	}
};
typedef std::vector<ProcTri> ProcTris;

#define MAX_GROUP_LIGHTS 16

struct ProcOptimizeGroup
{
	AABB				bounds;			// set in CarveGroupsByLight

	// all of these must match to add a triangle to the triList
	bool				smoothed;		// curves will never merge with brushes
	std::size_t			planeNum;
	std::size_t			areaNum;
	MaterialPtr			material;
	int					numGroupLights;
	ProcLight			groupLights[MAX_GROUP_LIGHTS];	// lights effecting this list
	const ProcFace*		mergeGroup;			// if this differs (guiSurfs, mirrors, etc), the
	const model::IModelSurface* mergeSurf;	// groups will not be combined into model surfaces
	const IPatch*		mergePatch;			// after optimization
	Vector4				texVec[2];

	bool				surfaceEmitted;

	ProcTris			triList;
	ProcTris			regeneratedTris;	// after each island optimization
	Vector3				axis[2];			// orthogonal to the plane, so optimization can be 2D
};

struct ProcArea
{
	typedef std::vector<ProcOptimizeGroup> OptimizeGroups;
	OptimizeGroups	groups;
};

// A primitive can either be a brush or a patch,
// so only one of the pointers is non-NULL
struct ProcPrimitive
{
	ProcBrushPtr	brush;
	ProcTris		patch;	// this is empty for brushes
};

struct ProcEntity
{
	// The reference into the scenegraph
	IEntityNodePtr	mapEntity;

	std::size_t		entityNum;

	Vector3			origin;

	// Each entity has 0..N primitives
	typedef std::vector<ProcPrimitive> Primitives;
	Primitives		primitives;

	BspTree			tree;

	std::size_t		numAreas;

	typedef std::vector<ProcArea> Areas;
	Areas			areas;	// populated in putPrimitiveInAreas()

	ProcEntity(const IEntityNodePtr& entityNode, std::size_t entityNum_) :
		mapEntity(entityNode),
		entityNum(entityNum_),
		numAreas(0)
	{}
};

struct ProcInterAreaPortal
{
	std::size_t area0;
	std::size_t area1;
	ProcFace*	side;
};

class LeakFile;
typedef boost::shared_ptr<LeakFile> LeakFilePtr;

/**
 * This class represents the processed data (entity models and shadow volumes)
 * as generated by the dmap compiler. Use the saveToFile() method to write the
 * data into the .proc file.
 */
class ProcFile
{
public:
	static const char* const FILE_ID;

	typedef std::vector<ProcEntityPtr> ProcEntities;
	ProcEntities entities;

	// All the planes in the map
	PlaneSet planes;

	std::size_t numPortals;
	std::size_t numPatches;
	std::size_t numWorldBrushes;
	std::size_t numWorldTriSurfs;

	AABB mapBounds;

	typedef std::vector<ProcLight> ProcLights;
	ProcLights lights;

	LeakFilePtr leakFile;

	typedef std::vector<ProcInterAreaPortal> InterAreaPortals;
	InterAreaPortals interAreaPortals;

	ProcFile() :
		numPortals(0),
		numPatches(0),
		numWorldBrushes(0),
		numWorldTriSurfs(0)
	{}

	void saveToFile(const std::string& path);

	bool hasLeak() const
	{
		return leakFile;
	}

	static const char* const Extension()
	{
		return ".proc";
	}

private:
	std::ostream& writeProcEntity(std::ostream& str, ProcEntity& entity);
	std::ostream& writeOutputPortals(std::ostream& str, ProcEntity& entity);
	std::ostream& writeOutputNodes(std::ostream& str, const BspTreeNodePtr& node);
	std::ostream& writeOutputNodeRecursively(std::ostream& str, const BspTreeNodePtr& node);
	std::ostream& writeShadowTriangles(std::ostream& str, const Surface& shadowTris);
};
typedef boost::shared_ptr<ProcFile> ProcFilePtr;

} // namespace
