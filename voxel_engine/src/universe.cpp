#include "universe.hpp"
#include "gl_utils.hpp"
#include <rapidjson/document.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
using namespace rapidjson;

Universe main_universe;

Universe* get_universe() {
    return &main_universe;
}

TextureAtlasser* Universe::get_atlasser() {
    return &this->atlasser;
}

int Universe::register_atlas_texture(const char* texture_path, ivec3 color_key) {
    string filename = std::filesystem::path(texture_path).filename().generic_string();
    if (atlas_texture_names.count(filename)) {
        return atlas_texture_names[filename];
    }
    int texture_id = atlasser.add_bmp(BMP(texture_path, color_key));
    atlas_texture_names[filename] = texture_id;
    return texture_id;
}

int Universe::register_texture(const char* texture_path, ivec3 color_key) {
    string filename = std::filesystem::path(texture_path).filename().generic_string();
    if (texture_names.count(filename)) {
        return texture_names[filename];
    }
    textures.push_back(Texture(BMP(texture_path, color_key)));
    int texture_id = textures.size();
    texture_names[filename] = texture_id;
    return texture_id;
}

int Universe::register_mesh(const char* mesh_path) {
    string filename = std::filesystem::path(mesh_path).filename().generic_string();
    if (mesh_names.count(filename)) {
        return mesh_names[filename];
    }
    meshes.push_back(Mesh(mesh_path));
    int mesh_id = meshes.size();
    mesh_names[filename] = mesh_id;
    return mesh_id;
}

int Universe::register_component(const char* component_path) {
    string filename = std::filesystem::path(component_path).stem().generic_string();
    if (component_names.count(filename)) {
        return component_names[filename];
    }


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

    delete[] buf;

    // Grab perspectives
    Value& persps = json["perspectives"];
    map<string,mat4> perspectives;

    for(auto& m : persps.GetObject()) {
        // Get rotation/translation/scale
        vec3 rotation = vec3(0);
        vec3 scale_factor = vec3(1);
        vec3 translation = vec3(0);
        for(auto& transform : m.value.GetObject()) {
            string k = transform.name.GetString();
            if (k.compare("rotation") == 0) {
                for(int i = 0; i < 3; i++) {
                    rotation[i] = radians(transform.value[i].GetFloat());
                }
            }
            if (k.compare("translation") == 0) {
                for(int i = 0; i < 3; i++) {
                    translation[i] = transform.value[i].GetFloat();
                }
            }
            if (k.compare("scale") == 0) {
                for(int i = 0; i < 3; i++) {
                    scale_factor[i] = transform.value[i].GetFloat();
                }
            }
        }

        // Apply matrix transformations

        mat4 model = mat4(1.0f);
        // Translation
        model = translate(model, translation);
        // Rotate
        mat4 euler_rotation = eulerAngleX(rotation.x) * eulerAngleY(rotation.y) * eulerAngleZ(rotation.z);
        model = model * euler_rotation;
        // Scale
        model = scale(model, scale_factor);
        
        const char* key = m.name.GetString();
        perspectives[key] = model;

        //dbg("Rotate: (%f, %f, %f)", rotation.x, rotation.y, rotation.z);
        //dbg("Translate: (%f, %f, %f)", translation.x, translation.y, translation.z);
        //dbg("Scale: (%f, %f, %f)", scale_factor.x, scale_factor.y, scale_factor.z);
    }

    // Grab mesh
    Value& s = json["mesh"];
    const char* mesh_name = json["mesh"].GetString();
    int mesh_id = mesh_names.at(mesh_name);

    // Grab pivot
    Value& pivot_pt = json["pivot"];
    vec3 pivot;
    for(int i = 0; i < 3; i++) {
        pivot[i] = pivot_pt[i].GetFloat();
    }

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
        perspectives,
        mesh_id,
        pivot,
        textures,
        opacities
    );

    components.push_back(std::move(c));
    int component_id = components.size();
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
                if (!this->component_names.count(comp["component"].GetString())) {
                    dbg("Could not find component name: %s", comp["component"].GetString());
                    exit(-1);
                }
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

int Universe::register_font(const char* filepath) {
    fonts.push_back(Font(filepath));
    return fonts.size();
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

Font* Universe::get_font(int font_id) {
    return &fonts.at(font_id - 1);
}
