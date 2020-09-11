#include "drop.hpp"
#include "player.hpp"

extern int on_break_event;
extern int on_pickup_event;
extern int on_tick_event;
extern int on_render_event;
extern Player* g_player;
extern World* g_world;

///// The DropMod handles drops

DropMod::DropMod() {
    // Create the Event on_pickup_event
    on_pickup_event = get_universe()->register_event();

    // Subscribe to the on tick event
    get_universe()->get_event(on_tick_event)->subscribe([this](void* data) {
        float delta_time = *(float*)data;

        // Iterate the DropMod class every tick
        for(uint i = 0; i < drops.size(); i++) {
            Drop& drop = drops[i];

            vec3 diff = g_player->body.position + vec3(0.0, 0.75, 0.0) - drop.item.body.position;
            if (diff.y > 0.0) {
                diff.y *= 1.5f; // Stricter standards when comparing vertically
            }
            if ( length(diff) < 2.0f) {
                if (length(diff) > 0.1f) {
                    drop.item.body.push(normalize(diff)*20.0f, delta_time);
                }
            }
            // Check for intersection with plyer
            if( length(diff) < 1.0f && glfwGetTime() - drop.spawn_time > 0.5f) {
                // Trigger the on-pickup-event
                get_universe()->get_event(on_pickup_event)->trigger_event(NULL);
                // Erase the ith drop from the list
                drops.erase(drops.begin() + i);
                // Continue looping through drops
                i--; // But check the new ith element again since we just deleted the ith element.
                continue;
            }
            
            // Spin the drop
            mat4 model = mat4(1.0);
            model = rotate(model, (float)radians(glfwGetTime() * 360 / 2), vec3(0.0, 1.0, 0.0));
            drop.item.model_matrix = model;
            
            // Apply gravity and then iterate
            drop.item.body.push(vec3(0.0, -9.8, 0.0), delta_time);
            drop.item.body.iterate(delta_time);

            // Have the drop bob up and down
            drop.item.aabb.min_point.y = -0.25/2 - 0.15 + 0.1*sin(radians(glfwGetTime() * 360 / 3));

            // Collide the drop against the world blocks
            g_world->collide(drop.item.get_aabb(), drop.item.get_on_collide());
        }
    });
    
    // Subscribe to the on render event
    get_universe()->get_event(on_render_event)->subscribe([this](void* data) {
        const mat4& PV = *(mat4*)data;
        // Iterate the DropMod class every tick
        for(Drop& drop : drops) {
            drop.item.render(PV);
        }
    });

    // Subscribe to the on break event
    get_universe()->get_event(on_break_event)->subscribe([this](void* data) {
        ivec3 location = *(ivec3*)data;
        Drop drop(vec3(location) + vec3(0.5, 0.5, 0.5));
        drop.item.model_id = g_world->read_block(location)->block_model;
        int hash = hash_ivec3(location, 99);
        drop.item.body.velocity = vec3(hash % 8, 10, (hash / 8) % 8) / 5.0f;
        drop.spawn_time = glfwGetTime();
        drops.push_back(drop);
    });
}

Drop::Drop(vec3 position){
    item.body.position = position;
    item.body.velocity = vec3(0);
    float radius = 0.25;
    item.aabb = AABB(vec3(-radius), vec3(radius));
}
