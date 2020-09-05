#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "utils.hpp"

class AABB {
public:
    vec3 min_point;
    vec3 max_point;

    AABB(vec3 min_point, vec3 max_point);

    bool test_point(vec3 p);
    bool is_colliding(const AABB& other);
    void translate(vec3 change);

    optional<double> test_plane(vec3 point, vec3 normal);

    // Check if frustum intersects the AABB
    // Note: This test may have rare false positives, claiming an interesection when there is none.
    //       If this test returns false, then it is guaranteed that the AABB is not in the given frustum
    bool test_frustum(const mat4& PV);

    optional<vec3> collide(AABB other);
};

#endif
