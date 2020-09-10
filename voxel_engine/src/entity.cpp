#include "universe.hpp"
#include "entity.hpp"

Entity::Entity() {
    body.position = vec3(-2.0, 16.4 + 0.05 * sin(radians(glfwGetTime() * 360 / 3)), 0.0);
    float width = 0.5f;
    float height = 1.0f;
    aabb = AABB(-vec3(width, 0, width), vec3(width, height, width));
}

AABB Entity::get_aabb() {
    AABB ret = this->aabb;
    ret.translate(body.position);
    return ret;
}

fn_on_collide Entity::get_on_collide() {
    return [this](vec3 collision_normal, float coefficient_of_friction) {
        body.collide(collision_normal, coefficient_of_friction);
    };
}

void Entity::render(const mat4& PV) {
    UNUSED(PV);
    mat4 model = model_matrix;
    model[3][0] += body.position.x;
    model[3][1] += body.position.y;
    model[3][2] += body.position.z;
    //get_universe()->get_mesh(mesh_id)->render(PV, model);
}
