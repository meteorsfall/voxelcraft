#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "player.hpp"
#include "input.hpp"
#include "aabb.hpp"
#include "UI.hpp"
#include "text.hpp"

GLFWwindow* window = NULL;

static int width = 600;
static int height = 400;

void resize_callback(GLFWwindow* window, int w, int h) {
	UNUSED(window);
	width = w;
	height = h;
}

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
	window = glfwCreateWindow( width, height, "VoxelCraft", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, resize_callback);

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

    Texture stone_texture("assets/images/stone.bmp", "assets/shaders/simple.vert", "assets/shaders/simple.frag");
    Texture dirt_texture("assets/images/dirt.bmp", "assets/shaders/simple.vert", "assets/shaders/simple.frag");
 
    BlockType stone_block = BlockType(&stone_texture);
    BlockType dirt_block = BlockType(&dirt_texture);
    
    World my_world;

    // 1-7, dirt 8-16 stone
    my_world.set_block(0,0,0, &dirt_block);
    my_world.set_block(0,1,0, &stone_block);

	Font f("assets/fonts/pixel.ttf");
    
    for(int i = 0; i < 2*CHUNK_SIZE; i++) {
        for(int j = 0; j < CHUNK_SIZE; j++) {
            for(int k = 0; k < 2*CHUNK_SIZE; k++) {
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

	UI ui;
	Texture crosshair_texture("assets/images/crosshair.bmp", "assets/shaders/ui.vert", "assets/shaders/ui.frag", true);
	Texture ui_test("assets/images/boxes_test.bmp", "assets/shaders/ui.vert", "assets/shaders/ui.frag", true);

	int frames = 0;
	double time = glfwGetTime();
    
    // START MAIN GAME LOOP
	while(true) {
		frames++;
		if (glfwGetTime() - time > 3.0) {
			printf("FPS: %f\n", frames / (glfwGetTime() - time));
			time = glfwGetTime();
			frames = 0;
		}

        // ********************
        // Input Handling
        // ********************
        
		glfwPollEvents();
        input_handler.handle_input();

		if (input_handler.exiting || glfwWindowShouldClose(window)) {
			break;
		}

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS);

		// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_TRUE);

        // ********************
        // Rendering Geometry
        // ********************

		double t1 = glfwGetTime();

		// Get Projection-View matrix
        mat4 PV = my_player.camera.get_camera_matrix(width / (float)height);

		// Render
        my_world.render(PV);

		printf("Time: %f\n", 1/(glfwGetTime() - t1));

        // ********************
        // Rendering UI Elements
        // ********************

		// Setup transparency and disable depth buffer writing
		glDepthMask(GL_FALSE);
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
		// Render
		float crosshair_size = 0.025;
		ui.render(&crosshair_texture, vec2(0.0, 0.0), crosshair_size, crosshair_size * width / height);
		//ui.render(&ui_test, vec2(0.0, 0.0), 0.2, 0.1);
		f.render(ivec2(width, height), ivec2(width / 80, height / 80), 0.3, "VoxelCraft v0.1.0", ivec3(240, 0, 0));

		// Restore gl settings
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

        // ********************
        // Display the image buffer
        // ********************
		glfwSwapBuffers(window);
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
