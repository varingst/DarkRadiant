#ifndef ECLASSMODELNODE_H_
#define ECLASSMODELNODE_H_

#include "nameable.h"
#include "inamespace.h"
#include "modelskin.h"
#include "ientity.h"

#include "scenelib.h"
#include "transformlib.h"
#include "instancelib.h"

#include "EclassModel.h"

namespace entity {

class EclassModelNode :
	public scene::Node,
	public scene::Instantiable,
	public scene::Cloneable,
	public Nameable,
	public Snappable,
	public TransformNode,
	public TraversableNodeSet, // implements scene::Traversable
	public EntityNode,
	public Namespaced
{
	InstanceSet m_instances;
	EclassModel m_contained;

public:
	// Constructor
	EclassModelNode(IEntityClassPtr eclass);
	// Copy Constructor
	EclassModelNode(const EclassModelNode& other);

	~EclassModelNode();

	// Snappable implementation
	virtual void snapto(float snap);

	// TransformNode implementation
	virtual const Matrix4& localToParent() const;

	// EntityNode implementation
	virtual Entity& getEntity();

	// Namespaced implementation
	virtual void setNamespace(INamespace& space);

	scene::INodePtr clone() const;

	scene::Instance* create(const scene::Path& path, scene::Instance* parent);
	void forEachInstance(const scene::Instantiable::Visitor& visitor);
	void insert(const scene::Path& path, scene::Instance* instance);
	scene::Instance* erase(const scene::Path& path);

	// Nameable implementation
	virtual std::string name() const;
	virtual void attach(const NameCallback& callback);
	virtual void detach(const NameCallback& callback);

private:
	void construct();
	void destroy();
};

} // namespace entity

#endif /*ECLASSMODELNODE_H_*/
