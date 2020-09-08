#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include "mesh.hpp"
#include "world.hpp"
#include "aabb.hpp"
#include "rigid_body.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// The Entity class represents a renderable mesh instance with internal state.
/**
 * Entities are renderable objects that intend to be any possible mesh, including meshes
 * with animations. They hold state and properties, and can subscribe to events.
 * They can modify the world around them and interact with other entities or blocks.
 * They are the most generic object type in the VoxelEngine.
 */

class Entity {
public:
    /// Creates a new Entity
    Entity();

    /// The texture ID that this entity will render with
    int texture_id;
    /// The mesh ID that this entity will render with
    int mesh_id;

    /// The RigidBody that controls the physics for this entity
    RigidBody body;
    /// The bounding box for this entity (Presuming that the entity is to be centered at the origin, get_aabb() will translate the AABB)
    AABB aabb;

    /// The model matrix that controls this entities transformations from the origin
    mat4 model_matrix;

    /// Gets an AABB for this entity
    AABB get_aabb();

    /// Renders the Entity
    void render(mat4& PV);

    /// Gets the on_collision callback for this entity
    fn_on_collide get_on_collide();
};

/**@}*/

#endif
