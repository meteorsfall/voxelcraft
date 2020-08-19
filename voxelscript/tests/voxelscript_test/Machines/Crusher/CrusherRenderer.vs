//import VC::Renderer;
//import VC::Block;
//import VC::Texture;
import Crusher;

/*
VC::Texture crusher_texture_off;
VC::Texture crusher_texture_on;

VC::Block crusher_on_block;
VC::Block crusher_off_block;

init() {
    crusher_texture_off = VC::TextureUtils.import("./crusher_off.tx");
    crusher_texture_on = VC::TextureUtils.import("./crusher_on.tx");

    crusher_off_block = VC::BlockUtils.create_block();
    crusher_on_block = VC::BlockUtils.create_block();

    crusher_off_block.set_texture(crusher_texture_off);
    crusher_on_block.set_texture(crusher_texture_on);
    crusher_off_block.set_opaque(true);
    crusher_on_block.set_opaque(true);
    crusher_off_block.set_collidable(true);
    crusher_on_block.set_collidable(true);
}

implement VC__RenderableVoxel on Crusher {
    void VC_iterate() {
        int prev_had_burnt_energy = this.had_burnt_energy;
        iterate();
        if (this.had_burnt_energy != prev_had_burnt_energy) {
            VC::Renderer.register_update(renderable_voxel_id);
        }
    }

    void VC_render(int x, int y, int z) {
        int block_id;
        if (had_burnt_energy) {
            block_id = crusher_on_block_id;
        } else {
            block_id = crusher_off_block_id;
        }

        VC::Renderer.set_block(x, y, z, block_id);
    }

    void VC_Destroy() {
        VC::Renderer.destroy(renderable_voxel_id);
    }
}

*/
export {};
