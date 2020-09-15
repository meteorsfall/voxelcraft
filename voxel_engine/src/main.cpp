#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "input.hpp"
#include "UI.hpp"
#include "example/main_game.hpp"
#include "example/main_ui.hpp"
#include "modloader.hpp"

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

int fib(int a, int seed=1) {
	if (a < 2) {
		return 1;
	} else {
		return fib(a-1, seed) + fib(a-2, seed) + seed;
	}
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

	GLFWmonitor* primary_monitor =  glfwGetPrimaryMonitor(); // The primary monitor.. Later Occulus?..
	
	int monitor_x, monitor_y;
	glfwGetMonitorPos(primary_monitor, &monitor_x, &monitor_y);

	const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
	width = mode->width * 2 / 3;
	height = mode->height * 2 / 3;

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( width, height, "VoxelCraft", NULL, NULL);
	if( window == NULL ) {
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	// Move window to the center of the monitor
	glfwSetWindowPos(window,
		monitor_x + (mode->width - width) / 2,
		monitor_y + (mode->height - height) / 2);
	
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, resize_callback);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	if (glfwExtensionSupported("WGL_EXT_swap_control_tear") == GLFW_TRUE
 	 || glfwExtensionSupported("GLX_EXT_swap_control_tear") == GLFW_TRUE) {
		dbg("Supports adaptive vsync!");
		glfwSwapInterval(-1);
	} else {
		dbg("No adaptive vsync support");
		glfwSwapInterval(1);
	}

    // Seems legit
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    glEnable(GL_CULL_FACE);
	glEnable(GL_SAMPLE_ALPHA_TO_MASK_EXT);

    InputHandler input_handler(window);

	int frames = 0;
	double time = glfwGetTime();

	TextureRenderer texture_renderer;
	texture_renderer.set_window_dimensions(width, height);
	g_texture_renderer = &texture_renderer;
	
	// Import mods
	Mod main_mod("mods/main.wasm");

	double t55t = glfwGetTime();
	for(int i = 0; i < 10; i++) {
		dbg("Fib: %d", fib(40, i));
	}
	dbg("Compile Time: %f", (glfwGetTime() - t55t) * 1000.0);

	double tt = glfwGetTime();
	//for(int i = 0; i < 10; i++) {
		main_mod.call("init");
	//}
	dbg("WASM Time: %f", (glfwGetTime() - tt) * 1000.0);

	// MAKE GAME HERE
	Game game;
	// MAKE ALL CUSTOM UIs HERE
	MainUI main_ui(&game);
    
    // START MAIN GAME LOOP
	float last_frame_time;
	for (int frame_index = 0; true; frame_index++) {
#if FRAME_TIMER
		dbg("** Begin Frame");
#endif
		last_frame_time = glfwGetTime();

		frames++;
		if (glfwGetTime() - time > 3.0) {
			dbg("FPS: %f", frames / (glfwGetTime() - time));
			time = glfwGetTime();
			frames = 0;
		}

        // ********************
        // Input Handling
        // ********************

		double input_time = glfwGetTime();
        
		// Relative mouse will be activated iff the game is not paused
        InputState input_state = input_handler.capture_input(!game.paused);

		if (input_handler.is_exiting() || glfwWindowShouldClose(window)) {
			break;
		}
		
#if FRAME_TIMER
		dbg("Input Time: %f", (glfwGetTime() - input_time) * 1000.0);
#endif

        // ********************
        // Iterate the Game State
        // ********************
		
		double iter_timer = glfwGetTime();

		if (!game.paused) {
			main_mod.set_input_state(&input_state, sizeof(input_state));
			main_mod.call("iterate");
		}
		game.iterate(input_state);

		double iter_timer_time = (glfwGetTime() - iter_timer) * 1000.0;
#if FRAME_TIMER
		dbg("Game Iterate Time: %f", iter_timer_time);
#endif
		
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LEQUAL);

		// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        // ********************
        // Rendering Geometry
        // ********************
		
		double game_timer = glfwGetTime();

		game.render();
		main_mod.call("render");

		double game_timer_time = (glfwGetTime() - game_timer) * 1000.0;
#if FRAME_TIMER
		dbg("CPU Render Time: %f", game_timer_time);
#endif

        // ********************
        // Rendering UI Elements
        // ********************

		double ui_timer = glfwGetTime();

		// Setup transparency and disable depth buffer writing
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// Render UI
		main_ui.iterate(input_state, width, height);
		double ttt = glfwGetTime();
		if (!game.paused) {
			main_mod.call("iterate_ui");
		}
		//dbg("Iter: %f", (glfwGetTime() - ttt)*1000);
		if (main_ui.exiting) break;
		main_ui.render();
		main_mod.call("render_ui");
#if FRAME_TIMER
		dbg("UI CPU Render Time: %f", (glfwGetTime() - ui_timer)*1000.0);
#endif

		double pre_swap_time = glfwGetTime();

        // ********************
        // Display the image buffer
        // ********************
		glfwSwapBuffers(window);
		
#if FRAME_TIMER
		dbg("GPU Buffer Time: %f", (glfwGetTime() - pre_swap_time)*1000);
		dbg("Frame Time: %f", (glfwGetTime() - last_frame_time)*1000);
		dbg("** End Frame\n");
#endif
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
