#include "world.hpp"

Texture::Texture(const char* filename, const char* vertex_shader, const char* fragment_shader) {
    this->opengl_texture_id = loadBMP(filename);
    
    // Create and compile our GLSL program from the shaders
    this->shader_id = LoadShaders(vertex_shader, fragment_shader);
}

Block::Block(Texture* texture) {
    this->texture = texture;

    // Save the cube vertex buffer data to this->vertex_buffer

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &this->vertex_buffer);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_vertex_buffer_data), g_cube_vertex_buffer_data, GL_STATIC_DRAW);

    // Save the UV buffer data to this->uv_buffer
    for(int i = 0; i < len(g_cube_uv_buffer_data); i++) {
        if (g_cube_uv_buffer_data[i] < 0.1) {
            g_cube_uv_buffer_data[i] = 0;
        } else if (g_cube_uv_buffer_data[i] < 0.4) {
            g_cube_uv_buffer_data[i] = 1.0f / 3.0f;
        } else if (g_cube_uv_buffer_data[i] < 0.7) {
            g_cube_uv_buffer_data[i] = 2.0f / 3.0f;
        } else {
            g_cube_uv_buffer_data[i] = 1.0f;
        }
    }

    glGenBuffers(1, &this->uv_buffer);
    // Make GL_ARRAY_BUFFER point to uvbuffer
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, uvbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_cube_uv_buffer_data), g_cube_uv_buffer_data, GL_STATIC_DRAW);
}

void Block::render(vec3 &position, mat4 &PV) {
    glUseProgram(this->texture->shader_id);
    
    GLuint shader_texture_id = glGetUniformLocation(this->texture->shader_id, "my_texture");
    // shader_texture_id = &fragment_shader.myTextureSampler;
    
    glActiveTexture(GL_TEXTURE0);
    // gl_internal_texture = 0;
    glBindTexture(GL_TEXTURE_2D, this->texture->opengl_texture_id);
    // GL_TEXTURE_2D[gl_internal_texture] = my_texture_id;
    glUniform1i(shader_texture_id, 0);
    // *shader_texture_id = GL_TEXTURE_2D[0]
    
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::translate(glm::mat4(1.0f), position);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = PV * Model; // Remember, matrix multiplication is the other way around

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint MatrixID = glGetUniformLocation(this->texture->shader_id, "MVP");
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
    //"vertex_shader.MVP = &mvp[0][0]"

    // Draw nothing, see you in tutorial 2 !
    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, this->uv_buffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);
}

World::World() {
    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i][j][k] = nullptr;
            }
        }
    }
}

void World::set_block(int x, int y, int z, Block* b) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return;
    }
    blocks[x][y][z] = b;
}

Block* World::get_block(int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return nullptr;
    }
    return blocks[x][y][z];
}

void World::render(mat4 &PV) {
    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                // If the block exists
                bool visible = !get_block(i-1, j, k) || !get_block(i+1, j, k)
                            || !get_block(i, j-1, k) || !get_block(i, j+1, k)
                            || !get_block(i, j, k-1) || !get_block(i, j, k+1);
                if (blocks[i][j][k] && visible) {
                    // Render it at i, j, k
                    vec3 position((float)i, (float)j, (float)k);
                    blocks[i][j][k]->render(position, PV);
                }
            }
        }
    }
}

bool World::is_in_block(vec3 position) {
    int x = position.x;
    int y = position.y;
    int z = position.z;
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return false;
    }
    return blocks[x][y][z];
}

optional<ivec3> World::raycast(vec3 position, vec3 direction, float max_distance, bool nextblock) {
    float ray = 0.01;
    direction = normalize(direction);
    for(int i = 0; i < max_distance/ray; i++){
        vec3 n = position + direction*ray*(float)i;
        if(is_in_block(position + direction*ray*(float)i)){
            if(nextblock){
                ivec3 loc = floor(position + direction*ray*(float)(i-1));
                return {loc};
            }
            return { ivec3(floor(position + direction*ray*(float)i)) };
        }
    }
    return nullopt;
}

void World::collide(AABB collision_box, fn_on_collide on_collide) {
    ivec3 bottom_left = ivec3(floor(collision_box.min_point) - vec3(1.0));
    ivec3 top_right = ivec3(ceil(collision_box.max_point));
    vec3 total_movement(0.0);
    for(int x = bottom_left.x; x <= top_right.x; x++) {
        for(int y = bottom_left.y; y <= top_right.y; y++) {
            for(int z = bottom_left.z; z <= top_right.z; z++) {
                if (get_block(x, y, z)) {
                    vec3 box(x, y, z);
                    AABB static_box(box, box + vec3(1.0));
                    optional<vec3> movement_o = static_box.collide(collision_box);
                    if (movement_o) {
                        vec3 movement = movement_o.value();
                        total_movement += movement;
                        collision_box.translate(movement);
                    }
                }
            }
        }
    }
    if (length(total_movement) > 0.0) {
        // Call the listener if a collision occured
        on_collide(total_movement);
    }
}