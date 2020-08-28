#include "chunk.hpp"
#include "gl_utils.hpp"
#include <cstring>

void draw_cube(mat4& PV, vec3 position, Texture* texture, int visible_neighbors);
void draw_cubes(mat4& PV, Texture* texture, int vertex_buffer, int uv_buffer, int num_triangles);

Chunk::Chunk(ivec3 location) {
    this->location = location;
    opengl_vertex_buffer = create_array_buffer(NULL, 1);
    opengl_uv_buffer = create_array_buffer(NULL, 1);
}

void Chunk::set_block(int x, int y, int z, BlockType* b) {
    x -= location.x * CHUNK_SIZE;
    y -= location.y * CHUNK_SIZE;
    z -= location.z * CHUNK_SIZE;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    blocks[x][y][z] = Block(b);
}

Block* Chunk::get_block(int x, int y, int z) {
    x -= location.x * CHUNK_SIZE;
    y -= location.y * CHUNK_SIZE;
    z -= location.z * CHUNK_SIZE;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return nullptr;
    }
    return blocks[x][y][z].block_type ? &blocks[x][y][z] : nullptr;
}

void Chunk::render(mat4 &PV, fn_get_block master_get_block) {
    ivec3 bottom_left = location*CHUNK_SIZE;

    // Check aabb of chunk against the view frustum
    AABB aabb(bottom_left, vec3(bottom_left) + vec3(CHUNK_SIZE));
    if (!aabb.test_frustum(PV)) {
        return;
    }

    if (this->chunk_rendering_cached) {
        draw_cubes(PV, texture_cache, opengl_vertex_buffer, opengl_uv_buffer, num_triangles_cache);
        return;
    }

    // 12 triangles in a cube, 3 vertices in a triangle, 3 position floats in a vertex
    GLfloat* chunk_vertex_buffer = new GLfloat[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3*3];
    int chunk_vertex_buffer_len = 0;
    // 12 triangles in a cube, 3 vertices in a triangle, 2 uv floats in a vertex
    GLfloat* chunk_uv_buffer = new GLfloat[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*2*3];
    int chunk_uv_buffer_len = 0;

    int num_triangles = 0;

    Texture* t = NULL;

    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                // If the block has no block type, just continue (It's an air block)
                if (!blocks[i][j][k].block_type) {
                    continue;
                }

                ivec3 position = bottom_left + ivec3(i, j, k);

                int visible_neighbors = 0;
                if (blocks[i][j][k].neighbor_cache) {
                    visible_neighbors = blocks[i][j][k].neighbor_cache;
                } else {
                    // Keep 1 << 7 so that visible_neighbors remains true even if all-zero data
                    visible_neighbors = 1 << 7;
                    // If master_get_block doesn't exist, then it must be air, so we must display that face
                    if (!master_get_block(position.x-1, position.y, position.z)) {
                        visible_neighbors |= 1 << 0;
                    }
                    if (!master_get_block(position.x+1, position.y, position.z)) {
                        visible_neighbors |= 1 << 1;
                    }
                    if (!master_get_block(position.x, position.y-1, position.z)) {
                        visible_neighbors |= 1 << 2;
                    }
                    if (!master_get_block(position.x, position.y+1, position.z)) {
                        visible_neighbors |= 1 << 3;
                    }
                    if (!master_get_block(position.x, position.y, position.z-1)) {
                        visible_neighbors |= 1 << 4;
                    }
                    if (!master_get_block(position.x, position.y, position.z+1)) {
                        visible_neighbors |= 1 << 5;
                    }
                    blocks[i][j][k].neighbor_cache = visible_neighbors;
                }
                
                if (visible_neighbors) {
                    // Render it at i, j, k
                    vec3 fpos(position);
                    bool buf[6];
                    for(int i = 0; i < 6; i++) buf[i] = (visible_neighbors >> i) & 1;
                    auto [vertex_buffer_data, vertex_buffer_len] = get_specific_cube_vertex_coordinates(buf);
                    auto [uv_buffer_data, uv_buffer_len] = get_specific_cube_uv_coordinates(buf);
                    
                    for(int i = 0; i < vertex_buffer_len/(int)sizeof(GLfloat); i += 3) {
                        vertex_buffer_data[i+0] += fpos.x;
                        vertex_buffer_data[i+1] += fpos.y;
                        vertex_buffer_data[i+2] += fpos.z;
                    }
                    
                    memcpy(&chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)], vertex_buffer_data, vertex_buffer_len);
                    memcpy(&chunk_uv_buffer[chunk_uv_buffer_len/sizeof(GLfloat)], uv_buffer_data, uv_buffer_len);

                    chunk_vertex_buffer_len += vertex_buffer_len;
                    chunk_uv_buffer_len += uv_buffer_len;
                    num_triangles += vertex_buffer_len / sizeof(GLfloat) / 3 / 3;

                    t = blocks[i][j][k].block_type->texture;
                }
            }
        }
    }

    reuse_array_buffer(opengl_vertex_buffer, chunk_vertex_buffer, chunk_vertex_buffer_len);
    reuse_array_buffer(opengl_uv_buffer, chunk_uv_buffer, chunk_uv_buffer_len);

    draw_cubes(PV, t, opengl_vertex_buffer, opengl_uv_buffer, num_triangles);
    this->chunk_rendering_cached = true;
    this->num_triangles_cache = num_triangles;
    this->texture_cache = t;

    delete[] chunk_vertex_buffer;
    delete[] chunk_uv_buffer;
}

void draw_cubes(mat4& PV, Texture* texture, int vertex_buffer, int uv_buffer, int num_triangles) {
    glUseProgram(texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, texture->opengl_texture_id);

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "M");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    vec3 position(0.0f);
    glUniform3fv(M_matrix_shader_pointer, 1, &position[0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    GLuint break_amount_shader_pointer = glGetUniformLocation(texture->shader_id, "break_amount");
    glUniform1f(break_amount_shader_pointer, (GLfloat)0.0);

    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    bind_array(0, vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, uv_buffer, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, num_triangles*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void draw_cube(mat4& PV, vec3 position, Texture* texture, int visible_neighbors) {
    bool buf[6];
    for(int i = 0; i < 6; i++) buf[i] = (visible_neighbors >> i) & 1;

    // 0 => Orangey-Red (-x)
    // 1 => Pink (+x)
    // 2 => Green (-y)
    // 3 => Orange (+y)
    // 4 => Dark Blue (-z)
    // 5 => Dark Red (+z)

    // Save the cube vertex buffer data to vertex_buffer
    auto [vertex_buffer_data, vertex_buffer_len] = get_specific_cube_vertex_coordinates(buf);
    auto [uv_buffer_data, uv_buffer_len] = get_specific_cube_uv_coordinates(buf);
    printf("Len: %d %d\n", vertex_buffer_len, uv_buffer_len);

    GLuint vertex_buffer = create_array_buffer(vertex_buffer_data, vertex_buffer_len);
    GLuint uv_buffer = create_array_buffer(uv_buffer_data, uv_buffer_len);


    glUseProgram(texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, texture->opengl_texture_id);

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(texture->shader_id, "M");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    glUniform3fv(M_matrix_shader_pointer, 1, &position[0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    GLuint break_amount_shader_pointer = glGetUniformLocation(texture->shader_id, "break_amount");
    glUniform1f(break_amount_shader_pointer, (GLfloat)0.0);

    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    bind_array(0, vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, uv_buffer, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
