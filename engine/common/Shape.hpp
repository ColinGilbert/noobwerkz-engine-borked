// This represents a conceptual geometric shape, using the physics engine under-the-hood.
// Limitation: btCollisionShape* inner_shape pointer must be manually cleared.
// Reason: The destructor used to do the above, but that unfortunately led to deletion of the btCollisionShape* every time it went out of scope in a local block. This led to all sorts of fun.

#pragma once

#include <vector>
#include <memory>

#include <noob/math/math_funcs.hpp>
#include <noob/component/component.hpp>
#include <rdestl/fixed_array.h>
#include <btBulletDynamicsCommon.h>

#include "Logger.hpp"
#include "Mesh3D.hpp"
#include "MeshUtils.hpp"


namespace noob
{
	class shape 
	{
		friend class compound_shape;
		friend class body;
		friend class ghost;
		public:
		enum class type { SPHERE, BOX, CYLINDER, CONE, CONVEX, TRIMESH };

		shape() noexcept(true) = default; //: physics_valid(false) {} //, scales(noob::vec3f(1.0, 1.0, 1.0)) {}

	// Initializers
	void sphere(float radius) noexcept(true);
	void box(float width, float height, float depth) noexcept(true);
	void cylinder(float radius, float height) noexcept(true);
	void capsule(float radius, float height) noexcept(true); // TODO: Fix
	void cone(float radius, float height) noexcept(true); // TODO: Fix
	void hull(const std::vector<noob::vec3f>&) noexcept(true);
	// Known limitation: The mesh indices MUST be fewer than uint16_t max or else UB...
	// TODO: Fix!
	void trimesh(const noob::mesh_3d&) noexcept(true);

	void set_margin(float) noexcept(true);
	float get_margin() const noexcept(true);

	noob::shape::type get_type() const noexcept(true);

	// TODO: Remove the scales member and use btCollisionShape directly.
	noob::vec3f get_scales() const noexcept(true);

	// TODO: Move out!
	noob::mesh_3d get_mesh() const noexcept(true);

	void clear() noexcept(true);

	void set_self_index(uint32_t) noexcept(true);
	uint32_t get_self_index() const noexcept(true);

	const btCollisionShape* get_inner() const noexcept(true);
	// btCollisionShape* get_inner_mutable() const noexcept(true);	

		protected:
	// void plane(const noob::vec3f& normal, float offset) noexcept(true);

	noob::shape::type shape_type;
	bool physics_valid = false;
	noob::vec3f scales = noob::vec3f(1.0, 1.0, 1.0);
	btCollisionShape* inner;
	};

	typedef noob::handle<shape> shape_handle;
}
