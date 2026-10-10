#pragma once 

#include <glm/glm.hpp>

#include "Transform.hpp"
#include "Mesh.hpp"

#include <memory>

namespace mk {
	class WorldObject {
	private:
		Transform m_transform = {};
		std::shared_ptr<Mesh> m_nullable_mesh = nullptr;
	public:
		Transform& transform();
		const Transform& transform() const;

		std::shared_ptr<Mesh>& mesh();
		std::shared_ptr<Mesh> const& mesh() const;
	};
}
