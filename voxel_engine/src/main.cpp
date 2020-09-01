#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "input.hpp"
#include "UI.hpp"
#include "example/main_game.hpp"
#include "example/main_ui.hpp"

TextureRenderer* g_texture_renderer;
GLFWwindow* window = NULL;

extern bool paused;

static int width = 600;
static int height = 400;

void resize_callback(GLFWwindow* win, int w, int h) {
	UNUSED(win);
	width = w;
	height = h;
	get_texture_renderer()->set_window_dimensions(width, height);
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

	GLFWmonitor* MyMonitor =  glfwGetPrimaryMonitor(); // The primary monitor.. Later Occulus?..

	const GLFWvidmode* mode = glfwGetVideoMode(MyMonitor);
	width = mode->width;
	height = mode->height;

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( width, height, "VoxelCraft", NULL, NULL);
	if( window == NULL ) {
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

	glfwSwapInterval(1);

    // Seems legit
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_CULL_FACE);

    InputHandler input_handler(window);

	int frames = 0;
	double time = glfwGetTime();

	TextureRenderer texture_renderer;
	texture_renderer.set_window_dimensions(width, height);
	g_texture_renderer = &texture_renderer;

	// MAKE GAME HERE
	Game game;
	// MAKE ALL CUSTOM UIs HERE
	MainUI main_ui(&game);
    
    // START MAIN GAME LOOP
	for (int frame_index = 0; true; frame_index++) {
		frames++;
		if (glfwGetTime() - time > 3.0) {
			printf("FPS: %f\n", frames / (glfwGetTime() - time));
			time = glfwGetTime();
			frames = 0;
		}

        // ********************
        // Input Handling
        // ********************
        
        InputState input_state = input_handler.handle_input(!game.paused);

		if (input_handler.exiting || glfwWindowShouldClose(window)) {
			break;
		}

        // ********************
        // Iterate the Game State
        // ********************

		game.iterate(input_state);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
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

		game.render();

		if (frame_index % 240 == 0) {
			printf("CPU Render Time: %f\n", 1/(glfwGetTime() - t1));
		}

        // ********************
        // Rendering UI Elements
        // ********************

		// Setup transparency and disable depth buffer writing
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// Render UI
		main_ui.iterate(input_state, width, height);
		if (main_ui.exiting) break;
		main_ui.render();

		//static float last_frame_time = 0.0;
		//printf("Frame Time: %f\n", (glfwGetTime() - last_frame_time)*1000);

        // ********************
        // Display the image buffer
        // ********************
		glfwSwapBuffers(window);

		//last_frame_time = glfwGetTime();
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
