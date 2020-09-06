#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "utils.hpp"

// *********
// AABB Represents an axis-aligned bounding box
// AABB's can 
// *********

class AABB {
public:
    vec3 min_point;
    vec3 max_point;

    // Will create an AABB with the given bottom-left point and top-right point
    AABB(vec3 min_point, vec3 max_point);

    // Return true if the point p is inside the AABB
    bool test_point(vec3 p);
    // Returns true if this AABB is colliding with another AABB
    bool is_colliding(const AABB& other);
    // Will translate an AABB
    void translate(vec3 change);

    // Returns true if the given plane intersects the AABB
    bool test_plane(vec3 point, vec3 normal);

    // Check if frustum may be intersecting the AABB (For frustum culling)
    // Note: This test is guaranteed to return true if the AABB actually intersects the frustum
    // However, it might also return true for other cases as well.
    // It will try its best to return false if the AABB is clearly not in the frustum
    bool test_frustum(const mat4& PV);

    // Check if the AABB collides with another AABB, and if it does,
    // It will return a vec3 for the direction that the other AABB must go
    // In order to avoid collision
    optional<vec3> collide(AABB other);
};

#endif
