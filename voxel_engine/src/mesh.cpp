#include "mesh.hpp"
#include "gl_utils.hpp"
#include <sstream>

Mesh::Mesh() {
}

Mesh::Mesh(const char* filepath) {
    ifstream obj_file;
    obj_file.open(filepath);

    // We have to set this->vertices and this->uv_coordinates correctly
    string line;
    vector<vec3> vertex_coords;
    vector<vec2> uv_coords;
    while (!obj_file.eof())
    {
        getline(obj_file, line);
        float x,y,z;

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
        char c;
        if(line[0] == 'f' && line[1] == ' ') {
            std::istringstream in(line.substr(2));

            in >> a_v >> c >> a_vt >> c >> a_vn >>
                  b_v >> c >> b_vt >> c >> b_vn >>
                  c_v >> c >> c_vt >> c >> c_vn;

            bool quad = false;
            if (in >> d_v >> c >> d_vt >> c >> d_vn) {
                quad = true;
            }

            int vertex_array[6] = {a_v, b_v, c_v, a_v, c_v, d_v};
            int uv_array[6] = {a_vt, b_vt, c_vt, a_vt, c_vt, d_vt};
            for(int i = 0; i < (quad ? 6 : 3); i++) {
                this->vertices.push_back(vertex_coords.at(vertex_array[i] - 1));
                this->uvs.push_back(uv_coords.at(uv_array[i] - 1));
            }
            this->num_triangles++;
            if (quad) {
                // Quad consists of two triangles
                this->num_triangles++;
            }
        }
    }
    
    obj_file.close();

    // Texture and Shader
    this->texture = Texture(BMP("assets/images/dirt.bmp"));
    this->opengl_shader = load_shaders("assets/shaders/mesh.vert", "assets/shaders/mesh.frag");
}

Mesh Mesh::cube_mesh() {
    Mesh m = Mesh();
    
    auto [vertex_buffer_data, vertex_buffer_len] = get_cube_vertex_coordinates();
    auto [uv_buffer_data, uv_buffer_len] = get_cube_uv_coordinates();

    m.opengl_vertex_buffer.opengl_id = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    m.opengl_uv_buffer.opengl_id = create_array_buffer(uv_buffer_data, uv_buffer_len);
    m.texture = Texture(BMP("assets/images/dirt.bmp"));
    m.opengl_shader = load_shaders("assets/shaders/mesh.vert", "assets/shaders/mesh.frag");

    // 12 triangles in a cube
    m.num_triangles = 12;

    return m;
}

void Mesh::render(const mat4& PV, mat4& M) {
    glUseProgram(opengl_shader);
    
    GLuint shader_texture_id = glGetUniformLocation(opengl_shader, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, texture.opengl_texture_id.opengl_id.value());

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
