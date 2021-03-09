#include "mesh.hpp"
#include "gl_utils.hpp"
#include <sstream>

Mesh::Mesh() {
}

Mesh::Mesh(const char* filepath) {
    ifstream obj_file;
    obj_file.open(filepath);

    auto read_face_vertex = [](std::istringstream& ss, int& a, int& b, int&c) -> bool {
        char read_slash;
        // Try reading vertex coordinate
        if (!(ss >> a)) return false;
        // Check for UV
        if (ss.peek() == '/') {
            // Read UV
            ss >> read_slash >> b;
            // Check for normal
            if (ss.peek() == '/') {
                // Read normal
                ss >> read_slash >> c;
            } else {
                c = -1;
            }
        } else {
            // Have UVs assume vertex coordinates
            b = a;
            c = -1;
        }
        return true;
    };

    map<string,int> cull_mapping = {
        {"-x",0},
        {"+x",1},
        {"-y",2},
        {"+y",3},
        {"-z",4},
        {"+z",5},
        {"none",-1}
    };

    map<string,int> texture_map;
    int current_texture = -1;
    int current_cull = cull_mapping["none"];

    vector<vec3> vertex_coords;
    vector<vec2> uv_coords;

    string line;
    while (!obj_file.eof())
    {
        getline(obj_file, line);
        float x,y,z;

        if(line[0] == 'u') {
            std::istringstream in(line);
            std::string s;
            in >> s;
            if (s.compare("usetexture") != 0) {
                dbg("Bad line! %s", line.c_str());
                continue;
            }

            // Read texture string
            std::string texture;
            in >> texture;

            if (!texture_map.count(texture)) {
                texture_map[texture] = textures.size();
                this->textures.push_back(texture);
            }

            // Set current texture
            current_texture = texture_map[texture];
        }

        if (line[0] == 'c') {
            std::istringstream in(line);
            string s;
            in >> s;
            if (s.compare("cull") != 0) {
                dbg("Bad line! %s", line.c_str());
                continue;
            }

            // Get cull string
            string cull_str;
            in >> cull_str;
            if (!cull_mapping.count(cull_str)) {
                dbg("Bad line! %s", line.c_str());
                continue;
            }

            // Set current cull status
            current_cull = cull_mapping[cull_str];
        }

        if(line[0] == 'v' && line[1] == ' ') {
            std::istringstream in(line.substr(2));
            in >> x >> y >> z;
            vertex_coords.push_back({x,y,z});
        }

        if(line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            std::istringstream in(line.substr(3));
            in >> x >> y;
            uv_coords.push_back({x,y});
        }

        int a_v, a_vt, a_vn, b_v, b_vt, b_vn, c_v, c_vt, c_vn, d_v, d_vt, d_vn;
        if(line[0] == 'f' && line[1] == ' ') {
            std::istringstream in(line.substr(2));

            read_face_vertex(in, a_v, a_vt, a_vn);
            read_face_vertex(in, b_v, b_vt, b_vn);
            read_face_vertex(in, c_v, c_vt, c_vn);

            if (current_texture < 0) {
                dbg("ERROR: usetexture has not been used yet!");
            }

            // Add triangle
            triangle_data.push_back(triangle{
                {vertex_coords.at(a_v-1), vertex_coords.at(b_v-1), vertex_coords.at(c_v-1)},
                {uv_coords.at(a_vt-1), uv_coords.at(b_vt-1), uv_coords.at(c_vt-1)},
                current_cull,
                current_texture,
            });

            // If there's a fourth coordinate for a QUAD, add another triangle
            if (read_face_vertex(in, d_v, d_vt, d_vn)) {
                triangle_data.push_back({
                    {vertex_coords.at(a_v-1), vertex_coords.at(c_v-1), vertex_coords.at(d_v-1)},
                    {uv_coords.at(a_vt-1), uv_coords.at(c_vt-1), uv_coords.at(d_vt-1)},
                    current_cull,
                    current_texture,
                });
            }
        }
    }

    // 3 vertices per triangle
    vertex_buffer.resize(triangle_data.size() * 3);
    uv_buffer.resize(triangle_data.size() * 3);    
    
    obj_file.close();
}

// Offset, Scale
tuple<byte*, byte*, int> Mesh::get_mesh_data(bool visible_neighbors[6], const vector<pair<vec2, vec2>>& texture_transformations) {
    int vertex_buffer_index = 0;
    int uv_buffer_index = 0;
    int num_triangles = triangle_data.size();
    int num_used_triangles = 0;
    for(int i = 0; i < num_triangles; i++) {
        triangle& tri = triangle_data[i];
        // If this triangle should be culled, then we cull it
        if (tri.cull_condition != -1 && !visible_neighbors[tri.cull_condition]) {
            continue;
        }

        int texture = tri.texture;
        const pair<vec2, vec2>& transformation = texture_transformations.at(texture);

        for(int j = 0; j < 3; j++) {
            // Copy vertex position over
            vertex_buffer[vertex_buffer_index+j] = tri.vertices[j];
            // Copy uv over, making sure to transform the UV to the correct location
            uv_buffer[uv_buffer_index+j] = transformation.first + tri.uvs[j] * transformation.second;
        }
        vertex_buffer_index += 3;
        uv_buffer_index += 3;
        num_used_triangles++;
    }
    return {(byte*)&vertex_buffer[0], (byte*)&uv_buffer[0], num_used_triangles};
}

const vector<string>& Mesh::get_texture_names() {
    return this->textures;
}

/*
void Mesh::render(const mat4& PV, mat4& M) {
    glUseProgram(opengl_shader);
    
    GLuint shader_texture_id = glGetUniformLocation(opengl_shader, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(1, shader_texture_id, texture.opengl_texture_id.opengl_id.value());

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(opengl_shader, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(opengl_shader, "M");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    glUniformMatrix4fv(M_matrix_shader_pointer, 1, GL_FALSE, &M[0][0]);

    // 1st attribute buffer : vertices
    bind_array(0, get_vertex_buffer(), 3);

    // 2nd attribute buffer : colors
    bind_array(1, get_uv_buffer(), 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, this->num_triangles*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

GLuint Mesh::get_vertex_buffer() {
    if (!opengl_vertex_buffer.opengl_id) {
        opengl_vertex_buffer.opengl_id = create_array_buffer(&vertices[0][0], vertices.size()*3*sizeof(GLfloat));
    }
    return opengl_vertex_buffer.opengl_id.value();
}

GLuint Mesh::get_uv_buffer() {
    if (!opengl_uv_buffer.opengl_id) {
        opengl_uv_buffer.opengl_id = create_array_buffer(&uvs[0][0], uvs.size()*3*sizeof(GLfloat));
    }
    return opengl_uv_buffer.opengl_id.value();
}
*/
