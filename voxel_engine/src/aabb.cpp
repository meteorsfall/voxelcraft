#include "aabb.hpp"

AABB::AABB(vec3 min_point, vec3 max_point) {
    this->min_point = min_point;
    this->max_point = max_point;
}

bool AABB::test_point(vec3 p) {
    if (min_point.x <= p.x && p.x <= max_point.x
        && min_point.y <= p.y && p.y <= max_point.y
        && min_point.z <= p.z && p.z <= max_point.z
    ) {
        return true;
    } else {
        return false;
    }
}

bool AABB::test_plane(vec3 point, vec3 normal) {
    // Convert AABB to center-extents representation
    vec3 center = (this->max_point + this->min_point) * 0.5f; // Compute AABB center
    vec3 extents = this->max_point - center; // Compute positive extents

    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    float r = dot(extents, abs(point));

    // Compute distance of box center from plane
    float s = dot(normal, center - point);

    // Intersection occurs when distance s falls within [-r,+r] interval
    return abs(s) <= r;
}

// Check if frustum intersects the AABB
// Note: This test may have rare false positives, claiming an interesection when there is none.
bool AABB::test_frustum(const mat4& PV) {

    // Generate all 8 points of the AABB
    vec4 points[8];
    for(int i = 0; i < 8; i++) {
        float x = (i) & 1 ? this->min_point.x : this->max_point.x;
        float y = (i>>1) & 1 ? this->min_point.y : this->max_point.y;
        float z = (i>>2) & 1 ? this->min_point.z : this->max_point.z;

        points[i] = PV * vec4(x, y, z, 1.0);
    }

    // Loop through pos-x, neg-x, pos-y, neg-y, pos-z, neg-z
    // We will check if all 8 points are either entirely to the left of (-1, _, _),
    // Or entirely to the right of (1, _, _), etc. If such a thing is true, we 
    for(int j = 0; j < 6; j++) {
        bool out_of_bounds = true;
        for(int i = 0; i < 8; i++) {
            if (j & 1) {
                // Check if every points.x / points.w <= -1, ie keep the AABB if it's >= -1
                if (points[i][j / 2] >= -points[i][3]) {
                    out_of_bounds = false;
                    break;
                }
            } else {
                // Check if every points.x / points.w >= 1
                if (points[i][j / 2] <= points[i][3]) {
                    out_of_bounds = false;
                    break;
                }
            }
        }
        if (out_of_bounds) {
            // If we're out of bounds, then we don't intersect with the frustum
            return false;
        }
    }

    // Otherwise, we do intersect (Or, at least it's very likely that we do intersect)
    return true;
}

bool AABB::is_colliding(const AABB& other) {
    // If our max_point is undernear the other's min_point, then we're obviously not going to collide
    bool max_invalid = this->max_point.x <= other.min_point.x
                    || this->max_point.y <= other.min_point.y
                    || this->max_point.z <= other.min_point.z;
    // If our min_point is above their max_point, then we're obviously not going to collide
    bool min_invalid = other.max_point.x <= this->min_point.x
                    || other.max_point.y <= this->min_point.y
                    || other.max_point.z <= this->min_point.z;

    // Otherwise, we must be colliding
    return !(min_invalid || max_invalid);
}

void AABB::translate(vec3 change) {
    this->min_point += change;
    this->max_point += change;
}

optional<vec3> AABB::collide(AABB other) {
    if (!is_colliding(other)) {
        return nullopt;
    }

    vec3 dimensions = this->max_point - this->min_point;

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
    float left_needed_change = -min(this->min_point.x - other.max_point.x, 0.0f);
    float right_needed_change =  max(this->max_point.x - other.min_point.x, 0.0f);
    float top_needed_change = -min(this->min_point.y - other.max_point.y, 0.0f);
    float bottom_needed_change = max(this->max_point.y - other.min_point.y, 0.0f);
    float forward_needed_change = -min(this->min_point.z - other.max_point.z, 0.0f);
    float backward_needed_change = max(this->max_point.z - other.min_point.z, 0.0f);

    bool found_move = false;
    float best_move_distance = 0.0;
    vec3 possible_move(0.0);

    // Find the shortest distance to shove the other AABB out of this AABB
    auto lam = [&found_move, &best_move_distance, &possible_move, dimensions](float move_distance, vec3 move_dir) -> void {
        // If right_needed_change > 0, and its less than the width,
        // Then it's a valid move to shove the other AABB to get it out of us
        // abs(dot(dimensions, move_dir)) tells us how wide this AABB is in that direction
        if (0 < move_distance && move_distance < abs(dot(dimensions, move_dir))) {
            // If we haven't found a move, this is our new move
            // If we have, but this move is a shorter distance,
            // then it's the better move
            if (!found_move || move_distance < best_move_distance) {
                found_move = true;
                // Keep track of the best move distance
                best_move_distance = move_distance;
                // The move is how far we have to move, times the direction we need to move in
                possible_move = move_distance*move_dir;
            }
        }
    };

    // We check by trying to move the other AABB in each direction,
    // by the amount it needs to move in that direction to exit the current AABB
    lam(left_needed_change, vec3(-1.0, 0.0, 0.0));
    lam(right_needed_change, vec3(1.0, 0.0, 0.0));
    lam(top_needed_change, vec3(0.0, -1.0, 0.0));
    lam(bottom_needed_change, vec3(0.0, 1.0, 0.0));
    lam(forward_needed_change, vec3(0.0, 0.0, -1.0));
    lam(backward_needed_change, vec3(0.0, 0.0, 1.0));

    // If we found a move, we return how we need to move to push the other AABB out of this AABB
    if (found_move) {
        return { possible_move };
    } else {
        return nullopt;
    }
}
