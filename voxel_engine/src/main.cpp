#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "input.hpp"
#include "html_renderer.hpp"
#include "example/main_game.hpp"
#include "example/main_ui.hpp"
#include "modloader.hpp"

TextureRenderer* g_texture_renderer;
GLFWwindow* window = NULL;
HTMLRenderer html_renderer;

extern bool paused;

static int width = 600;
static int height = 400;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void resize_callback(GLFWwindow* win, int w, int h) {
    if (win != window) {
        dbg("Unknown GLFW window: %p, expected %p", (void*)win, (void*)window);
        return;
    }
    width = w;
    height = h;
    get_texture_renderer()->set_window_dimensions(width, height);
}

int main( void )
{
    // Initialise GLFW
    glfwSetErrorCallback(glfw_error_callback);
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
        fprintf( stderr, "Failed to open GLFW %dx%d window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n", width, height );
        getchar();
        glfwTerminate();
        return -1;
    }

    html_renderer.init(window);
    if (!html_renderer.load_html(WSTR("file://html/menu.html"))) {
        dbg("Failed to load HTML");
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

    // Makes a vertex array object for our graphics engine
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);

    InputHandler input_handler(window);

    // Set a black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);

    TextureRenderer texture_renderer;
    texture_renderer.set_window_dimensions(width, height);
    g_texture_renderer = &texture_renderer;
    
    // Import mods
    Mod main_mod("mods/main.wasm");
    main_mod.call("initialize");
    
    // ********************
    // START MAIN GAME LOOP
    // ********************

    int frames_since_last_fps_calculation = 0;
    double time_since_last_fps_calculation = glfwGetTime();
    float last_frame_time = 0;
    for (int frame_index = 0; true; frame_index++) {
#if FRAME_TIMER
        dbg("** Begin Frame");
#endif
        while ((glfwGetTime() - last_frame_time) < 1/90.0);
        last_frame_time = glfwGetTime();

        frames_since_last_fps_calculation++;
        // Once 3 seconds since an FPS calculation passes,
        // print the average FPS over the past ~3 seconds
        if (glfwGetTime() - time_since_last_fps_calculation > 3.0) {
            dbg("FPS: %f", frames_since_last_fps_calculation / (glfwGetTime() - time_since_last_fps_calculation));
            time_since_last_fps_calculation = glfwGetTime();
            frames_since_last_fps_calculation = 0;
        }

        // ********************
        // Capture input
        // ********************

        double input_time = glfwGetTime();
        
        // Relative mouse will be activated iff the game is not paused
        InputState input_state = input_handler.capture_input(true);

        if (input_handler.is_exiting() || glfwWindowShouldClose(window)) {
            // Exit if their input tried to exit, or if glfw detected a close event (E.g. like the 'X' button)
            break;
        }
        
#if FRAME_TIMER
        dbg("Input Time: %f", (glfwGetTime() - input_time) * 1000.0);
#endif

        // ********************
        // Iterate the Game State
        // ********************
        
        double iterate_timer = glfwGetTime();

        // Pass player input, and then iterate the game state
        main_mod.set_input_state(&input_state, sizeof(input_state));
        main_mod.call("iterate");

        double iterate_time = (glfwGetTime() - iterate_timer) * 1000.0;
#if FRAME_TIMER
        dbg("Game Iterate Time: %f", iterate_time);
#endif
        
        // ********************
        // Render Geometry
        // ********************

        // NOTE: Due to Sciter, do NOT glBindTexture any textures to GL_TEXTURE0 OR GL_TEXTURE31 while rendering
        
        double render_timer = glfwGetTime();
        
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LEQUAL);
        // Setup other OpenGL parameters
        glBindVertexArray(VertexArrayID);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_SAMPLE_ALPHA_TO_MASK_EXT);

        // Set the viewport based on current width/height
        glViewport(0, 0, width, height);
        // Clear the screen to black, helps prevent flickering
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the game!
        main_mod.call("render");

        double render_time = (glfwGetTime() - render_timer) * 1000.0;
#if FRAME_TIMER
        dbg("CPU Render Time: %f", render_time);
#endif

        // ********************
        // Rendering UI Elements
        // ********************

        double ui_timer = glfwGetTime();

        // Setup transparency and disable depth buffer writing
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glDisable(GL_SAMPLE_ALPHA_TO_MASK_EXT);
        glDisable(GL_MULTISAMPLE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Render Mod UI
        main_mod.call("render_ui");
        // Render HTML UI
        html_renderer.render(width, height);

        double ui_time = (glfwGetTime() - ui_timer) * 1000.0;
#if FRAME_TIMER
        dbg("UI CPU Render Time: %f", ui_time);
#endif

        // ********************
        // Display the image buffer
        // ********************

        double swap_buffer_timer = glfwGetTime();
        glfwSwapBuffers(window);
        
        double swap_buffer_time = (glfwGetTime() - swap_buffer_timer) * 1000.0;
#if FRAME_TIMER
        dbg("GPU Buffer Time: %f", swap_buffer_time);
        dbg("Frame Time: %f", (glfwGetTime() - last_frame_time)*1000);
        dbg("** End Frame\n");
#endif
    }

    // Close GLFW window and terminate GLFW
    html_renderer.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
