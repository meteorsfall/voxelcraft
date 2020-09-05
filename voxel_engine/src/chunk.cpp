#include "chunk.hpp"
#include "gl_utils.hpp"
#include <cstring>
#include "texture_atlasser.hpp"

static bool loaded_chunk_shader = false;
static GLuint chunk_shader_id;

Chunk::Chunk(ivec3 location, fn_get_blocktype get_block_type) {
    // Statically load chunk shaders
    if (!loaded_chunk_shader) {
        chunk_shader_id = load_shaders("assets/shaders/chunk.vert", "assets/shaders/chunk.frag");
        loaded_chunk_shader = true;
    }

    this->location = location;
    this->get_block_type = get_block_type;
    opengl_vertex_buffer = create_array_buffer(NULL, 1);
    opengl_uv_buffer = create_array_buffer(NULL, 1);
    opengl_break_amount_buffer = create_array_buffer(NULL, 1);
}

void Chunk::set_block(int x, int y, int z, int b) {
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
    // Return nullptr if it's an air block
    return blocks[x][y][z].block_type ? &blocks[x][y][z] : nullptr;
}

void Chunk::render(mat4& P, mat4& V, TextureAtlasser& texture_atlas, fn_get_block master_get_block, bool dont_rerender) {
    ivec3 bottom_left = location*CHUNK_SIZE;
    
    // Check aabb of chunk against the view frustum
    AABB aabb(bottom_left, vec3(bottom_left) + vec3(CHUNK_SIZE));

    // If it's cached, or if we don't care about rerendering an out-of-date cached chunk
    if (this->chunk_rendering_cached || (dont_rerender && this->has_ever_cached)) {
        if (aabb.test_frustum(P*V)) {
            cached_render(P, V);
        }
        return;
    }

    // If we don't care about rerendering it, but it's never cached, then we can't draw it this frame
    if (dont_rerender) {
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

    vector<int> textures;
    // 6 sides per cube
    textures.reserve(CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*6);

    auto is_opaque = [this](Block* b) {
        // If it exists, and it's not transparent
        return b && b->block_type > 0 && !this->get_block_type(b->block_type)->is_transparent;
    };
    
	double t1 = glfwGetTime();
    UNUSED(t1);

    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                // If the block has a block type of 0, just skip it (It's an air block)
                if (!blocks[i][j][k].block_type) {
                    continue;
                }

                ivec3 position = bottom_left + ivec3(i, j, k);

// Optimize by only calling master_get_block if its outside of the current chunk
#define get_neighboring_block(xx, yy, zz, outside) ((outside) ? master_get_block(position.x+xx, position.y+yy, position.z+zz) : &blocks[i+xx][j+yy][k+zz])

                int visible_neighbors = 0;
                if (blocks[i][j][k].neighbor_cache) {
                    visible_neighbors = blocks[i][j][k].neighbor_cache;
                } else {
                    // Keep 1 << 7 so that visible_neighbors remains true even if all-zero data
                    visible_neighbors = 1 << 7;
                    // If master_get_block doesn't exist, then it must be air, so we must display that face
                    if (!is_opaque(get_neighboring_block(-1, 0, 0, i == 0))) {
                        visible_neighbors |= 1 << 0;
                    }
                    if (!is_opaque(get_neighboring_block(1, 0, 0, i == CHUNK_SIZE-1))) {
                        visible_neighbors |= 1 << 1;
                    }
                    if (!is_opaque(get_neighboring_block(0, -1, 0, j == 0))) {
                        visible_neighbors |= 1 << 2;
                    }
                    if (!is_opaque(get_neighboring_block(0, 1, 0, j == CHUNK_SIZE-1))) {
                        visible_neighbors |= 1 << 3;
                    }
                    if (!is_opaque(get_neighboring_block(0, 0, -1, k == 0))) {
                        visible_neighbors |= 1 << 4;
                    }
                    if (!is_opaque(get_neighboring_block(0, 0, 1, k == CHUNK_SIZE-1))) {
                        visible_neighbors |= 1 << 5;
                    }
                    blocks[i][j][k].neighbor_cache = visible_neighbors;
                }
                
                if (visible_neighbors & ~(1 << 7)) {
                    // Generate boolean array from visible neighbor bitset
                    // This will represent which faces to include and which to cull
                    vec3 fpos(position);

                    // Get vertex buffer and uv buffer data with any undesired faced culled
                    auto [vertex_buffer_data, vertex_buffer_len] = get_specific_cube_vertex_coordinates(visible_neighbors);
                    auto [uv_buffer_data, uv_buffer_len] = get_specific_cube_uv_coordinates(visible_neighbors);
                    
                    // Translate vertices and save vertex-specific metadata
                    for(int m = 0; m < vertex_buffer_len/(int)sizeof(GLfloat); m += 3) {
                        vertex_buffer_data[m+0] += fpos.x;
                        vertex_buffer_data[m+1] += fpos.y;
                        vertex_buffer_data[m+2] += fpos.z;
                        chunk_break_amount_buffer[chunk_break_amount_buffer_len/sizeof(GLfloat)] = blocks[i][j][k].break_amount;
                        chunk_break_amount_buffer_len += sizeof(GLfloat);
                    }
                    
                    // Memcpy vertex and uv buffers
                    memcpy(&chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)], vertex_buffer_data, vertex_buffer_len);
                    memcpy(&chunk_uv_buffer[chunk_uv_buffer_len/sizeof(GLfloat)], uv_buffer_data, uv_buffer_len);

                    // Keep track vertex and uv buffer sizes
                    chunk_vertex_buffer_len += vertex_buffer_len;
                    chunk_uv_buffer_len += uv_buffer_len;
                    num_triangles += vertex_buffer_len / sizeof(GLfloat) / 3 / 3;

                    // Save textures
                    for(int m = 0; m < 6; m++) {
                        if ((visible_neighbors >> m) & 1) {
                            int texture = get_block_type(blocks[i][j][k].block_type)->textures[m];
                            textures.push_back(texture);
                        }
                    }
                }
            }
        }
    }
    
    double time = (glfwGetTime() - t1) * 1000.0;
    if (time > 4) {
        //dbg("Chunk Construction Time: %f", time);
    }

    int atlas_width = texture_atlas.get_atlas()->width;
    int atlas_height = texture_atlas.get_atlas()->height;
    int index = 0;
    for(int texture : textures) {
        for(int i = index; i < index + 2*3*2; i += 2) {
            ivec2 offset = texture_atlas.get_top_left(texture);
            // Reorient offset to bottom-left, with (0, 0) in the bottom-left
            offset.y += texture_atlas.get_bmp(texture)->height;
            offset.y = atlas_height - offset.y;
            
            chunk_uv_buffer[i+0] = offset.x / (float)atlas_width + chunk_uv_buffer[i+0] * texture_atlas.get_bmp(texture)->width / (float)atlas_width;
            chunk_uv_buffer[i+1] = offset.y / (float)atlas_height + chunk_uv_buffer[i+1] * texture_atlas.get_bmp(texture)->height / (float)atlas_height;
        }
        index += 2*3*2;
    }

    reuse_array_buffer(opengl_vertex_buffer, chunk_vertex_buffer, chunk_vertex_buffer_len);
    reuse_array_buffer(opengl_uv_buffer, chunk_uv_buffer, chunk_uv_buffer_len);
    reuse_array_buffer(opengl_break_amount_buffer, chunk_break_amount_buffer, chunk_break_amount_buffer_len);

    this->chunk_rendering_cached = true;
    this->has_ever_cached = true;
    this->num_triangles_cache = num_triangles;
    
    this->opengl_texture_atlas = texture_atlas.get_atlas_texture();


    if (aabb.test_frustum(P*V)) {
        cached_render(P, V);
    }

    delete[] chunk_vertex_buffer;
    delete[] chunk_uv_buffer;
    delete[] chunk_break_amount_buffer;
}

// Render the chunk presuming all of its rendering data has been cached
void Chunk::cached_render(mat4& P, mat4& V) {
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
    GLuint P_matrix_shader_pointer = glGetUniformLocation(chunk_shader_id, "P");
    GLuint V_matrix_shader_pointer = glGetUniformLocation(chunk_shader_id, "V");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(P_matrix_shader_pointer, 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(V_matrix_shader_pointer, 1, GL_FALSE, &V[0][0]);
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

// Makes the buffer object. First 2 bytes of each block is block_id, 3rd byte is break_amount
pair<byte*, int> Chunk::serialize() {
    byte* buffer = new byte[SERIALIZED_CHUNK_SIZE];
    for(unsigned i = 0; i < CHUNK_SIZE; i++){
        for(unsigned j = 0; j < CHUNK_SIZE; j++){
            for(unsigned k = 0; k < CHUNK_SIZE; k++){
                short the_block_id = blocks[i][j][k].block_type;
                int index = (k*CHUNK_SIZE*CHUNK_SIZE + j*CHUNK_SIZE + i)*3;
                buffer[index] = (the_block_id >> 8) % 256;
                buffer[index + 1] = the_block_id % 256;
                buffer[index + 2] = ((int)(blocks[i][j][k].break_amount * 256)) % 256;
            }
        }
    }
    return {buffer, SERIALIZED_CHUNK_SIZE};
}

//figures out blocktype and damage from buffer object
void Chunk::deserialize(byte* buffer, int size) {
    if (size != SERIALIZED_CHUNK_SIZE) {
        printf("Error deserializing chunk size! %d", size);
        return;
    }
    for(unsigned i = 0; i < CHUNK_SIZE; i++){
        for(unsigned j = 0; j < CHUNK_SIZE; j++){
            for(unsigned k = 0; k < CHUNK_SIZE; k++){
                int index = (k*CHUNK_SIZE*CHUNK_SIZE + j*CHUNK_SIZE + i)*3;
                blocks[i][j][k].break_amount = buffer[index + 2]/256.0;
                blocks[i][j][k].block_type = buffer[index]*256 + buffer[index+1];
            }
        }
    }
}

bool Chunk::is_cached() {
    return chunk_rendering_cached;
}

void Chunk::invalidate_cache() {
    chunk_rendering_cached = false;
}
