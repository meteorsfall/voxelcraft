import Entity;

trait Combustible {
    int get_energy();
}

class Coal {
}
implement Coal {
}

implement Entity on Coal {
    int entity_id() {
        return 77282931;
    }
}

implement Combustible on Coal {
    int get_energy() {
        return 10;
    }
}

export {Coal, Combustible};
