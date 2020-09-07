#ifndef _RIGID_BODY_HPP_
#define _RIGID_BODY_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/**
 * This class represents a position and a velocity.
 * It allows users of this class to set a terminal velocity,
 * apply forces to it, and then iterate the physics simulations.
 * It allows users to trigger collisions with the rigid body as well,
 * which will cause the rigid body to slide against various collision planes.
 */

class RigidBody {
public:
    /// Terminal velocity will be ignored if negative, otherwise all velocities will be clamped to it
    float terminal_velocity = -1.0;

    vec3 position = vec3(0.0);
    vec3 velocity = vec3(0.0);

    /** Apply a force to a rigid body, presuming mass = 1
     * 
     * @param acceleration The acceleration to apply to the rigid body
     * @param delta The length of the time-step that will be simulated
     */
    void push(vec3 acceleration, float delta);

    /** Increment the rigid body's motion by the given time-step
     * 
     * @param delta The length of the time-step that will be simulated
     */
    void iterate(float delta);

    /** Collide the rigid body against a plane, and slide against it.
     * This function will exit the collision plane, and adjust its velocity to slide against the collision plane.
     * 
     * @param collision_direction A vector that represents how to be pushed in order to no longer be colliding.
     * Its direction will be normal to the plane,
     * Its length will be how deep into the object the rigid body is
     */
    void collide(vec3 collision_direction);
};

/**@}*/
#endif
