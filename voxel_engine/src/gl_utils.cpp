#include "gl_utils.hpp"
#include <fstream>
#include <sstream>

GLuint load_shaders(const char * vertex_file_path, const char * fragment_file_path){
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint create_array_buffer(const GLfloat* data, int len) {
    // The array buffer id
    GLuint id;
    // Generate 1 buffer, put the resulting identifier in id
    glGenBuffers(1, &id);
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, id);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, len, data, GL_STATIC_DRAW);

    // return the array buffer id
    return id;
}

void reuse_array_buffer(GLuint array_buffer_id, const GLfloat* data, int len) {
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_id);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    glBufferData(GL_ARRAY_BUFFER, len, data, GL_STATIC_DRAW);
}

void bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id) {
    if (texture_num < 0 || texture_num >= GL_MAX_TEXTURE_UNITS) {
        printf("BAD TEXTURE NUM: %d / %d", texture_num, GL_MAX_TEXTURE_UNITS);
    }

    glActiveTexture(GL_TEXTURE0 + texture_num);
    // gl_active_texture = texture_num;
    glBindTexture(GL_TEXTURE_2D, opengl_texture_id);
    // GL_TEXTURE_2D[gl_active_texture] = my_texture_id;
    glUniform1i(shader_texture_pointer, texture_num);
    // *shader_texture_pointer = GL_TEXTURE_2D[texture_num]
}

void bind_array(int array_num, GLuint array_buffer, GLint size) {
    glEnableVertexAttribArray(array_num); // Must match attribute
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
    glVertexAttribPointer(
        array_num,                        // attribute. Must match the layout in the shader.
        size,                             // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
}

// *******************
// Coordinates for Meshes
// *******************
static GLfloat g_cube_vertex_buffer_data[] = {
    0.0f, 0.0f, 0.0f, // Triangle 0
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 0.0f, // Triangle 1
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f, // Triangle 2
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, // Triangle 3
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 1.0f, // Triangle 4
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, // Triangle 5
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, // Triangle 6
    0.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, // Triangle 7
    1.0f, 1.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 
    1.0f, 1.0f, 0.0f, // Triangle 8
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, // Triangle 9
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f, // Triangle 10
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, // Triangle 11
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f
};

static GLfloat g_cube_uv_buffer_data[] = {
    0.000000f, 1.000000f-0.000000f, // Triangle 0
    0.000000f, 1.000000f-0.500000f,
    0.333333f, 1.000000f-0.500000f,
    0.000000f, 1.000000f-0.000000f, // Triangle 1
    0.333333f, 1.000000f-0.500000f,
    0.333333f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.500000f, // Triangle 2
    0.666666f, 1.000000f-0.000000f,
    0.333333f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.000000f, // Triangle 3
    0.333333f, 1.000000f-0.500000f,
    0.666666f, 1.000000f-0.500000f,
    0.666666f, 1.000000f-0.500000f, // Triangle 4
    0.333333f, 1.000000f-1.000000f,
    0.666666f, 1.000000f-1.000000f,
    0.666666f, 1.000000f-0.500000f, // Triangle 5
    0.333333f, 1.000000f-0.500000f,
    0.333333f, 1.000000f-1.000000f,
    0.000000f, 1.000000f-0.500000f, // Triangle 6
    0.333333f, 1.000000f-1.000000f,
    0.333333f, 1.000000f-0.500000f,
    0.000000f, 1.000000f-0.500000f, // Triangle 7
    0.000000f, 1.000000f-1.000000f,
    0.333333f, 1.000000f-1.000000f,
    1.000000f, 1.000000f-0.000000f, // Triangle 8
    0.666666f, 1.000000f-0.500000f,
    1.000000f, 1.000000f-0.500000f,
    1.000000f, 1.000000f-0.000000f, // Triangle 9
    0.666666f, 1.000000f-0.000000f,
    0.666666f, 1.000000f-0.500000f,
    1.000000f, 1.000000f-1.000000f, // Triangle 10
    1.000000f, 1.000000f-0.500000f,
    0.666666f, 1.000000f-0.500000f,
    0.666666f, 1.000000f-1.000000f, // Triangle 11
    1.000000f, 1.000000f-1.000000f,
    0.666666f, 1.000000f-0.500000f
};

static GLfloat g_plane_vertex_buffer_data[] = {
   -1.0f,-1.0f, 0.0f, // triangle 1 : begin
    1.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 1.0f, // triangle 1 : end
   -1.0f,-1.0f, 0.0f, // triangle 2 : begin
    1.0f,-1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, // triangle 2 : end
};

static GLfloat g_plane_uv_buffer_data[] = {
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

pair<GLfloat*, int> get_specific_cube_vertex_coordinates(bool f[6]) {
 
    // 0 => Yellow (-x)
    // 1 => Magenta (+x)
    // 2 => Green (-y)
    // 3 => Cyan (+y)
    // 4 => Blue (-z)
    // 5 => Red (+z)

    static GLfloat cube_buf[18*6];
    int len = 0;
    for(int i = 0; i < 6; i++) {
        if (f[i]) {
            // src will contain a pointer to the i'th face
            GLfloat* src = &g_cube_vertex_buffer_data[18*i];
            // dst will contain a pointer to the top of cube_buf (Past the already saved data)
            GLfloat* dst = &cube_buf[len];
            // Will copy the face into cube_buf
            memcpy(dst, src, 18*sizeof(GLfloat));
            len += 18;
        }
    }
    return {cube_buf, len*sizeof(GLfloat)};
}

pair<GLfloat*, int> get_specific_cube_uv_coordinates(bool f[6]) {
    static GLfloat cube_buf[12*6];
    int len = 0;
    for(int i = 0; i < 6; i++) {
        if (f[i]) {
            // src will contain a pointer to the i'th face
            GLfloat* src = &g_cube_uv_buffer_data[12*i];
            // dst will contain a pointer to the top of cube_buf (Past the already saved data)
            GLfloat* dst = &cube_buf[len];
            // Will copy the face into cube_buf
            memcpy(dst, src, 12*sizeof(GLfloat));
            len += 12;
        }
    }
    return {cube_buf, len*sizeof(GLfloat)};
}

pair<const GLfloat*, int> get_cube_vertex_coordinates() {
    return {g_cube_vertex_buffer_data, sizeof(g_cube_vertex_buffer_data)};
}

pair<const GLfloat*, int> get_cube_uv_coordinates() {
    return {g_cube_uv_buffer_data, sizeof(g_cube_uv_buffer_data)};
}
pair<const GLfloat*, int> get_plane_vertex_coordinates() {
    return {g_plane_vertex_buffer_data, sizeof(g_plane_vertex_buffer_data)};
}
pair<const GLfloat*, int> get_plane_uv_coordinates() {
    return {g_plane_uv_buffer_data, sizeof(g_plane_uv_buffer_data)};
}
