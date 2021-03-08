#include "chunk.hpp"
#include "gl_utils.hpp"
#include <cstring>
#include "texture_atlasser.hpp"
#include "universe.hpp"

static bool loaded_chunk_shader = false;
static GLuint chunk_shader_id;

Chunk::Chunk() {
    // Statically load chunk shaders
    if (!loaded_chunk_shader) {
        chunk_shader_id = load_shaders("assets/shaders/chunk.vert", "assets/shaders/chunk.frag");
        loaded_chunk_shader = true;
    }
}

void Chunk::set_block(int x, int y, int z, int model) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        printf("Bad coordinates! %d %d %d\n", x, y, z);
        return;
    }
    blocks[x][y][z] = BlockData(model);
}

BlockData* Chunk::get_block(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return nullptr;
    }
    // Return nullptr if it's an air block
    return blocks[x][y][z].block_model ? &blocks[x][y][z] : nullptr;
}

void Chunk::render(const mat4& P, const mat4& V, ivec3 location, const TextureAtlasser& texture_atlas, fn_get_block master_get_block, bool dont_rerender) {
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
    static GLfloat chunk_vertex_buffer[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3*3];
    int chunk_vertex_buffer_len = 0;
    // 12 triangles in a cube, 3 vertices in a triangle, 2 uv floats in a vertex
    static GLfloat chunk_uv_buffer[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3*2];
    int chunk_uv_buffer_len = 0;
    // 1 break amount per cube
    static GLfloat chunk_break_amount_buffer[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*12*3];
    int chunk_break_amount_buffer_len = 0;

    int num_triangles = 0;

    auto is_opaque = [this](BlockData* b, int dir) {
        // If it exists, and opaque
        if (b) {
            if (b->block_model == 0) {
                return false;
            }
            // If any of the components are opaque in the given direction,
            // then this block is opaque in that direction

            const vector<vector<int>>& components = get_universe()->get_model(b->block_model)->generate_model_instance(map<string,string>{});
            bool opaque = false;
            for(uint i = 0; i < components.size(); i++) {
                opaque |= get_universe()->get_component(components[i][0])->get_opacities()[dir];
            }
            return opaque;
        } else {
            return false;
        }
    };
    
    double t1 = glfwGetTime();
    UNUSED(t1);

    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                // If the block has a block type of 0, just skip it (It's an air block)
                if (!blocks[i][j][k].block_model) {
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
                    if (!is_opaque(get_neighboring_block(-1, 0, 0, i == 0), 1)) {
                        visible_neighbors |= 1 << 0;
                    }
                    if (!is_opaque(get_neighboring_block(1, 0, 0, i == CHUNK_SIZE-1), 0)) {
                        visible_neighbors |= 1 << 1;
                    }
                    if (!is_opaque(get_neighboring_block(0, -1, 0, j == 0), 3)) {
                        visible_neighbors |= 1 << 2;
                    }
                    if (!is_opaque(get_neighboring_block(0, 1, 0, j == CHUNK_SIZE-1), 2)) {
                        visible_neighbors |= 1 << 3;
                    }
                    if (!is_opaque(get_neighboring_block(0, 0, -1, k == 0), 5)) {
                        visible_neighbors |= 1 << 4;
                    }
                    if (!is_opaque(get_neighboring_block(0, 0, 1, k == CHUNK_SIZE-1), 4)) {
                        visible_neighbors |= 1 << 5;
                    }
                    blocks[i][j][k].neighbor_cache = visible_neighbors;
                }
                
                if (visible_neighbors & ~(1 << 7)) {
                    // Generate boolean array from visible neighbor bitset
                    // This will represent which faces to include and which to cull
                    vec3 fpos(position);

                    int block_model = blocks[i][j][k].block_model;

                    const vector<ComponentPossibilities>& sm = get_universe()->get_model(block_model)->generate_model_instance(map<string,string>{});
                    for(const ComponentPossibilities& cp : sm) {
                        int component_id = cp[0];

                        bool visible_neighbors_array[6];
                        for(int ar = 0; ar < 6; ar++) {
                            visible_neighbors_array[ar] = (visible_neighbors >> ar) & 1;
                        }
                        auto p = get_universe()->get_component(component_id)->get_mesh_data(visible_neighbors_array);
                        vec3* vertex_buf = (vec3*)get<0>(p);
                        vec2* uv_buf = (vec2*)get<1>(p);
                        int num_model_triangles = get<2>(p);

                        // Loop over every triangle in the model
                        for(int tri = 0; tri < num_model_triangles; tri++) {
                            // Loop over each vertex and add it to the chunk buffer
                            for(int vert = 0; vert < 3; vert++) {
                                vec3 vertex = vertex_buf[tri*3 + vert] + fpos;
                                chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)] = vertex.x;
                                chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)+1] = vertex.y;
                                chunk_vertex_buffer[chunk_vertex_buffer_len/sizeof(GLfloat)+2] = vertex.z;
                                chunk_vertex_buffer_len += 3*sizeof(GLfloat);
                                vec2 uv = uv_buf[tri*3 + vert];
                                chunk_uv_buffer[chunk_uv_buffer_len/sizeof(GLfloat)] = uv.x;
                                chunk_uv_buffer[chunk_uv_buffer_len/sizeof(GLfloat)+1] = uv.y;
                                chunk_uv_buffer_len += 2*sizeof(GLfloat);
                                chunk_break_amount_buffer[chunk_break_amount_buffer_len/sizeof(GLfloat)] = blocks[i][j][k].break_amount;
                                chunk_break_amount_buffer_len += sizeof(GLfloat);
                            }
                        }

                        num_triangles += num_model_triangles;
                    }
                }
            }
        }
    }
    
    double time = (glfwGetTime() - t1) * 1000.0;
    if (time > 4) {
        //dbg("Chunk Construction Time: %f", time);
    }

    if (chunk_vertex_buffer_len > (int)sizeof(chunk_vertex_buffer)) {
        dbg("BAD vertex buffer len! %d", chunk_vertex_buffer_len);
    }
    if (chunk_uv_buffer_len > (int)sizeof(chunk_uv_buffer)) {
        dbg("BAD UV buffer len! %d", chunk_uv_buffer_len);
    }
    if (chunk_break_amount_buffer_len > (int)sizeof(chunk_break_amount_buffer)) {
        dbg("BAD vertex buffer len! %d", chunk_break_amount_buffer_len);
    }

    opengl_vertex_buffer.reuse(chunk_vertex_buffer, chunk_vertex_buffer_len);
    opengl_uv_buffer.reuse(chunk_uv_buffer, chunk_uv_buffer_len);
    opengl_break_amount_buffer.reuse(chunk_break_amount_buffer, chunk_break_amount_buffer_len);

    this->chunk_rendering_cached = true;
    this->has_ever_cached = true;
    this->num_triangles_cache = num_triangles;
    
    this->opengl_texture_atlas_cache = texture_atlas.get_atlas_texture();

    if (aabb.test_frustum(P*V)) {
        cached_render(P, V);
    }
}

// Render the chunk presuming all of its rendering data has been cached
void Chunk::cached_render(const mat4& P, const mat4& V) {
    if (num_triangles_cache == 0) {
        // No need to render if there are no triangles
        return;
    }

    glUseProgram(chunk_shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(chunk_shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    bind_texture(0, shader_texture_id, opengl_texture_atlas_cache);

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
    opengl_vertex_buffer.bind(0, 3);

    // 2nd attribute buffer : colors
    opengl_uv_buffer.bind(1, 2);

    // 3rd attribute buffer : break amount
    opengl_break_amount_buffer.bind(2, 1);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, num_triangles_cache*3); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

// Makes the buffer object. First 2 bytes of each block is block_id, 3rd byte is break_amount
pair<byte*, int> Chunk::serialize() {
    static byte buffer[SERIALIZED_CHUNK_SIZE];
    for(unsigned i = 0; i < CHUNK_SIZE; i++){
        for(unsigned j = 0; j < CHUNK_SIZE; j++){
            for(unsigned k = 0; k < CHUNK_SIZE; k++){
                short the_block_id = blocks[i][j][k].block_model;
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
                blocks[i][j][k].block_model = buffer[index]*256 + buffer[index+1];
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
