
class BasicMath {
    float pi;
    init();
    float sin(float theta);
    float cos(float theta);
    float deg2rad(float deg);
    float rad2deg(float rad);
    float sqrt(float a);
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
        float s = this.sin(theta);
        return this.sqrt(1.0 - s*s);
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
}

BasicMath math = new BasicMath();

export {math};
