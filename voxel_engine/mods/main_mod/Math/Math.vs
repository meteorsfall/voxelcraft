
class BasicMath {
    float pi;
    init();
    float sin(float theta);
    float cos(float theta);
    float deg2rad(float deg);
    float rad2deg(float rad);
    float sqrt(float a);
    float floor(float x);
}

implement BasicMath {
    init() {
        this.pi = 3.1415926;
    }
    float sin(float x) {
        float sign = 1.0;
        if (x < 0.0) {
            sign = -1.0;
            x = -x;
        }
        if (x > 2.0*this.pi) {
            x -= <float>(<int>(x/(2.0*this.pi)))*2.0*this.pi;
        }
        float res = 0.0;
        float term = x;
        float k = 1.0;
        while (res+term!=res) {
            res += term;
            k += 2.0;
            term *= -x*x/k/(k-1.0);
        }
        
        return sign*res;
    }
    float cos(float theta) {
        return this.sin(this.pi/2.0 - theta);
    }
    float tan(float theta) {
        return this.sin(theta) / this.cos(theta);
    }
    float deg2rad(float deg) {
        return deg * this.pi / 180.0;
    }
    float rad2deg(float rad) {
        return rad * 180.0 / this.pi;
    }
    float sqrt(float x) {
        // let initial guess to be 1
        float z = 1.0;
        for(int i = 1; i <= 10; i++) {
            z -= (z*z - x) / (2.0*z); // MAGIC LINE!!
        }
        return z;
    }
    float floor(float x) {
        if (x <= 0.0 && x != <float><int>x) {
            return <float>(<int>x - 1);
        } else {
            return <float><int>x;
        }
    }
    float ceil(float x) {
        if (x >= 0.0 && x != <float><int>x) {
            return <float>(<int>x + 1);
        } else {
            return <float><int>x;
        }
    }
    float min(float x, float y) {
        return x < y ? x : y;
    }
    float max(float x, float y) {
        return x > y ? x : y;
    }
    float abs(float x) {
        return x < 0.0 ? -x : x;
    }
}

BasicMath math = new BasicMath();

export {math};
