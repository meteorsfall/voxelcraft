import Crusher;
import Fuels;
import Ores;
import Entity;

trait Eq {
    bool is_equal(Eq e);
}

trait Hash {
    int hash();
}

class Vec4 {
    int x;
    int y;
    int z;
    init(int x, int y, int z);
    void func();
}

implement Vec4 {
    init(int x, int y, int m) {
        this.x = x;
        this.y = y;
        this.z = m;
    }
    void func() {
    }
}

class Vec3 {
    int x;
    int y;
    int z;
    init(int x, int y, int z);
}

implement Vec3 {
    init(int x, int y, int z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
}

implement Eq on Vec3 {
    bool is_equal(Eq e) {
        if (e is Vec3) {
            Vec3 v = <Vec3>e;
            return this.x == v.x && this.y == v.y && this.z == v.z;
        } else {
            return this.truey();
        }
    }
    bool truey() {
        return true;
    }
}

implement Hash on Vec3 {
    int hash() {
        return this.x * 123 + 321 * this.y + 9001 * this.z;
    }
}

class Option<T: any> {
    init();
    T get();
    bool is_set();
    void set(T val);
    void unset();
}

implement Option<T> {
    T val;
    bool _is_set;

    init() {
        this._is_set = false;
    }
    T get() {
        if (!this._is_set) {
            throw "Tried to unwrap unset option";
        }
        return this.val;
    }
    bool is_set() {
        return this._is_set;
    }
    void set(T val) {
        this.val = val;
        this._is_set = true;
    }
    void unset() {
        this._is_set = false;
    }
}

class HashMap<Key: Hash + Eq, Value: any> {
    init();
    Value get(Key k);
    void set(Key k, Value v);
}

implement HashMap<Key, Value> {
    // Set of values
    Value[][] vals;
    Key[][] keys;
    // Refers to vals.size()
    int capacity;
    // Refers to # of elements in HashMap
    int size;

    init() {
        this.size = 0;
        this.capacity = 25;
        this.vals = [];
        this.keys = [];
        this.vals.resize(25);
        this.keys.resize(25);
        for(int i = 0; i < 25; i++) {
            this.vals[i] = [];
            this.keys[i] = [];
        }
    }

    Value get(Key k) {
        int hash = k.hash();
        Key[] keys = this.keys[hash % this.capacity];
        int found = 0-1;
        for(int i = 0; i < keys.size(); i++) {
            if (keys[i].is_equal(k)) {
                found = i;
            }
        }
        if (found >= 0) {
            return this.vals[hash % this.capacity][found];
        } else {
            throw "Key not found!";
        }
    }

    bool is_set(Key k) {
        int hash = k.hash();
        Key[] keys = this.keys[hash % this.capacity];
        for(int i = 0; i < keys.size(); i++) {
            if (keys[i].is_equal(k)) {
                return true;
            }
        }
        return false;
    }

    void set(Key k, Value v) {
        int hash = k.hash();
        Key[] keys = this.keys[hash % this.capacity];
        int found = 0-1;
        for(int i = 0; i < keys.size(); i++) {
            if (keys[i].is_equal(k)) {
                found = i;
            }
        }
        if (found != -1) {
            // Set the value at that location
            this.vals[hash % this.capacity][found] = v;
        } else {
            // Add to the linked list
            this.keys[hash % this.capacity].push(k);
            this.vals[hash % this.capacity].push(v);
        }
    }

    void unset(Key k) {
        int hash = k.hash();
        Key[] keys = this.keys[hash % this.capacity];
        int found = -1;
        for(int i = 0; i < keys.size(); i++) {
            if (keys[i].is_equal(k)) {
                // Remove the discovered key/value pair
                keys.remove(i);
                this.vals[hash % this.capacity].remove(i);
                return;
            }
        }
    }
}


class Pair<U: any, V: any> {
    U first;
    V second;
    init(U first, V second);
}
implement Pair<U, V> {
    init(U first, V second) {
        this.first = first;
        this.second = second;
    }
}

class Holder<T: any> {
    T test;
    init(T in);
    T get_holder();
}
implement Holder<T> {
    init(T in) {
        this.test = in;
    }

    T get_holder() {
        return this.test;
    }
}

// *****************
// Begin Tester
// *****************

class Tester {
    init();
}
implement Tester {
    Vec3[] null_test;
    init() {
        Coal f = new Coal();
        IronOre io = new IronOre();
        Crusher c = new Crusher();

        Holder<Coal> h = new Holder<Coal>(f);

        Pair<int, string> my_pair = new Pair<int, string>(5, "hi");

        print("my_pair<", my_pair.first, ",", my_pair.second, ">");

        c.fuel.give(h.get_holder(), 1);
        c.crushing.give(io, 1);
        for(int i = 0; i < 250; i++) {
            print("<progress=", c.current_progress, ",energy_in_crusher=", c.energy_in_crusher, ">");
            c.iterate();
        }

        // Custom tests

        HashMap<Vec3, float> m = new HashMap<Vec3, float>();
        m.set(new Vec3(2, 3, 4), 5.0);
        float v = m.get(new Vec3(2, 3, 4));
        print("Integer: ", v);

        Holder<int> hh = new Holder<int>(5);
        print("Held Int: ", hh.get_holder());

        string a = "test1";
        string b = "test2";
        string cc = a.concat(b);
        bool mmm = "38912".is_integer();
        print("M: ", mmm);

        float start_time = time();

        Vec3[] arr = [];
        for(int i = 0; i < 10000; i++) {
            arr = [];
            for(int j = 0; j < 1000; j++) {
                arr.push(new Vec3(1.0, 2.0, 3.0));
            }
            arr.resize(0);
        }
        print("Hi!");

        float[] test_arr = [4.0, 2.0, 1.0];

        print("Float < Int", test_arr[0] < 2);

        print("Time: ", time() - start_time);

        print("test arr: ", test_arr);

        float randi_sum = 0.0;
        float randf_sum = 0.0;
        for(int i = 0; i < 100; i++) {
            randi_sum += randi();
            randf_sum += randf();
        }
        print("Average of 100 randi()'s: ", randi_sum / 100);
        print("Average of 100 randf()'s: ", randf_sum / 100);

        // Should throw null-pointer exception
        // this.null_test.push(new Vec3(1.0, 2.0, 3.0));

        //string ret = input();
        //print("Copy: ", ret);

        // Check for char parsing
        char char1 = 'B';
        char char2 = '\x41';
        if (char1 != char2) print("Char comparison passed!");

        // Check for string indexing
        string strstr = "abcd";
        print("strstr[2] = ", strstr[2]);

        // Check for while loops with no body
        int i = 0;
        while(i++ < 50);

        print("INT: ", 12345);
        print("INT: ", -15);

        // Test exponential notation
        float asdkf = 5.5e+5 + 5.55e3 - 1.23e-3;
        print("expFloat: ", asdkf);

    }
}

Tester t = new Tester();

import Env;

class Main {
    init();
    void _init();
    void _iterate();
    void _render();
    void _iterate_ui();
    void _render_ui();
}
implement Main {
    init() {}
    void _init() {
        env.VoxelEngine__register_font("Testing!");
    }
    void _iterate() {
    }
    void _render() {
    }
    void _iterate_ui() {
    }
    void _render_ui() {
    }
}

export {Main};
