import { degToRad } from './math/Math';
import { Matrix4 } from './math/Matrix4';
import {Vector2} from './math/Vector2';
import {Vector3} from './math/Vector3';
import {print} from './api';

class Camera {
    position: Vector3 = new Vector3(0);
    horizontal_angle: f32 = 3.14;
    vertical_angle: f32 = 0;
    fov: f32 = 75;

    get_direction(): Vector3 {
        let direction: Vector3 = new Vector3(
            NativeMathf.cos(this.vertical_angle) * NativeMathf.sin(this.horizontal_angle),
            NativeMathf.sin(this.vertical_angle),
            NativeMathf.cos(this.vertical_angle) * NativeMathf.cos(this.horizontal_angle)
        );
        return direction.normalize();
    }

    get_right(): Vector3 {
        return new Vector3(
            NativeMathf.sin(this.horizontal_angle - NativeMathf.PI/2.0),
            0,
            NativeMathf.cos(this.horizontal_angle - NativeMathf.PI/2.0)
        );
    }

    get_up(): Vector3 {
        return this.get_right().cross(this.get_direction());
    }

    move_toward(change: Vector3, clip_y: boolean): void {
        let direction = this.get_direction();
        let right = this.get_right();

        let change_pos: Vector3 = direction.multiplyScalar(change.x);
        change_pos.y += change.y;
        change_pos = change_pos.add(right.multiplyScalar(change.z));
        if (clip_y) {
            change_pos.y = 0;
        }
        this.position = this.position.add(change_pos);
    }

    rotate(change: Vector2): void {
        this.horizontal_angle += change.x;
        this.vertical_angle += change.y;
        let min: f32 = -NativeMathf.PI / 2.0 + 0.01;
        let max: f32 = NativeMathf.PI / 2.0 - 0.01;
        if (this.vertical_angle < min) {
            this.vertical_angle = min;
        }
        if (this.vertical_angle > max) {
            this.vertical_angle = max;
        }
    }

    get_camera_projection_matrix(aspect_ratio: f32): Matrix4 {
        let proj: Matrix4 = new Matrix4();
        proj.makePerspective(degToRad(this.fov), aspect_ratio, 0.1, 230);
        return proj;
    }

    get_camera_view_matrix(): Matrix4 {
        let direction: Vector3 = this.get_direction();
        let right: Vector3 = this.get_right();
        let up: Vector3 = right.cross(direction);

        let view: Matrix4 = new Matrix4();
        
        view.lookAt(this.position, direction.add(this.position), up);
        
        return view;
    }
}

export {Camera};
