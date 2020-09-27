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
            return false;
        }
    }
}

implement Hash on Vec3 {
    int hash() {
        return this.x * 123 + 321 * this.y + 9001 * this.z;
    }
}

class Option<T: any> {
    T val;
}

implement Option<T> {}

class HashMap<Key: Hash + Eq, Value: any> {
    init();
    Value get(Key k);
    void set(Key k, Value v);
}

implement HashMap<Key, Value> {
    Option<Value>[] vals;
    int size;
    int capacity;

    init() {
        this.size = 0;
        this.capacity = 25;
        this.vals = [];
        for(int i = 0; i < 25; i++) {
            this.vals.push(new Option<Value>());
        }
    }

    Value get(Key k) {
        int hash = k.hash();
        return this.vals[hash % this.capacity].val;
    }

    void set(Key k, Value v) {
        int hash = k.hash();
        this.vals[hash % this.capacity].val = v;
    }
}

class Array<T: any> {
    T[] arr;
    int size;
    int capacity;
}

implement Array<T> {

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

class Tester {
    init();
}
implement Tester {
    init() {
        Coal f = new Coal();
        IronOre io = new IronOre();
        Crusher c = new Crusher();

        Holder<Coal> h = new Holder<Coal>(f);

        HashMap<Vec3, int> m = new HashMap<Vec3, int>();
        m.set(new Vec3(2, 3, 4), 5);
        int v = m.get(new Vec3(2, 3, 4));
        print("Integer: ", v);

        Holder<int> hh = new Holder<int>(5);
        print("Held Int: ", hh.get_holder());

        c.fuel.give(h.get_holder(), 1);
        c.crushing.give(io, 1);
        int i = 0;
        while(i < 250) {
            print("<progress=", c.current_progress, ",energy_in_crusher=", c.energy_in_crusher, ">");
            c.iterate();
            i++;
        }
    }
}

Tester t = new Tester();

export {};
