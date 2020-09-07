#include "entity.hpp"
#include "universe.hpp"

Entity::Entity(int mesh_id) {
    this->mesh_id = mesh_id;
}

void Entity::render(mat4& PV, mat4& M) {
    get_universe()->get_mesh(mesh_id)->render(PV, M);
}
