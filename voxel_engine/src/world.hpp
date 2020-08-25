#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "utils.hpp"
#include <vector>
#include <functional>

using fn_on_collide = std::function<void(vec3)>;

class Texture {
public:
    int texture_id;
    GLuint opengl_texture_id;
    GLuint shader_id;

    Texture(const char* filename, const char* vertex_shader, const char* fragment_shader) {
        this->opengl_texture_id = loadBMP(filename);
        
        // Create and compile our GLSL program from the shaders
        this->shader_id = LoadShaders(vertex_shader, fragment_shader);
    }
};

class Block {
public:
    // This will identify our vertex buffer
    GLuint vertex_buffer;
    GLuint uv_buffer;

    int block_id;
    Texture* texture;

    Block(Texture* texture) {
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

    void render(vec3 &position, mat4 &PV){
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
};

class World {
public:
    int world_id;
    Block* blocks[16][16][16];

    World() {
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 16; j++) {
                for(int k = 0; k < 16; k++) {
                    blocks[i][j][k] = nullptr;
                }
            }
        }
    }

    void set_block(int x, int y, int z, Block* b);

    Block* get_block(int x, int y, int z);

    void render(mat4 &PV) {
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 16; j++) {
                for(int k = 0; k < 16; k++) {
                    // If the block exists
                    if (blocks[i][j][k]) {
                        // Render it at i, j, k
                        vec3 position((float)i, (float)j, (float)k);
                        blocks[i][j][k]->render(position, PV);
                    }
                }
            }
        }
    }

    bool is_in_block(vec3 position) {
        int x = position.x;
        int y = position.y;
        int z = position.z;
        if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
            return false;
        }
        return blocks[x][y][z];
    }

    void collide(vec3 position, fn_on_collide on_collide) {
        if (is_in_block(position)) {
            // Round to the nearest block
            vec3 rounded_position = round(position);
            vec3 diff = abs(rounded_position - position);
            float smallest_escape = 1.0;

            vec3 new_position = position;

            // Check x direction
            if (!is_in_block(position - vec3(1.0, 0.0, 0.0)) && abs(position.x - floor(position.x)) < smallest_escape) {
                smallest_escape = abs(position.x - floor(position.x));
                new_position.x = floor(position.x);
            }
            if (!is_in_block(position + vec3(1.0, 0.0, 0.0)) && abs(position.x - ceil(position.x)) < smallest_escape) {
                smallest_escape = abs(position.x - ceil(position.x));
                new_position.x = ceil(position.x);
            }
            // Check y direction
            if (!is_in_block(position + vec3(0.0, 1.0, 0.0)) && abs(position.y - ceil(position.y)) < smallest_escape) {
                smallest_escape = abs(position.y - ceil(position.y));
                new_position.y = ceil(position.y);
            }
            if (!is_in_block(position - vec3(0.0, 1.0, 0.0)) && abs(position.y - floor(position.y)) < smallest_escape) {
                smallest_escape = abs(position.y - floor(position.y));
                new_position.y = floor(position.y);
            }
            // Check z direction
            if (!is_in_block(position + vec3(0.0, 0.0, 1.0)) && abs(position.z - ceil(position.z)) < smallest_escape) {
                smallest_escape = abs(position.z - ceil(position.z));
                new_position.z = ceil(position.z);
            }
            if (!is_in_block(position - vec3(0.0, 0.0, 1.0)) && abs(position.z - floor(position.z)) < smallest_escape) {
                smallest_escape = abs(position.z - floor(position.z));
                new_position.z = floor(position.z);
            }

            // Call the listener
            on_collide(new_position);
        }
    }
};

class Universe {
public:
    vector<Texture> textures;
    vector<Block> blocks;
    vector<World> worlds;

    Universe() {
    }
};

#endif
