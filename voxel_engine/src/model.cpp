#include "model.hpp"
#include "universe.hpp"

Component::Component(map<string, mat4> perspectives, int mesh_id, map<string,int> textures, bool opacities[6]) {
    this->perspectives = perspectives;
    this->textures = textures;
    this->mesh_id = mesh_id;
    for(int i = 0; i < 6; i++) {
        this->opacities[i] = opacities[i];
    }

    const vector<string>& texture_names = get_universe()->get_mesh(mesh_id)->get_texture_names();

    // Find texture transformations for each texture name
    texture_transformations.resize(texture_names.size());
    for(uint i = 0; i < texture_names.size(); i++) {
        // For the ith texture name, find the texture_id associated with it
        int atlas_texture_id = textures.at(texture_names[i]);
        ivec2 itop_left = get_universe()->get_atlasser()->get_top_left(atlas_texture_id);
        vec2 top_left = vec2(itop_left);

        const BMP* bmp = get_universe()->get_atlasser()->get_bmp(atlas_texture_id);
        const BMP* atlas_bmp = get_universe()->get_atlasser()->get_atlas();
        vec2 scale;
        scale.x = bmp->get_width() / (float)atlas_bmp->get_width();
        scale.y = bmp->get_height() / (float)atlas_bmp->get_height();

        top_left.y += bmp->get_height();
        top_left.y = atlas_bmp->get_height() - top_left.y;

        top_left.x /= atlas_bmp->get_width();
        top_left.y /= atlas_bmp->get_height();

        texture_transformations[i].first = vec2(top_left.x, top_left.y);
        texture_transformations[i].second = scale;
    }
}

tuple<byte*, byte*, int> Component::get_mesh_data(bool visible_neighbors[6]) {
    return get_universe()->get_mesh(this->mesh_id)->get_mesh_data(visible_neighbors, this->texture_transformations);
}

SpecifiedComponent::SpecifiedComponent(int component_id, float x_rotation, float y_rotation, bool uvlock) {
    this->component_id = component_id;
    this->x_rotation = x_rotation;
    this->y_rotation = y_rotation;
    this->uvlock = uvlock;
    this->weight = 1;
}

tuple<byte*, byte*, int> SpecifiedComponent::get_mesh_data(bool visible_neighbors[6]) {
    return get_universe()->get_component(this->component_id)->get_mesh_data(visible_neighbors);
}

Model::Model(vector<string> valid_properties, SpecifiedModelGenerator model_generator) {
    this->valid_properties = valid_properties;
    this->model_generator = model_generator;   
}

const vector<ComponentPossibilities>& Model::generate_model_instance(const map<string,string>& properties) {
    string key = "";
    for(auto& s : properties) {
        key += s.first;
        key += "=";
        key += s.second;
        key += ",";
    }
    if (!cache.count(key)) {
        cache[key] = this->model_generator(properties);
    }
    return cache[key];
}
