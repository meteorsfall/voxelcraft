#include "chunk.hpp"
#include "gl_utils.hpp"
#include <cstring>
#include "texture_atlasser.hpp"

static bool loaded_chunk_shader = false;
static GLuint chunk_shader_id;

Chunk::Chunk(ivec3 location) {
    this->location = location;
    if (!loaded_chunk_shader) {
        chunk_shader_id = LoadShaders("assets/shaders/chunk.vert", "assets/shaders/chunk.frag");
        loaded_chunk_shader = true;
    }
    opengl_vertex_buffer = create_array_buffer(NULL, 1);
    opengl_uv_buffer = create_array_buffer(NULL, 1);
    opengl_break_amount_buffer = create_array_buffer(NULL, 1);
}

void Chunk::set_block(int x, int y, int z, BlockType* b) {
    x -= location.x * CHUNK_SIZE;
    y -= location.y * CHUNK_SIZE;
    z -= location.z * CHUNK_SIZE;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        printf("Bad coordinates! %d %d %d\n", x, y, z);
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

void Chunk::render(mat4 &PV, TextureAtlasser& texture_atlas, fn_get_block master_get_block) {
    ivec3 bottom_left = location*CHUNK_SIZE;

    // Check aabb of chunk against the view frustum
    AABB aabb(bottom_left, vec3(bottom_left) + vec3(CHUNK_SIZE));
    if (!aabb.test_frustum(PV)) {
        return;
    }

    if (this->chunk_rendering_cached) {
        cached_render(PV);
        return;
    }

    // 12 triangles in a cube, 3 vertices in a triangle, 3 position floats in a vertex
    GLfloat* chunk_vertex_buffer = new GLfloat[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3*3];
    int chunk_vertex_buffer_len = 0;
    // 12 triangles in a cube, 3 vertices in a triangle, 2 uv floats in a vertex
    GLfloat* chunk_uv_buffer = new GLfloat[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3*2];
    int chunk_uv_buffer_len = 0;
    // 1 break amount per cube
    GLfloat* chunk_break_amount_buffer = new GLfloat[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3];
    int chunk_break_amount_buffer_len = 0;

    int num_triangles = 0;

    set<int> textures;
    vector<pair<int, ivec2>> texture_choices;

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
                
                if (visible_neighbors & ~(1 << 7)) {
                    // Generate boolean array from visible neighbor bitset
                    // This will represent which faces to include and which to cull
                    vec3 fpos(position);
                    bool buf[6];
                    for(int m = 0; m < 6; m++) buf[m] = (visible_neighbors >> m) & 1;

                    // Get vertex buffer and uv buffer data with any undesired faced culled
                    auto [vertex_buffer_data, vertex_buffer_len] = get_specific_cube_vertex_coordinates(buf);
                    auto [uv_buffer_data, uv_buffer_len] = get_specific_cube_uv_coordinates(buf);
                    
                    // Translate vertices and save vertex-specific metadata
                    for(int m = 0; m < vertex_buffer_len/(int)sizeof(GLfloat); m += 3) {
                        vertex_buffer_data[m+0] += fpos.x;
                        vertex_buffer_data[m+1] += fpos.y;
                        vertex_buffer_data[m+2] += fpos.z;
                        chunk_break_amount_buffer[chunk_break_amount_buffer_len/sizeof(GLfloat)] = blocks[i][j][k].break_amount;
                        chunk_break_amount_buffer_len += sizeof(GLfloat);
                    }

                    for(int m = 0; m < uv_buffer_len/(int)sizeof(GLfloat); m += 2) {
                        /*
                        if (uv_buffer_data[m+0] < 0.1) {
                            uv_buffer_data[m+0] += 0.005;
                        } else {
                            uv_buffer_data[m+0] -= 0.005;
                        }
                        if (uv_buffer_data[m+1] < 0.1) {
                            uv_buffer_data[m+1] += 0.005;
                        } else {
                            uv_buffer_data[m+1] -= 0.005;
                        }
                        */
                    }

                    int uv_loc = chunk_uv_buffer_len/sizeof(GLfloat);
                    
                    // Memcpy vertex and uv buffers
                    memcpy(&chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)], vertex_buffer_data, vertex_buffer_len);
                    memcpy(&chunk_uv_buffer[chunk_uv_buffer_len/sizeof(GLfloat)], uv_buffer_data, uv_buffer_len);

                    // Keep track vertex and uv buffer sizes
                    chunk_vertex_buffer_len += vertex_buffer_len;
                    chunk_uv_buffer_len += uv_buffer_len;
                    num_triangles += vertex_buffer_len / sizeof(GLfloat) / 3 / 3;

                    // Save texture
                    int texture = blocks[i][j][k].block_type->texture;
                    textures.insert(texture);
                    texture_choices.push_back({texture, ivec2(uv_loc, uv_buffer_len/sizeof(GLfloat))});
                }
            }
        }
    }

    //printf("New Size: %ld\n", texture_choices.size());
    int atlas_width = texture_atlas.get_atlas()->width;
    int atlas_height = texture_atlas.get_atlas()->height;
    for(auto& texture_choice : texture_choices) {
        int texture = texture_choice.first;
        ivec2 uv_bounds = texture_choice.second;
        //printf("Bounds: (%d, %d)\n", uv_bounds[0], uv_bounds[1]);
        for(int i = uv_bounds[0]; i < uv_bounds[0] + uv_bounds[1]; i += 2) {
            //printf("I: %d\n", i);
            ivec2 offset = texture_atlas.get_top_left(texture);
            //printf("Translating (%f, %f) / (%d, %d) based on (%d, %d) in (%d, %d)\n", chunk_uv_buffer[i+0], chunk_uv_buffer[i+1], texture_atlas.get_bmp(texture)->width, texture_atlas.get_bmp(texture)->height, offset.x, offset.y, atlas_width, atlas_height);
            chunk_uv_buffer[i+0] = offset.x / (float)atlas_width + chunk_uv_buffer[i+0] * texture_atlas.get_bmp(texture)->width / atlas_width;
            chunk_uv_buffer[i+1] = offset.y / (float)atlas_height + chunk_uv_buffer[i+1] * texture_atlas.get_bmp(texture)->height / atlas_height;
            //printf("To: (%f, %f)\n", chunk_uv_buffer[i+0], chunk_uv_buffer[i+1]);
        }
    }

    reuse_array_buffer(opengl_vertex_buffer, chunk_vertex_buffer, chunk_vertex_buffer_len);
    reuse_array_buffer(opengl_uv_buffer, chunk_uv_buffer, chunk_uv_buffer_len);
    reuse_array_buffer(opengl_break_amount_buffer, chunk_break_amount_buffer, chunk_break_amount_buffer_len);

    this->chunk_rendering_cached = true;
    this->num_triangles_cache = num_triangles;
    
    this->opengl_texture_atlas = texture_atlas.get_atlas_texture();
    cached_render(PV);

    delete[] chunk_vertex_buffer;
    delete[] chunk_uv_buffer;
    delete[] chunk_break_amount_buffer;
}

void Chunk::cached_render(mat4& PV) {
    if (num_triangles_cache == 0) {
        // No need to render if there are no triangles
        return;
    }

    glUseProgram(chunk_shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(chunk_shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, opengl_texture_atlas);

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint PV_matrix_shader_pointer = glGetUniformLocation(chunk_shader_id, "PV");
    GLuint M_matrix_shader_pointer = glGetUniformLocation(chunk_shader_id, "M");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(PV_matrix_shader_pointer, 1, GL_FALSE, &PV[0][0]);
    vec3 position(0.0f);
    glUniform3fv(M_matrix_shader_pointer, 1, &position[0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    bind_array(0, opengl_vertex_buffer, 3);

    // 2nd attribute buffer : colors
    bind_array(1, opengl_uv_buffer, 2);

    // 3rd attribute buffer : break amount
    bind_array(2, opengl_break_amount_buffer, 1);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, num_triangles_cache*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
