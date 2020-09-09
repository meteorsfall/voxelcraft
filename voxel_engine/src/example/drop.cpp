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
    
    Mesh m = Mesh::cube_mesh();
    int mesh_id = get_universe()->register_mesh(m);
    int texture_id = get_universe()->register_texture("assets/images/dirt.bmp");
    
    vec3 position = vec3(-2.0, 16.4, 0.0);
    Drop d(position);
    d.item.mesh_id = mesh_id;
    d.item.texture_id = texture_id;
    drops.push_back(d);

    // Subscribe to the on tick event
    get_universe()->get_event(on_tick_event)->subscribe([this](void* data) {
        float delta_time = *(float*)data;

        // Iterate the DropMod class every tick
        for(uint i = 0; i < drops.size(); i++) {
            Drop& drop = drops[i];

            // Check for intersection with plyer
            if( length(g_player->body.position - drop.item.body.position) < 1.0f ) {
                // Trigger the on-pickup-event
                get_universe()->get_event(on_pickup_event)->trigger_event(NULL);
                // Erase the ith drop from the list
                drops.erase(drops.begin() + i);
                // Continue looping through drops
                i--; // But check the new ith element again since we just deleted the ith element.
                continue;
            }
            
            // Rotate and scale the drop

            mat4 model = mat4(1.0);
            // Scale
            model = scale(model, vec3(0.2f));
            // Rotate
            model = translate(model, vec3(0.5));
            model = rotate(model, (float)radians(glfwGetTime() * 360 / 3), vec3(0.0, 1.0, 0.0));
            model = translate(model, -vec3(0.5));
            drop.item.model_matrix = model;
            
            drop.item.body.push(vec3(0.0, -9.8, 0.0), delta_time);
            drop.item.body.iterate(delta_time);
            drop.item.aabb.min_point.y = -0.25 + 0.1*sin(radians(glfwGetTime() * 360 / 3));

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
    get_universe()->get_event(on_break_event)->subscribe([this, mesh_id, texture_id](void* data) {
        ivec3 location = *(ivec3*)data;
        Drop drop(vec3(location) + vec3(0.5, 0.5, 0.5));
        drop.item.mesh_id = mesh_id;
        drop.item.texture_id = texture_id;
        int hash = hash_ivec3(location, 99);
        drop.item.body.velocity = vec3(hash % 8, 10, (hash / 8) % 8) / 5.0f;
        drops.push_back(drop);
    });
}

Drop::Drop(vec3 position){
    item.body.position = position;
    item.body.velocity = vec3(0);
}
