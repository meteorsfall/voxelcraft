#include "universe.hpp"
#include "gl_utils.hpp"

Universe main_universe;

Universe* get_universe() {
    return &main_universe;
}

BlockType* Universe::get_block_type(int block_type_id) {
    return &this->block_types.at(block_type_id-1);
}

TextureAtlasser* Universe::get_atlasser() {
    return &this->atlasser;
}

int Universe::register_atlas_texture(const char* texture_path, ivec3 color_key) {
    return atlasser.add_bmp(BMP(texture_path, color_key));
}

int Universe::register_texture(const char* texture_path, ivec3 color_key) {
    textures.push_back(Texture(BMP(texture_path, color_key)));
    return textures.size();
}

int Universe::register_mesh(const char* mesh_path) {
    meshes.push_back(Mesh(mesh_path));
    return meshes.size();
}

extern int grass_block_component;

int Universe::register_model(const char* model_path) {
    UNUSED(model_path);
    Model my_model = Model(vector<string>{}, [](const map<string, string>& props) -> vector<ComponentPossibilities> {
        UNUSED(props);
        vector<vector<int>> models;

        models.push_back(vector<int>{
            grass_block_component
        });

        return models;
    });
    models.push_back(std::move(my_model));
    return models.size();
}

extern int mesh_id;
extern int grass_side_texture;
extern int dirt_texture;
extern int grass_top_texture;

int Universe::register_component(const char* component_path) {
    UNUSED(component_path);

    bool opacities[6] = {true, true, true, true, true, true};
    Component c(map<string,mat4>{
        {"block",mat4()}
    }, mesh_id, map<string,int>{
        {"negative_x", grass_side_texture},
        {"positive_x", grass_side_texture},
        {"negative_y", dirt_texture},
        {"positive_y", grass_top_texture},
        {"negative_z", grass_side_texture},
        {"positive_z", grass_side_texture}
    }, opacities);

    components.push_back(std::move(c));
    return components.size();
}

int Universe::register_event() {
    events.push_back(Event());
    return events.size();
}

Event* Universe::get_event(int event_id) {
    return &events.at(event_id-1);
}

Mesh* Universe::get_mesh(int mesh_id) {
    return &meshes.at(mesh_id-1);
}

int Universe::register_cubemap_texture(const char* texture_path, ivec3 color_key) {
    BMP cubemap = BMP(texture_path, color_key);
    cubemap_textures.push_back(CubeMapTexture(cubemap));
    return cubemap_textures.size();
}

CubeMapTexture* Universe::get_cubemap_texture(int texture_id) {
    return &cubemap_textures.at(texture_id-1);
}

Texture* Universe::get_texture(int texture_id) {
    return &textures.at(texture_id - 1);
}

Component* Universe::get_component(int component_id) {
    return &components.at(component_id - 1);
}

Model* Universe::get_model(int model_id) {
    return &models.at(model_id - 1);
}

int Universe::register_blocktype(int nx, int px, int ny, int py, int nz, int pz, bool is_opaque) {
    // Block_ID starts at 1 and increments afterwards
    block_types.push_back(BlockType(nx, px, ny, py, nz, pz, is_opaque));
    block_types.back().block_id = block_types.size();
    return block_types.back().block_id;
}
