import vec3;

class mat4 {
    float[] arr;
    init(float val);
}

implement mat4 {
    init(float val) {
        this.arr = [];
        this.arr.resize(16);
        this.arr[0] = val;
        this.arr[5] = val;
        this.arr[10] = val;
        this.arr[15] = val;
    }
    mat4 make_perspective(float fov, float aspect_ratio, float near, float far) {
        mat4 ret = new mat4(0.0);
        float frustum_depth = far - near;
        float one_over_depth = 1.0 / frustum_depth;

        ret.arr[4*1+1] = 1.0 / math.tan(0.5 * fov);
        ret.arr[4*0+0] = ret.arr[4*1+1] / aspect_ratio;
        ret.arr[4*2+2] = -(near+far) * one_over_depth;
        ret.arr[4*3+2] = -(2.0*near*far) * one_over_depth;
        ret.arr[4*2+3] = -1.0;

        return ret;
    }
    mat4 look_at(vec3 eye, vec3 target, vec3 up) {
        vec3 f = target.sub(eye).normalize();
        vec3 s = f.cross(up).normalize();
        vec3 u = s.cross(f);

        mat4 ret = this.clone();
        ret.arr[4*0 + 0] = s.x;
        ret.arr[4*1 + 0] = s.y;
        ret.arr[4*2 + 0] = s.z;
        ret.arr[4*0 + 1] = u.x;
        ret.arr[4*1 + 1] = u.y;
        ret.arr[4*2 + 1] = u.z;
        ret.arr[4*0 + 2] = -f.x;
        ret.arr[4*1 + 2] = -f.y;
        ret.arr[4*2 + 2] = -f.z;
        ret.arr[4*3 + 0] = -s.dot(eye);
        ret.arr[4*3 + 1] = -u.dot(eye);
        ret.arr[4*3 + 2] = f.dot(eye);

        return ret;
    }
    mat4 clone() {
        mat4 ret = new mat4(1.0);
        for(int i = 0; i < 16; i++) {
            ret.arr[i] = this.arr[i];
        }
        return ret;
    }
}

export {mat4};
