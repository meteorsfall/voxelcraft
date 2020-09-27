import Crusher;
import Fuels;
import Ores;
import Entity;

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
