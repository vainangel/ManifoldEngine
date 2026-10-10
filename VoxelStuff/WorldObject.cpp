#include "WorldObject.hpp"

using namespace mk;

Transform& WorldObject::transform() { return m_transform; }

const Transform& WorldObject::transform() const { return m_transform; }

std::shared_ptr<Mesh>& WorldObject::mesh() { return m_nullable_mesh; }

std::shared_ptr<Mesh> const& WorldObject::mesh() const { return m_nullable_mesh; }


