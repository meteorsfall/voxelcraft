#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "utils.hpp"
#include "mesh.hpp"
#include "texture.hpp"

/// A Component consists of a mesh, perspectives to view the mesh from, textures to apply to the mesh, 
class Component {
public:
    Component(map<string, mat4> perspectives, int mesh_id, vec3 pivot, map<string,int> textures, bool opacities[6]);
    tuple<byte*, byte*, int> get_mesh_data(bool visible_neighbors[6]);
    const bool* get_opacities();
    const mat4& get_perspective(const string& perspective);
    vec3 get_pivot();
private:
    vector<pair<vec2, vec2>> texture_transformations;
    map<string, mat4> perspectives;
    int mesh_id;
    vec3 pivot;
    map<string,int> textures;
    bool opacities[6];
};

/// Specifies a Component after certain parameters have been applied
class SpecifiedComponent {
    SpecifiedComponent(int component_id, float x_rotation, float y_rotation, bool uvlock);
    tuple<byte*, byte*, int> get_mesh_data(bool visible_neighbors[6]);
private:
    int component_id;
    float x_rotation;
    float y_rotation;
    bool uvlock;
    int weight;
};

/// Represents a set of SpecificComponent ids, one which one will be selected at random
typedef vector<int> ComponentPossibilities;

/// From a set of properties, a ModelGenerator will generate a model
using SpecifiedModelGenerator = function<
  vector<ComponentPossibilities>(const map<string,string>&)
>;

class Model {
public:
    Model(vector<string> valid_properties, SpecifiedModelGenerator model_generator);
    const vector<ComponentPossibilities>& generate_model_instance(const map<string,string>& properties);
    void render(const mat4& P, const mat4& V, const mat4& M, string perspective, const map<string,string>& properties);
private:
    map<string,vector<ComponentPossibilities>> cache;
    vector<string> valid_properties;
    SpecifiedModelGenerator model_generator;
};

#endif
