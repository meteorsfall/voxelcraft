#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

#include "mesh.hpp"

class Entity {
public:
    int mesh_id;
    Entity(int mesh_id);
    void render(mat4& PV, mat4& M);
};

#endif