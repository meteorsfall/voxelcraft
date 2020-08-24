// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include Util file
#include "utils.hpp"
#include "camera.hpp"

#define WIDTH 1024
#define HEIGHT 768

GLFWwindow* window;

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, HEIGHT, "VoxelCraft", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Seems legit
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    for(int i = 0; i < len(g_uv_buffer_data); i++) {
        if (g_uv_buffer_data[i] < 0.1) {
            g_uv_buffer_data[i] = 0;
        } else if (g_uv_buffer_data[i] < 0.4) {
            g_uv_buffer_data[i] = 1.0f / 3.0f;
        } else if (g_uv_buffer_data[i] < 0.7) {
            g_uv_buffer_data[i] = 2.0f / 3.0f;
        } else {
            g_uv_buffer_data[i] = 1.0f;
        }
    }

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
    // Make GL_ARRAY_BUFFER point to colorbuffer
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    // Give our vertices to GL_ARRAY_BUFFER (ie, colorbuffer)
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "assets/simple.vert", "assets/simple.frag" );

    float camera_x = 3;
	float fov = 45.0;

    GLuint my_texture_id = loadBMP("assets/stone.bmp");

    Camera camera;

	// Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    double lastTime = glfwGetTime();

    glEnable(GL_CULL_FACE);

    // START MAIN GAME LOOP
	do {
        double currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(programID);

        // Calculate MVP Matrix
        mat4 PV = camera.get_camera_matrix();
        
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 Model = glm::mat4(1.0f);
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 mvp = PV * Model; // Remember, matrix multiplication is the other way around

		// Get a handle for our "MVP" uniform
        // Only during the initialisation
        GLuint MatrixID = glGetUniformLocation(programID, "MVP");
        
        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        //"vertex_shader.MVP = &mvp[0][0]"

        GLuint shader_texture_id = glGetUniformLocation(programID, "myTextureSampler");
        // shader_texture_id = &fragment_shader.myTextureSampler;
        
        glActiveTexture(GL_TEXTURE0);
        // gl_internal_texture = 0;
        glBindTexture(GL_TEXTURE_2D, my_texture_id);
        // GL_TEXTURE_2D[gl_internal_texture] = my_texture_id;
        glUniform1i(shader_texture_id, 0);
        // *shader_texture_id = GL_TEXTURE_2D[0]

		// Draw nothing, see you in tutorial 2 !
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
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
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
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
		
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

        // Input Handling

        float mouseSpeed = 0.1;
        
        vec2 mouse_rotation;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwSetCursorPos(window, WIDTH/2, HEIGHT/2);
        mouse_rotation.x = mouseSpeed * deltaTime * float(WIDTH/2 - xpos );
        mouse_rotation.y = mouseSpeed * deltaTime * float( HEIGHT/2 - ypos );

        vec3 movement = vec3(0.0, 0.0, 0.0);
        if (glfwGetKey(window, GLFW_KEY_W)) {
            movement.x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            movement.x -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            movement.y += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            movement.y -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            movement.z += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            movement.z -= 1;
        }
        float speed = 3.3;
        movement.x *= speed * deltaTime;
        movement.y *= speed * deltaTime;
        movement.z *= speed * deltaTime;
        camera.move(movement);
        camera.rotate(mouse_rotation);

		if(glfwGetKey(window, GLFW_KEY_N)) {
            fov += 0.6;
        }

        if (glfwGetKey(window, GLFW_KEY_M)) {
            fov -= 0.6;
        }
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
