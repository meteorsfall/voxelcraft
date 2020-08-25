#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "player.hpp"

#define WIDTH 1920
#define HEIGHT 1080

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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    double lastTime = glfwGetTime();

    glEnable(GL_CULL_FACE);

    Texture stone_texture("assets/stone.bmp", "assets/simple.vert", "assets/simple.frag");
    Texture dirt_texture("assets/dirt.bmp", "assets/simple.vert", "assets/simple.frag");
 
    Block stone_block = Block(&stone_texture);
    Block dirt_block = Block(&dirt_texture);
    
    World my_world;

    // 1-7, dirt 8-16 stone
    my_world.set_block(0,0,0, &dirt_block);
    my_world.set_block(0,1,0, &stone_block);
    
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            for(int k = 0; k < 16; k++) {
                if (j <= 7) {
                    my_world.set_block(i,j,k, &stone_block);
                } else {
                    my_world.set_block(i,j,k, &dirt_block);
                }
            }
        }
    }

    Player my_player;
    
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
        mat4 PV = my_player.camera.get_camera_matrix();
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
        movement.x *= speed;
        movement.y *= speed;
        movement.z *= speed;
        vec3 old_position = my_player.position;
        my_player.move_toward(movement, deltaTime);
        const float JUMP_INITIAL_VELOCITY = 4.0;
        vec3 jump_velocity = vec3(0.0);
        if (glfwGetKey(window, GLFW_KEY_SPACE) && my_player.is_on_floor) {
            vec3 dir = my_player.position - old_position;
            jump_velocity = vec3(dir.x, 9.8, dir.z);
            jump_velocity = normalize(jump_velocity) * JUMP_INITIAL_VELOCITY;
        }
        my_player.move(jump_velocity, vec3(0.0, -9.8, 0.0), deltaTime);
        my_player.rotate(mouse_rotation);
        
        my_world.collide(my_player.position, my_player.get_on_collide());
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
