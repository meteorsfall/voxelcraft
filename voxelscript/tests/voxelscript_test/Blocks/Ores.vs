import Entity;
import Fuels;

// Iron Ore
class IronOre {
}
implement IronOre {
}

implement Entity on IronOre {
    int entity_id() {
        return 121948428;
    }
}

implement Block on IronOre {
}

// Coal Ore
class CoalOre {
}
implement CoalOre {
}

implement Entity on CoalOre {
    int entity_id() {
        return 312910253;
    }
}

implement Block on CoalOre {
    Entity drop() {
        return new Coal();
    }
}

export {IronOre, CoalOre};
