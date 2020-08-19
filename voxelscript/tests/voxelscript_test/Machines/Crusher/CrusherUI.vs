import EntityContainer;
import Crusher;
//import VC::UI;

class CrusherUI {
	Crusher crusher;
	EntityContainer hand;
	
	init(Crusher c);
	void right_click(EntityContainer c);
	void left_click(EntityContainer c);
	void on_right_click(int x, int y);
	void on_left_click(int x, int y);
}

implement CrusherUI {
	init(Crusher c) {
		this.crusher = c;
		this.hand = new EntityContainer();
	}

	void right_click(EntityContainer c) {
		EntityContainer hand = this.hand;

		// If the hand exists, 
		if (hand.exists()) {
			// Only allow standard right-click-add into entity container when it's empty or of the same type
			if (!c.exists() || c.entity.same_entity_as(hand.entity)) {
				// If it's valid to right click and place into the entity container, then do it
				if (c.can_give(hand.entity, 1)) {
					c.give(hand.take(1).entity, 1);
				}
			// Otherwise, try to swap the whole hand with the whole entity container
			} else if (c.can_swap(hand)) {
				c.swap(hand);
			}
		// If the hand doesn't exist, we try to grab half the stack
		} else if (c.exists()) {
			int amount_to_take = (c.stack_size()+1)/2;
			if (hand.can_give(c.entity, amount_to_take) && c.can_take(amount_to_take)) {
				hand.give(c.take(amount_to_take).entity, amount_to_take);
			}
		} else {
			// If you right click with nothing, and nothing's there, then nothing happens
		}
	}

	void left_click(EntityContainer c) {
		if (c.can_swap(this.hand)) {
			c.swap(this.hand);
		}
	}

	void on_right_click(int x, int y) {
		if (this.within_fuel_slot(x, y)) {
			this.right_click(this.crusher.fuel);
		}
		if (this.within_crushing_slot(x, y)) {
			this.right_click(this.crusher.crushing);
		}
		if (this.within_crushed_slot(x, y)) {
			this.right_click(this.crusher.crushed);
		}
	}

	void on_left_click(int x, int y) {
		if (this.within_fuel_slot(x, y)) {
			this.left_click(this.crusher.fuel);
		}
		if (this.within_crushing_slot(x, y)) {
			this.left_click(this.crusher.crushing);
		}
		if (this.within_crushed_slot(x, y)) {
			this.left_click(this.crusher.crushed);
		}
	}

	bool within_fuel_slot(int x, int y) {
		return false;
	}
	bool within_crushing_slot(int x, int y) {
		return false;
	}
	bool within_crushed_slot(int x, int y) {
		return false;
	}
}

export {CrusherUI};
