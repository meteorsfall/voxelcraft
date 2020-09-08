#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include "utils.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/**
 * The Camera class represents the camera that views the world around it
 */

class Camera {
public:
    /// Create a default camera. FOV of 45 degrees, with the camera pointed in the -x direction.
    Camera();

    /// The Camera's position
    vec3 position;

    /// Get the direction vector of what the camera is looking at relative to the camera itself. This will be orthogonal to the viewing plane.
    vec3 get_direction();
    /// Get a direction vector pointing right with respect to the viewing plane
    vec3 get_right();
    /// Get a direction pointing vertical with respect to the viewing plane
    vec3 get_up();

    /// Move the camera in the direction of this.get_direction() 
    /**
     * 
     * @param change The vector that represents how the camera will move.
     * change.x is how far to move forwards, change.y is how far to move up or down,
     * and change.z is how far to move to the right. change.x will use get_direction(),
     * and change.z will use get_right(). However, change.y will only control movement up or down along the y-axis.
     * change.y will not use get_up().
     * 
     * @param clip_y If true, the camera will not move up or down. For example, if clip_y is false, and change.x indicates that
     * the user wants to move in the direction of get_direction(), then the camera's y-coordinate will increase
     * if the camera is tilted upwards. This is desirable in creative mode, but not survival mode.
     */
    void move_toward(vec3 change, bool clip_y);

    /// Rotate the camera's viewing plane
    /**
     * 
     * @param delta delta.x will represent how much to rotate left or right (the yaw),
     * and delta.y will represent how much to tilt up or down (the pitch). delta.x if positive,
     * will rotate to the right. The pitch will be clamped to between -pi/2 + 0.01, and pi/2 - 0.01.
     * This is to prevent bending the neck too far up or down, and to prevent gimbal lock at multiples of pi/2
     */
    void rotate(vec2 delta);

    /// Get a matrix that represents the Camera's projection matrix
    mat4 get_camera_projection_matrix(float aspect_ratio);
    /// Get a matrix that represents the Camera's rotation and position
    mat4 get_camera_view_matrix();
private:
    GLfloat horizontal_angle;
    GLfloat vertical_angle;
    GLfloat fov;
};

/**@}*/

#endif
