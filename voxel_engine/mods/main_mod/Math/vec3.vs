import Math;

class vec3 {
    float x;
    float y;
    float z;
    init(float x, float y, float z);
    vec3 clone();
    float length();
    vec3 normalize();
    vec3 cross(vec3 other);
    vec3 times(float scalar);
    vec3 add(vec3 other);
    vec3 sub(vec3 other);
    float dot(vec3 other);
    vec3 floor();
}

implement vec3 {
    init(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    vec3 clone() {
        return new vec3(this.x, this.y, this.z);
    }
    float length() {
        return math.sqrt(this.x*this.x + this.y*this.y + this.z*this.z);
    }
    vec3 normalize() {
        float len = this.length();
        return new vec3(this.x/len, this.y/len, this.z/len);
    }
    vec3 cross(vec3 other) {
        return new vec3(
            this.y * other.z - this.z * other.y,
            this.z * other.x - this.x * other.z,
		    this.x * other.y - this.y * other.x
        );
    }
    vec3 times(float scalar) {
        return new vec3(
            this.x * scalar,
            this.y * scalar,
            this.z * scalar
        );
    }
    vec3 add(vec3 other) {
        return new vec3(
            this.x + other.x,
            this.y + other.y,
            this.z + other.z
        );
    }
    vec3 sub(vec3 other) {
        return new vec3(
            this.x - other.x,
            this.y - other.y,
            this.z - other.z
        );
    }
    float dot(vec3 other) {
        return this.x * other.x + this.y * other.y + this.z * other.z;
    }
    vec3 floor() {
        return new vec3(math.floor(this.x), math.floor(this.y), math.floor(this.z));
    }
}

export {vec3};
