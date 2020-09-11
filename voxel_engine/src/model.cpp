#include "model.hpp"
#include "universe.hpp"

Component::Component(map<string, mat4> perspectives, int mesh_id, vec3 pivot, map<string,int> textures, bool opacities[6]) {
    this->perspectives = perspectives;
    this->textures = textures;
    this->mesh_id = mesh_id;
    this->pivot = pivot;
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

const bool* Component::get_opacities() {
    return this->opacities;
}

const mat4& Component::get_perspective(const string& perspective) {
    return this->perspectives[perspective];
}

vec3 Component::get_pivot() {
    return this->pivot;
}

/*
SpecifiedComponent::SpecifiedComponent(int component_id, float x_rotation, float y_rotation, bool uvlock) {
    this->component_id = component_id;
    this->x_rotation = x_rotation;
    this->y_rotation = y_rotation;
    this->uvlock = uvlock;
    this->weight = 1;
}

tuple<byte*, byte*, int> SpecifiedComponent::get_mesh_data(bool visible_neighbors[6]) {
    return get_universe()->get_component(this->component_id)->get_mesh_data(visible_neighbors);
}*/

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

static GLuint entity_shader;
static bool loaded_entity_shader = false;

static void mesh_render(const mat4& PV, const mat4& M, tuple<byte*, byte*, int> mesh_data) {
    if (!loaded_entity_shader) {
        entity_shader = load_shaders("assets/shaders/entity.vert", "assets/shaders/entity.frag");
        loaded_entity_shader = true;
    }
    glUseProgram(entity_shader);
    
    GLuint shader_texture_id = glGetUniformLocation(entity_shader, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, get_universe()->get_atlasser()->get_atlas_texture());

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(entity_shader, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(entity_shader, "M");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    glUniformMatrix4fv(M_matrix_shader_pointer, 1, GL_FALSE, &M[0][0]);


    int num_triangles = get<2>(mesh_data);

    GLuint vertex_buffer;
    GLuint uv_buffer;
    vertex_buffer = create_array_buffer((GLfloat*)get<0>(mesh_data), num_triangles*3*3*sizeof(GLfloat));
    uv_buffer = create_array_buffer((GLfloat*)get<1>(mesh_data), num_triangles*3*2*sizeof(GLfloat));

    if (PV == mat4(1.0f)) {
        for(int i = 0; i < 3; i++) {
            dbg("FOUND!");
            vec3 vert = ((vec3*)get<0>(mesh_data))[i];
            dbg("Vert: (%f, %f, %f)", vert.x, vert.y, vert.z);
            vert = vec3(M * vec4(vert, 1.0));
            dbg("Vert: (%f, %f, %f)", vert.x, vert.y, vert.z);
        }
    }

    // 1st attribute buffer : vertices
    bind_array(0, vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, uv_buffer, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, num_triangles*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &uv_buffer);
}

void Model::render(const mat4& P, const mat4& V, const mat4& M, string perspective, const map<string,string>& properties) {
    UNUSED(M);
    vector<ComponentPossibilities> v = generate_model_instance(properties);
    for(const vector<int>& component_possibilities : v) {
        int component_id = component_possibilities[0];
        Component* component = get_universe()->get_component(component_id);

        bool visible_neighbors[6] = {true, true, true, true, true, true};
        tuple<byte*, byte*, int> mesh_data = component->get_mesh_data(visible_neighbors);
        mat4 perspective_matrix = component->get_perspective(perspective);

        mesh_render(P*V, M*perspective_matrix*translate(mat4(1.0f), -component->get_pivot()), mesh_data);
    }
}
