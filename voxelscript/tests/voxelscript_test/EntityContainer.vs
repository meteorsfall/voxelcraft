import Entity;
import Ores;
import Fuels;
 
trait Stackable  {
    int max_stack_size();
}

typedef bool fn_can_change(Entity old_entity, int old_qty, Entity new_entity, int new_qty);
typedef void fn_on_change(Entity old_entity, int old_qty, Entity new_entity, int new_qty);

class EntityContainer {
	Entity entity;
	int qty;

    init();

    // Locks or unlocks the EntityContainer behind can_change
    void lock();
    void unlock();
    
    // Checks if changing to the given e,qty is valid
    bool can_change(Entity e, int qty);
    // Changes to the given e,qty
    void change(Entity e, int qty);
	
    // Take(qty) means removing qty items from an EntityContainer and then returning qty of them
    bool can_take(int qty);
    EntityContainer take(int qty);

    // Give will add qty to the existing stack. Requires entity == e, unless entity is NullEntity
    bool can_give(Entity e, int qty);
    void give(Entity e, int qty);

    bool can_swap(EntityContainer ec); // Short hand for this.can_give(ec.entity, ec.qty) && ec.can_give(this.entity, this.qty)
    void swap(EntityContainer ec); // Swap two entity containers, but maintain filters
    
    // Gets called when either the underlying data changes
    // These filters affect GUI interactions, but during the iterate() function .take/.give can be called with no restrictions
    void register_can_change(fn_can_change can_change);
    void register_on_change(fn_on_change on_change);

    int stack_size();
    bool is_full();
    bool exists(); // True if qty >= 1

    int max_stack_size();
}

implement EntityContainer {
    fn_can_change[] registered_can_change = [];
    fn_on_change[] registered_on_change = [];

    bool is_locked = true;

    init() {
        this.entity = NullEntity;
        this.qty = 0;
    }

    void lock() {
        this.is_locked = true;
    }

    void unlock() {
        this.is_locked = false;
    }
    
    bool can_change(Entity e, int qty) {
        if (!this.is_locked) {
            return true;
        }
        if (qty < 0) {
            return false;
        }
        if (qty == 0) {
            e = NullEntity;
        }
        if (qty > 0 && e.same_entity_as(NullEntity)) {
            return false;
        }

        if (qty > 1 && e is not Stackable) {
            return false;
        }
        if (qty > 1 && e is Stackable && qty > ((Stackable)e).max_stack_size()) {
            return false;
        }

        for (reg_can_change : this.registered_can_change) {
            if (!reg_can_change(this.entity, this.qty, e, qty)) {
                return false;
            }
        }

        return true;
    }

    void change(Entity e, int qty) {
        if (qty == 0) {
            e = NullEntity;
        }
        if (!this.can_change(e, qty)) {
            throw "Cannot change to e, qty!";
        }

        for (reg_on_change : this.registered_on_change) {
            reg_on_change(this.entity, this.qty, e, qty);
        }

        this.entity = e;
        this.qty = qty;
    }

    bool can_take(int qty) {
        if (qty < 0) {
            return false;
        }
        return this.can_change(this.entity, this.qty - qty);
    }

    EntityContainer take(int qty) {
        if (this.can_take(qty)) {
            EntityContainer ret = new EntityContainer();
            this.change(this.entity, this.qty - qty);
            return ret;
        } else {
            throw "Cannot take qty!";
        }
    }

    bool can_give(Entity e, int qty) {
        if (this.entity.exists()) {
            // Must be the same
            if (this.entity.same_entity_as(e)) {
                return this.can_change(e, this.qty + qty);
            } else {
            // Otherwise we can't mix
                return false;
            }
        } else {
            // If there's no entity, we can check (e, qty)
            return this.can_change(e, qty);
        }
    }

    void give(Entity e, int qty) {
        if (!this.can_give(e, qty)) {
            throw "Cannot give!";
        }
        this.change(e, this.qty + qty);
    }

    bool can_swap(EntityContainer ec) {
        return this.can_change(ec.entity, ec.qty) && ec.can_change(this.entity, this.qty);
    }

    void swap(EntityContainer ec) {
        if (!this.can_swap(ec)) {
            throw "Cannot swap!";
        }
        Entity other_entity = ec.entity;
        int other_qty = ec.qty;
        ec.change(this.entity, this.qty);
        this.change(other_entity, other_qty);
    }
    
    void register_can_change(fn_can_change can_change) {
        this.registered_can_change.push(can_change);
    }
    
    void register_on_change(fn_on_change on_change) {
        this.registered_on_change.push(on_change);
    }
    
    int stack_size() {
        return this.qty;
    }
    bool is_full() {
        return this.qty == this.max_stack_size();
    }
    bool exists() {
        return this.qty >= 1;
    }

    int max_stack_size() {
        if (this.entity is not Stackable) {
            return 1;
        } else {
            return ((Stackable)this.entity).max_stack_size();
        }
    }
}

// Implement Stackable on all the main ores
implement Stackable on IronOre {
    int max_stack_size() {
        return 32;
    }
}

// Implement Stackable on all the main ores
implement Stackable on CoalOre {
    int max_stack_size() {
        return 32;
    }
}

// Implement Stackable on all the main ores
implement Stackable on Coal {
    int max_stack_size() {
        return 64;
    }
}

export {Stackable, EntityContainer};
