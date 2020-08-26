#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "player.hpp"
#include "input.hpp"
#include "aabb.hpp"

GLFWwindow* window = NULL;

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

    // Seems legit
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_CULL_FACE);

    Texture stone_texture("assets/stone.bmp", "assets/simple.vert", "assets/simple.frag");
    Texture dirt_texture("assets/dirt.bmp", "assets/simple.vert", "assets/simple.frag");
 
    BlockType stone_block = BlockType(&stone_texture);
    BlockType dirt_block = BlockType(&dirt_texture);
    
    World my_world;

    // 1-7, dirt 8-16 stone
    my_world.set_block(0,0,0, &dirt_block);
    my_world.set_block(0,1,0, &stone_block);
    
    for(int i = 0; i < CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < CHUNK_SIZE; k++) {
                if (j <= 7) {
                    my_world.set_block(i,j,k, &stone_block);
                } else {
                    my_world.set_block(i,j,k, &dirt_block);
                }
            }
        }
    }

    Player my_player;
	my_player.hand = &stone_block;
    Input input_handler(window, &my_world, &my_player);
    
    // START MAIN GAME LOOP
	while(true) {
        // ********************
        // Input Handling
        // ********************
        
        input_handler.handle_input();
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS || glfwWindowShouldClose(window)) {
			break;
		}

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ********************
        // Rendering
        // ********************
        mat4 PV = my_player.camera.get_camera_matrix();
        my_world.render(PV);

        // ********************
        // Display the image buffer
        // ********************
		glfwSwapBuffers(window);
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
