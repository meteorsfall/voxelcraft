#ifndef _AABB_HPP_
#define _AABB_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// AABB represents an axis-aligned bounding box, AABB's can check against collisions and simulate collision events.

class AABB {
public:
    /// The bottom-left point of the AABB
    vec3 min_point;
    /// The top-right point of the AABB
    vec3 max_point;

    /// Creates an AABB with the given bottom-left point and top-right point
    AABB(vec3 min_point, vec3 max_point);

    /// Returns true if the point p is inside the AABB
    bool test_point(vec3 p);
    /// Returns true if this AABB is colliding with another AABB
    bool is_colliding(const AABB& other);
    /// Will translate the current AABB
    void translate(vec3 change);

    /// Returns true if the given plane intersects the AABB
    bool test_plane(vec3 point, vec3 normal);

    /// Check if frustum could be intersecting the AABB (For frustum culling)
    /** Note: This a quick but weak intersection test.
     * It is guaranteed to return true if the AABB actually does intersect the frustum.
     * However, it may also return true even if the AABB does not intersect the frustum.
     * It will indeed try its best to return false if the AABB is clearly not in the frustum however.
     */
    bool test_frustum(const mat4& PV);

    /// Check if two AABBs have collided, and returns a vector for how to correct the collision
    /** If the two AABBs collide, it will return a vec3 for the translation that the other AABB
     * must do in order to force itself out of the collision
     */
    optional<vec3> collide(AABB other);
};

/**@}*/

#endif
