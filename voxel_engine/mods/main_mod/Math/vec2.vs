import Math;

class vec2 {
    float x;
    float y;
    init(float x, float y);
}

implement vec2 {
    init(float x, float y) {
        this.x = x;
        this.y = y;
    }
    vec2 clone() {
        return new vec2(this.x, this.y);
    }
    float length() {
        return math.sqrt(this.x*this.x + this.y*this.y);
    }
    vec2 normalize() {
        float len = this.length();
        return new vec2(this.x/len, this.y/len);
    }
    vec2 times(float scalar) {
        return new vec2(
            this.x * scalar,
            this.y * scalar
        );
    }
    vec2 add(vec2 other) {
        return new vec2(
            this.x + other.x,
            this.y + other.y
        );
    }
}

export {vec2};
