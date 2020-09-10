#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include "utils.hpp"
#include "mesh.hpp"
#include "texture.hpp"

/**
 *\addtogroup VoxelEngine
 * @{
 */

/// A Component consists of a mesh, perspectives to view the mesh from, textures to apply to the mesh, 
class Component {
public:
    /// Creates a new Component
    /**
     * @param perspectives A given component can be viewed from several perspectives in-game.
     * The common ones are "left-hand", "right-hand", "hat", "drop", "block"
     * @param mesh_id The mesh that this component will use for rendering
     * @param pivot The center of the mesh that model matrix transformations will act on
     * @param textures A mapping from mesh textures to texture_ids. For example,
     * if cube.obj has texture names "negative_y", "positive_y", etc, then textures["positive_y"]
     * might be a texture ID referring to grass_top.bmp
     * @param opacities Says whether or not the mesh is opaque on a given side. A fully opaque block
     * will consist of 6 true's, while a fully transparent block will consist of 6 false's. The brewing
     * stand is only opaque on the bottom face, so in that case opacities[2] will be true, while the rest will be false.
     * These will be interpreted in the same order as Mesh::get_mesh_data interprets visible_neighbors
     */
    Component(map<string, mat4> perspectives, int mesh_id, vec3 pivot, map<string,int> textures, bool opacities[6]);
    /// Retrives the mesh and uv data for this component
    tuple<byte*, byte*, int> get_mesh_data(bool visible_neighbors[6]);
    /// Retrives the opacities information
    const bool* get_opacities();
    /// Retrives the matrix that represents a given perspective
    const mat4& get_perspective(const string& perspective);
    /// Retrives the pivot point
    vec3 get_pivot();
private:
    vector<pair<vec2, vec2>> texture_transformations;
    map<string, mat4> perspectives;
    int mesh_id;
    vec3 pivot;
    map<string,int> textures;
    bool opacities[6];
};

/*
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
*/

/// Represents a single component. It is given as a vector because one will be chosen probabilistically
typedef vector<int> ComponentPossibilities;

/// From a set of properties, a ModelGenerator will generate a model
using SpecifiedModelGenerator = function<
  vector<ComponentPossibilities>(const map<string,string>&)
>;

/// The Model class represents an in-game Model that can be drawn as an in-game block, or a GUI element, or a drop item, or a hat, etc.
class Model {
public:
    /// Create a new model
    /**
     * @param valid_properties Creating a Model Instance from a Model requires specifying properties, such as
     * which directions have a neighboring fence-post, if this Model represents a fence-post that must
     * dynamically attach to neighbors. In this case, the valid properties would be "east", "west", "north", and "south",
     * whose value may be any string. The values of the properties themselves will be provided when Model::generate_model_instance is called.
     * @param model_generator A model generator will take a set of properties and instance the model based on those properties
     */
    Model(vector<string> valid_properties, SpecifiedModelGenerator model_generator);
    /// Generate a model instance based on the given properties. This will be an array of components to render. (Will call the internal model_generator)
    const vector<ComponentPossibilities>& generate_model_instance(const map<string,string>& properties);
    /// Render the model given a selection of parameters
    /**
     * @param P Projection Matrix (Creates 3D perspective)
     * @param V View Matrix (Location of the Camera)
     * @param M Model Matrix (Location/Rotation/Scale of the model, taken from about each component's pivot)
     * @param perspective In which perspective the model will be viewed ("block", "drop", "lefthand", etc)
     * @param properties The set of properties that define how the model will be rendered ("east":"connected", "west":"none", etc)
     */
    void render(const mat4& P, const mat4& V, const mat4& M, string perspective, const map<string,string>& properties);
private:
    map<string,vector<ComponentPossibilities>> cache;
    vector<string> valid_properties;
    SpecifiedModelGenerator model_generator;
};

/**@}*/

#endif
