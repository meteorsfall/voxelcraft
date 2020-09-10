#include "universe.hpp"
#include "gl_utils.hpp"
#include <rapidjson/document.h>
using namespace rapidjson;

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
    int texture_id = atlasser.add_bmp(BMP(texture_path, color_key));
    string filename = std::filesystem::path(texture_path).filename().generic_string();
    atlas_texture_names[filename] = texture_id;
    return texture_id;
}

int Universe::register_texture(const char* texture_path, ivec3 color_key) {
    textures.push_back(Texture(BMP(texture_path, color_key)));
    int texture_id = textures.size();
    string filename = std::filesystem::path(texture_path).filename().generic_string();
    texture_names[filename] = texture_id;
    return texture_id;
}

int Universe::register_mesh(const char* mesh_path) {
    meshes.push_back(Mesh(mesh_path));
    int mesh_id = meshes.size();
    string filename = std::filesystem::path(mesh_path).filename().generic_string();
    mesh_names[filename] = mesh_id;
    return mesh_id;
}

int Universe::register_component(const char* component_path) {
    // Read JSON
    ifstream in(component_path, std::ios::binary | std::ios::ate);

    int size = in.tellg();
    in.seekg(0, std::ios::beg);

    char* buf = new char[size+1];
    in.read(buf, size);
    buf[size] = '\0';
    in.close();

    Document json;
    json.Parse(buf);

    free(buf);

    // Grab mesh
    Value& s = json["mesh"];
    const char* mesh_name = json["mesh"].GetString();
    int mesh_id = mesh_names.at(mesh_name);

    // Grab textures
    map<string,int> textures;
    const Value& json_textures = json["textures"];
    for(auto& m : json_textures.GetObject()) {
        textures[m.name.GetString()] = atlas_texture_names.at(m.value.GetString());
    }

    // Grab opacities
    bool opacities[6] = {true};
    const Value& json_opacities = json["opacities"].GetObject();
    const char* opacity_name[] = {"-x", "+x", "-y", "+y", "-z", "+z"};
    for(int i = 0; i < 6; i++) {
        opacities[i] = json_opacities[opacity_name[i]].GetBool();
    }

    Component c(
        map<string,mat4>{
            {"block",mat4()}
        },
        mesh_id,
        textures,
        opacities
    );

    components.push_back(std::move(c));
    int component_id = components.size();
    string filename = std::filesystem::path(component_path).stem().generic_string();
    component_names[filename] = component_id;
    return component_id;
}

int Universe::register_model(const char* model_path) {
    ifstream in(model_path, std::ios::binary | std::ios::ate);

    int size = in.tellg();
    in.seekg(0, std::ios::beg);

    vector<char> buf(size+1);
    in.read(&buf[0], size);
    buf[size] = '\0';
    in.close();

    // Document

    Model my_model = Model(vector<string>{}, [this, buf](const map<string, string>& props) -> vector<ComponentPossibilities> {
        UNUSED(props);
        vector<vector<int>> models;

        Document json;
        json.Parse(&buf[0]);

        const Value& as_arr = json;
        for(uint i = 0; i < as_arr.Size(); i++) {
            const Value& obj = json[i].GetObject();
            const Value& apply = obj["apply"];

            // Get array of component possibilities from this "apply"
            vector<int> components;
            for(uint j = 0; j < apply.Size(); j++) {
                const Value& comp = apply[j];
                components.push_back(this->component_names.at(comp["component"].GetString()));
            }

            // Add ComponentPossibilities to the model
            models.push_back(components);
        }

        return models;
    });
    models.push_back(std::move(my_model));
    return models.size();
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
