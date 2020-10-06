import VoxelEngine;

class Models {
    int stone_model;
    int dirt_model;
    init();
    void initialize();
}
implement Models {
    init() {
    }
    void initialize() {
        this.stone_model = voxel_engine.register_model("assets/models/stone_block.json");
        this.dirt_model = voxel_engine.register_model("assets/models/dirt_block.json");
    }
}

Models models = new Models();

export {models};
