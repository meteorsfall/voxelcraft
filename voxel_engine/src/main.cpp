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

static UINT SC_CALLBACK sciter_handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam);

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void resize_callback(GLFWwindow* win, int w, int h) {
    if (win != window) {
        dbg("Unknown GLFW windows: %p, expected %p", (void*)win, (void*)window);
    }
    width = w;
    height = h;
    get_texture_renderer()->set_window_dimensions(width, height);
    // Update Sciter width/height
    SciterProcX(window, SCITER_X_MSG_SIZE(width, height));
}

int fib(int a, int seed=1) {
    if (a < 2) {
        return 1;
    } else {
        return fib(a-1, seed) + fib(a-2, seed) + seed;
    }
}

void render_sciter();

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

    // Initializer Sciter
    SciterSetOption(NULL, SCITER_SET_UX_THEMING, TRUE); // Removes system-dependent CSS
    SciterProcX(window, SCITER_X_MSG_CREATE(GFX_LAYER_SKIA_OPENGL, FALSE));
    // Set Sciter DPI
    float xscale;
    glfwGetWindowContentScale(window, &xscale, NULL);
    SciterProcX(window, SCITER_X_MSG_RESOLUTION(96*xscale));
    // Set Sciter width/height
    SciterProcX(window, SCITER_X_MSG_SIZE(width, height));
    // Set Sciter callback
    SciterSetCallback(window, sciter_handle_notification, nullptr);
    const char html[] = /* utf8 BOM */ "\xEF\xBB\xBF" /* HTML */ R"ENDHTML(
<html>
<head>
    <style>
    html, body {background: transparent;}
    daiv {background: white;}
    </style>
</head>
<body>
    <div>
        <h1 id="titlebar">Hello</h1>
        <p>World!</p>
    </div>
    <script>
        document.getElementById("titlebar").innerHTML="Javascript!";
    </script>
</body>
</html>)ENDHTML";
    if (SciterLoadHtml(window, (LPCBYTE)html, strlen(html), nullptr) != TRUE) {
        dbg("Failed to load HTML");
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
        
        // Render UI
        main_mod.call("render_ui");
        // Render Sciter
        render_sciter();

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

    // Close Sciter
    SciterProcX(window, SCITER_X_MSG_DESTROY());
    // Close GLFW window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// ================
// Sciter Functions
// ================

void render_sciter() {
    // Static variables to store Sciter OpenGL Context
    static GLint sciter_vertex_array = 0;
    static GLint sciter_texture_unit = GL_TEXTURE0;
    static GLint sciter_program = 0;

    // Load up Sciter Opengl Context from saved values or known constants
    glBindVertexArray(sciter_vertex_array);
    glActiveTexture(sciter_texture_unit);
    glUseProgram(sciter_program);
    glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // Open all vertex attributes
    GLint max_vertex_attribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
    for(int i = 0; i < max_vertex_attribs; i++) {
        glEnableVertexAttribArray(i);
    }

    // Give Sciter the time, so that it can synchronize animations
    SciterProcX(window, SCITER_X_MSG_HEARTBIT(UINT(glfwGetTime() * 1000)));
    // Render HTML/CSS with Sciter using Sciter OpenGL context
    SciterProcX(window, SCITER_X_MSG_PAINT());

    // Close all vertex attributes
    for(int i = 0; i < max_vertex_attribs; i++) {
        glDisableVertexAttribArray(i);
    }

    // Save Sciter OpenGL Context
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &sciter_vertex_array);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &sciter_texture_unit);
    glGetIntegerv(GL_CURRENT_PROGRAM, &sciter_program);
}

// See sciter-sdk/demos.lite/sciter-glfw-opengl/basic.cpp for examples

UINT SC_CALLBACK sciter_handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam)
{
    UNUSED(pnm);
    UNUSED(callbackParam);
    
    switch (pnm->code) {
        case SC_LOAD_DATA:
            return 0; // Unimplemented
        case SC_DATA_LOADED:
            return 0; // Unimplemented
        case SC_ATTACH_BEHAVIOR:
            return 0; // Unimplemented
        case SC_INVALIDATE_RECT:
            // We draw on every frame, so the rectangle being invalidated isn't relevant to us
            return 0;
        case SC_ENGINE_DESTROYED: break;
    }
    return 0;
}
