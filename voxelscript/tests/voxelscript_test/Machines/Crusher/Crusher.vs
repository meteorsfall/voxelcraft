import Entity;
import EntityContainer;
import Fuels;

const TICKS_PER_UNIT_ENERGY = 20;

// An Entity is Crushable if it implements this trait
trait Crushable {
    EntityContainer crush();
    int ticks_needed_to_crush();
}

class Crusher {
    // The coal/charcoal/oil/etc that powers the crusher
    EntityContainer fuel;
    // The Entity that we're crushing
    EntityContainer crushing;
    // The Entitys that have been crushed
    EntityContainer crushed;

    // How much cumulative effort has been put into crushing
    int current_progress;
    // How much the fuel has left to burn
    int energy_in_crusher;

    init();

    void iterate();
}

// The Crucher object
implement Crusher {
    private Entity to_crush_entity = NullEntity;

    // Initialize a crusher
    init() {
        // The entity to be used as fuel
        this.fuel = new EntityContainer();
        
        this.fuel.register_can_change((_, _, Entity new_entity, _) => {
            // You can either take fuel out entirely, or put in Combustible fuel
            return !new_entity.exists() || new_entity is Combustible;
        });

        // The entity to be crushed
        this.crushing = new EntityContainer();
        this.crushing.register_can_change((_, _, Entity new_entity, _) => {
            // You can take stuff out of the crushing container, or you can put Crushable objects in
            return !new_entity.exists() || new_entity is Crushable;
        });
        this.crushing.register_on_change((Entity old_entity, _, Entity new_entity, _) => {
            // Keep track of the to_crush_entity, and then reset the progress
            if (new_entity.same_entity_as(NullEntity)) {
                this.to_crush_entity = NullEntity;
            } else {
                this.to_crush_entity = ((Crushable)new_entity).crush().entity;
            }
            if (!old_entity.same_entity_as(new_entity)) {
                this.current_progress = 0;
            }
        });

        // The entity resulted from crush
        this.crushed = new EntityContainer();
        this.crushed.register_can_change((Entity old_entity, int old_qty, Entity new_entity, int new_qty) => {
            // If it's the same entity, you must remove qty
            if (old_entity.same_entity_as(new_entity)) {
                return new_qty < old_qty;
            // Otherwise, you must be taking the whole thing out
            } else {
                return !new_entity.exists();
            }
        });

        this.current_progress = 0;
        this.energy_in_crusher = 0;
    }

    // Check if we can crush
    private bool can_crush() {
        bool exists_something_to_crush = this.crushing.exists();
        bool somewhere_to_put_crushed_Entity = this.crushed.can_give(this.to_crush_entity, 1);
        return exists_something_to_crush && somewhere_to_put_crushed_Entity;
    }

    private bool had_burnt_energy;

    void iterate() {
        this.crushed.unlock();

        // Continue to lose energy
        bool burnt_energy = false;
        if (this.energy_in_crusher > 0) {
            this.energy_in_crusher--;
            burnt_energy = true;
        }

        // If we can crush, then we shall progress onwards
        if (this.can_crush()) {
            // Check if we need fuel and can start burning the next fuel
            if (this.fuel.exists() && !burnt_energy) {
                int energy_of_fuel = ((Combustible)this.fuel.entity).get_energy();
                this.fuel.take(1);
                this.energy_in_crusher = energy_of_fuel * TICKS_PER_UNIT_ENERGY;

                this.energy_in_crusher--;
                burnt_energy = true;
            }

            // If we've burnt energy, then progress in crushing has occured
            if (burnt_energy) {
                this.current_progress++;
            }

            // If we've progressed enough to crush the Entity, then we crush it
            if (this.current_progress == ((Crushable)this.crushing.entity).ticks_needed_to_crush()) {
                this.current_progress = 0;
                Crushable to_crush = (Crushable)this.crushing.take(1);
                Entity newly_crushed = to_crush.crush().entity;
                if (!this.crushed.can_give(newly_crushed, 1)) {
                    throw "Could not give recipe result with newly crushed object!";
                }
                this.crushed.give(newly_crushed, 1); // Consumes newly_crushed!
            }
        }

        this.had_burnt_energy = burnt_energy;

        this.crushed.lock();
    }
}

// Properties of Crusher
implement Entity on Crusher {
    // Crusher is itself an Entity
    int entity_id() {
        return 4298398134;
    }
}

implement Block on Crusher {
    Entity drop() {
        return this;
    }
}

implement Stackable on Crusher {
    // Crusher can be stacked in stacks of 64
    int max_stack_size() {
        return 64;
    }
}

import Ores;

// Implement Crushable on all the main ores
implement Crushable on IronOre {
    EntityContainer crush() {
        Function ToDust = (Entity a) => {return a;};
        Entity ret = ToDust(this);
        EntityContainer stack = new EntityContainer();
        stack.change(ret, 2 5);
        return stack;
    }

    int ticks_needed_to_crush() {
        return 100;
    }
}

export {Crusher, Crushable};
