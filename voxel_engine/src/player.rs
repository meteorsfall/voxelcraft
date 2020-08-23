use three_d::*;

type Vector3 = cgmath::vector::Vector3;

struct Player {
    position : Vector3,
    velocity : Vector3,
    direction : Vector3,
}

impl Player {
    pub fn new(position : &Vector3) -> Self {
        return Player {
            posiion: position,
            velocity: vec3(0.0, 0.0, 0.0),
        };
    }
    
    // move(vel, accel) => First move 
    pub fn move(velocity : &Vector3, accel : &Vector3, delta : f32) {
        // Move the player
        self.position += velocity * delta;

        // Move the internal velocity vector based on acceleration
        self.velocity += accel * delta;
        self.position += self.velocity * delta;
    }

    pub fn set_position(position : &Vector3) {
        self.position = position;
    }

    pub fn set_velocity(velocity : &Vector3) {
        self.velocity = velocity;
    }

    pub fn get_position() -> Vector3 {
        return self.position;
    }

    pub fn get_velocity() -> Vector3 {
        return self.velocity;
    }
    
    pub fn rotate(theta : &Vector3) {
        self.direction += theta;
    }
    
    pub fn get_direction() -> Vector3 {
        return self.direction;
    }
}