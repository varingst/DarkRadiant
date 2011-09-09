#pragma once

#include "imodel.h"
#include "imd5model.h"
#include "math/AABB.h"
#include <vector>
#include "parser/DefTokeniser.h"

#include "MD5Surface.h"
#include "RenderableMD5Skeleton.h"

namespace md5
{

// generic model node
class MD5Model :
	public IMD5Model,
	public model::IModel
{
private:
	typedef std::vector<MD5SurfacePtr> SurfaceList;
	SurfaceList _surfaces;

	AABB _aabb_local;

	// Gets initialised during parsing
	std::size_t _polyCount;
	std::size_t _vertexCount;

	// The list of shader names for this model (for ModelSelector)
	std::vector<std::string> _surfaceNames;

	// The filename of this model
	std::string _filename;

	// The VFS path to this model
	std::string _modelPath;

	// The animation which is currently active on this model
	IMD5AnimPtr _anim;

	// The current state of our animated skeleton
	MD5Skeleton _skeleton;

	// The OpenGLRenderable visualising the MD5Skeleton
	RenderableMD5Skeleton _renderableSkeleton;

public:
	MD5Model();

	typedef SurfaceList::const_iterator const_iterator;

	// Public iterator-related methods
	const_iterator begin() const;
	const_iterator end() const;
	std::size_t size() const;

	/** greebo: Reads the model data from the given tokeniser.
	 */
	void parseFromTokens(parser::DefTokeniser& tok);

	RenderableMD5Skeleton& getRenderableSkeleton()
	{
		return _renderableSkeleton;
	}

	void updateAABB();

	virtual const AABB& localAABB() const;

	void testSelect(Selector& selector, SelectionTest& test, const Matrix4& localToWorld);

	// Sets the filename this model was loaded from
	void setFilename(const std::string& name);

	// IModel implementation
	virtual std::string getFilename() const;

	virtual std::string getModelPath() const;
	void setModelPath(const std::string& modelPath);

	virtual void applySkin(const ModelSkin& skin);

	/** Return the number of material surfaces on this model. Each material
	 * surface consists of a set of polygons sharing the same material.
	 */
	virtual int getSurfaceCount() const;

	/** Return the number of vertices in this model, equal to the sum of the
	 * vertex count from each surface.
	 */
	virtual int getVertexCount() const;

	/** Return the number of triangles in this model, equal to the sum of the
	 * triangle count from each surface.
	 */
	virtual int getPolyCount() const;

	/** Return a vector of strings listing the active materials used in this
	 * model, after any skin remaps. The list is owned by the model instance.
	 */
	virtual const std::vector<std::string>& getActiveMaterials() const;

	const model::IModelSurface& getSurface(int surfaceNum) const;

	// OpenGLRenderable implementation
	virtual void render(const RenderInfo& info) const;

	void setRenderSystem(const RenderSystemPtr& renderSystem);

	// IMD5Model implementation
	virtual void setAnim(const IMD5AnimPtr& anim);
	virtual const IMD5AnimPtr& getAnim() const;
	virtual void updateAnim(std::size_t time);

	/**
	 * Helper: Parse an MD5 vector, which consists of three separated numbers
	 * enclosed with parentheses.
	 */
	static Vector3 parseVector3(parser::DefTokeniser& tok);

private:

	// Creates a new MD5Surface, adds it to the local list and returns the reference
	MD5Surface& newSurface();

	// Re-populates the list of active shader names
	void updateMaterialList();
};
typedef boost::shared_ptr<MD5Model> MD5ModelPtr;

} // namespace
