import vec3;
import vec2;
import mat4;

const float MOVEMENT_SPEED = 4.5;
const float MOVEMENT_FALLING_SPEED = 3.5;
const float FLYING_ACCEL_SPEED = 6.0;
const float FLYING_MAX_SPEED = 54.0;
const float MOUSE_SPEED = 0.1;
const float JUMP_VELOCITY = 6.0;

class Player {
    int hand;
    int[] hotbar;
    int[] inventory;
    bool is_flying;
    bool is_on_floor;

    vec3 position;
    vec3 velocity;
    float flying_speed;
    float horizontal_angle;
    float vertical_angle;

    init();
}
implement Player {
    init() {
        this.is_flying = true;

        this.position = new vec3(16.0 / 2.0, 16.0 + 1.0, 16.0 / 2.0);
        this.velocity = new vec3(0.0, 0.0, 0.0);
        this.horizontal_angle = 3.14;
        this.vertical_angle = 0.0;
        this.is_on_floor = false;
    }

    void set_fly(bool is_flying) {
        this.is_flying = is_flying;
    }

    // Get look directions
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

    void move_toward(vec3 change, float delta_time) {
        if (this.is_flying) {
            if (change.x == 0.0 && change.y == 0.0 && change.z == 0.0) {
                this.velocity = new vec3(0.0, 0.0, 0.0);
                this.flying_speed = MOVEMENT_SPEED;
            } else {
                this.flying_speed += FLYING_ACCEL_SPEED * delta_time;
                if (this.flying_speed > FLYING_MAX_SPEED) {
                    this.flying_speed = FLYING_MAX_SPEED;
                }
                change = change.normalize().times(this.flying_speed);
            }
        } else {
            change.y = 0.0;
            if (change.length() > 0.0) {
                change = change.normalize().times((this.is_on_floor ? MOVEMENT_SPEED : MOVEMENT_FALLING_SPEED));
            }
        }

        change = change.times(delta_time);
        
        vec3 direction = this.get_direction();
        vec3 right = this.get_right();

        vec3 change_pos = direction.times(change.x);
        change_pos.y += change.y;
        change_pos = change_pos.add(right.times(change.z));

        if (!this.is_flying) {
            change_pos.y = 0.0;
            if (change_pos.length() > 0.01) {
                change_pos = change_pos.normalize().times(this.velocity.length());
            }
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

    mat4 get_projection_matrix(float aspect_ratio, float fov) {
        mat4 proj = new mat4(1.0);
        proj = proj.make_perspective(math.deg2rad(fov), aspect_ratio, 0.1, 230.0);
        return proj;
    }

    mat4 get_view_matrix() {
        vec3 direction = this.get_direction();
        vec3 right = this.get_right();
        vec3 up = right.cross(direction);
        mat4 view = new mat4(1.0);
        // Camera position is 1.67 above this.position, as this.position refers to the bottom of the player
        vec3 camera_position = this.position.add(new vec3(0.0, 1.67, 0.0));
        view = view.look_at(camera_position, camera_position.add(direction), up);
        return view;
    }
}

export {Player};
