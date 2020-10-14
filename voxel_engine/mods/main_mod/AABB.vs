import vec3;
import Optional;

class AABB {
    vec3 min_point;
    vec3 max_point;
    init(vec3 min_point, vec3 max_point);
}
implement AABB {
    init(vec3 min_point, vec3 max_point) {
        this.min_point = min_point;
        this.max_point = max_point;
    }

    void translate(vec3 change) {
        this.min_point = this.min_point.add(change);
        this.max_point = this.max_point.add(change);
    }
    
    bool is_colliding(AABB other) {
        // If our max_point is underneath the other's min_point, then we're obviously not going to collide
        bool max_invalid = this.max_point.x <= other.min_point.x
                        || this.max_point.y <= other.min_point.y
                        || this.max_point.z <= other.min_point.z;
        // If our min_point is above their max_point, then we're obviously not going to collide
        bool min_invalid = other.max_point.x <= this.min_point.x
                        || other.max_point.y <= this.min_point.y
                        || other.max_point.z <= this.min_point.z;

        return !(min_invalid || max_invalid);
    }

    Optional<vec3> collide(AABB other) {
        if (!this.is_colliding(other)) {
            return new Optional<vec3>();
        }

        vec3 dimensions = this.max_point.sub(this.min_point);

        //    
        //         V-- right_needed_change = this.max.x - other.min.x
        //         V 
        // |--------|
        // | this |-|-------|
        // |--------| other |
        //        |---------|
        //
        // In other words, right_needed_change is how far right the other AABB
        // needs to move, in order to no longer be colliding with our AABB
        // 

        // We found out how much need to move in each direction
        float left_needed_change = -math.min(this.min_point.x - other.max_point.x, 0.0);
        float right_needed_change =  math.max(this.max_point.x - other.min_point.x, 0.0);
        float top_needed_change = -math.min(this.min_point.y - other.max_point.y, 0.0);
        float bottom_needed_change = math.max(this.max_point.y - other.min_point.y, 0.0);
        float forward_needed_change = -math.min(this.min_point.z - other.max_point.z, 0.0);
        float backward_needed_change = math.max(this.max_point.z - other.min_point.z, 0.0);

        float[] needed_changes = [
            left_needed_change,
            right_needed_change,
            top_needed_change,
            bottom_needed_change,
            forward_needed_change,
            backward_needed_change
        ];

        vec3[] directions = [
            new vec3(-1.0, 0.0, 0.0),
            new vec3(1.0, 0.0, 0.0),
            new vec3(0.0, -1.0, 0.0),
            new vec3(0.0, 1.0, 0.0),
            new vec3(0.0, 0.0, -1.0),
            new vec3(0.0, 0.0, 1.0)
        ];

        bool found_move = false;
        float best_move_distance = 0.0;
        vec3 possible_move = new vec3(0.0, 0.0, 0.0);

        // Find the shortest distance to shove the other AABB out of this AABB
        for(int i = 0; i < needed_changes.size(); i++) {
            float move_distance = needed_changes[i];
            vec3 move_dir = directions[i];

            // If right_needed_change > 0, and its less than the width,
            // Then it's a valid move to shove the other AABB to get it out of us
            // abs(dot(dimensions, move_dir)) tells us how wide this AABB is in that direction
            if (0.0 < move_distance && move_distance < math.abs(dimensions.dot(move_dir))) {
                // If we haven't found a move, this is our new move
                // If we have, but this move is a shorter distance,
                // then it's the better move
                if (!found_move || move_distance < best_move_distance) {
                    found_move = true;
                    // Keep track of the best move distance
                    best_move_distance = move_distance;
                    // The move is how far we have to move, times the direction we need to move in
                    possible_move = move_dir.times(move_distance);
                }
            }
        }

        // If we found a move, we return how we need to move to push the other AABB out of this AABB
        if (found_move) {
            Optional<vec3> ret = new Optional<vec3>();
            ret.set_value(possible_move);
            return ret;
        } else {
            return new Optional<vec3>();
        }
    }
}

export {AABB};
