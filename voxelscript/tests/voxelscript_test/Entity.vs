trait Entity {
    // A unique identifier to identify various entities
    int entity_id();
    
    bool exists() {
        return this.entity_id() != 0;
    }

    // True if the same entity as the given one
    bool same_entity_as(Entity e) {
        return this.entity_id() == e.entity_id();
    }
}

class NullEntityObject {}
implement NullEntityObject {}

implement Entity on NullEntityObject {
    int entity_id() {
        return 0;
    }
}

NullEntityObject NullEntity = new NullEntityObject();

trait Block {
    Entity drop() {
        return NullEntity;
    }
}

// Exports
export {Entity, NullEntity, Block};
