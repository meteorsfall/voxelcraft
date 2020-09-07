#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include "mesh.hpp"
#include "world.hpp"
#include "aabb.hpp"
#include "rigid_body.hpp"

class Entity {
public:
    int texture_id;
    int mesh_id;

    RigidBody body;

    mat4 model_matrix;
    Entity();
    void render(mat4& PV);
    AABB get_aabb();
    fn_on_collide get_on_collide();
};

#endif
