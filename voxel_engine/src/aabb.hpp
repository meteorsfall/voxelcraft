#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "utils.hpp"

class AABB {
public:
    vec3 min_point;
    vec3 max_point;

    AABB(vec3 min_point, vec3 max_point);

    bool is_point_inside(vec3 p);
    bool is_colliding(AABB other);
    void translate(vec3 change);

    optional<double> test_aabb_plane(vec3 point, vec3 normal);

    optional<vec3> collide(AABB other);
};

#endif
