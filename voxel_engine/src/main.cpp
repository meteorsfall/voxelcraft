#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "camera.hpp"
#include "world.hpp"

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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    float camera_x = 3;
	float fov = 45.0;

    Camera camera;

	// Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    double lastTime = glfwGetTime();

    glEnable(GL_CULL_FACE);

    Texture my_texture("assets/stone.bmp", "assets/simple.vert", "assets/simple.frag");

    Block my_block = Block(&my_texture);
    World my_world;

    my_world.set_block(0,0,0, &my_block);
    my_world.set_block(0,1,0, &my_block);
    my_world.set_block(1,0,0, &my_block);
    
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

        // RENDER ALL THE THINGS HERE
        mat4 PV = camera.get_camera_matrix();
        my_world.render(PV);

		// Swap buffers, shows the image
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
