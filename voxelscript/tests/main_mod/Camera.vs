/*
import { degToRad } from './math/Math';
import { Matrix4 } from './math/Matrix4';
import {Vector2} from './math/Vector2';
import {vec3} from './math/vec3';
import {print} from './api';
*/

import vec2;
import vec3;
import mat4;

class Camera {
    vec3 position;
    float horizontal_angle;
    float vertical_angle;
    float fov;

    init();
}

implement Camera {
    init() {
        this.position = new vec3(0, 0, 0);
        this.horizontal_angle = 3.14;
        this.vertical_angle = 0.0;
        this.fov = 75.0;
    }

    vec3 get_direction() {
        vec3 direction = new vec3(
            math.cos(this.vertical_angle) * math.sin(this.horizontal_angle),
            math.sin(this.vertical_angle),
            math.cos(this.vertical_angle) * math.cos(this.horizontal_angle)
        );
        return direction.normalize();
    }

    vec3 get_right() {
        return new vec3(
            math.sin(this.horizontal_angle - math.pi/2.0),
            0.0,
            math.cos(this.horizontal_angle - math.pi/2.0)
        );
    }

    vec3 get_up() {
        return this.get_right().cross(this.get_direction());
    }

    void move_toward(vec3 change, bool clip_y) {
        vec3 direction = this.get_direction();
        vec3 right = this.get_right();
        vec3 change_pos = direction.times(change.x);
        change_pos.y += change.y;
        change_pos = change_pos.add(right.times(change.z));
        if (clip_y) {
            change_pos.y = 0.0;
        }
        this.position = this.position.add(change_pos);
    }

    void rotate(vec2 change) {
        this.horizontal_angle += change.x;
        this.vertical_angle += change.y;
        float min = -math.pi / 2.0 + 0.01;
        float max = math.pi / 2.0 - 0.01;
        if (this.vertical_angle < min) {
            this.vertical_angle = min;
        }
        if (this.vertical_angle > max) {
            this.vertical_angle = max;
        }
    }

    mat4 get_camera_projection_matrix(float aspect_ratio) {
        mat4 proj = new mat4(1.0);
        proj = proj.make_perspective(math.deg2rad(this.fov), aspect_ratio, 0.1, 230.0);
        return proj;
    }

    mat4 get_camera_view_matrix() {
        vec3 direction = this.get_direction();
        vec3 right = this.get_right();
        vec3 up = right.cross(direction);
        mat4 view = new mat4(1.0);
        view = view.look_at(this.position, direction.add(this.position), up);
        return view;
    }
}

export {Camera};
