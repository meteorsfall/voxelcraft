#include "gl_utils.hpp"
#include <fstream>
#include <sstream>

GLReference::GLReference() {
    this->opengl_id = nullopt;
}

GLReference::~GLReference() {
    if (this->opengl_id) {
        dbg("ERROR: Memory leak found! GLReference not freed yet!");
    }
}

// GLReference my_new_ref = my_old_ref;
GLReference::GLReference(const GLReference& other) // copy constructor
{
    if (other.opengl_id) {
        dbg("ERROR: Tried to copy-construct an allocated GLReference!");
        // Pass it along anyway
        this->opengl_id = other.opengl_id;
    }
}

// GLReference my_new_ref = std::move(my_old_ref);
GLReference::GLReference(GLReference&& other) noexcept {
    this->opengl_id = other.opengl_id;
    other.opengl_id = nullopt;
}

// my_old_ref = my_older_ref;
GLReference& GLReference::operator=(const GLReference& other) // copy assignment
{
    if(this != &other) {
        // Real copy-assign constructor
        if (other.opengl_id) {
            dbg("ERROR: Tried to copy-assign from an allocated GLReference!");
        }
        if (this->opengl_id) {
            dbg("ERROR: Tried to copy-assign to an allocated GLReference!");
        }
        // Pass it long anyway
        this->opengl_id = other.opengl_id;
    }
    return *this;
}

// my_old_ref = std::move(my_older_ref);
GLReference& GLReference::operator=(GLReference&& other) noexcept {
    if (this->opengl_id) {
        dbg("ERROR: Memory leak found! Move-assigned into an allocated GLReference!");
        // Pass it along anyway
    }
    this->opengl_id = other.opengl_id;
    other.opengl_id = nullopt;
    return *this;
}

GLArrayBuffer::GLArrayBuffer() {}
GLArrayBuffer::GLArrayBuffer(const GLfloat* data, int len) { init(data, len); }

GLArrayBuffer::~GLArrayBuffer()
{
    if (this->array_buffer_id.opengl_id) {
        glDeleteBuffers(1, &this->array_buffer_id.opengl_id.value());
        this->array_buffer_id.opengl_id = nullopt;
    }
}

void GLArrayBuffer::reuse(const GLfloat* data, int len) {
    if (array_buffer_id.opengl_id) {
        reuse_array_buffer(this->array_buffer_id.opengl_id.value(), data, len);
    } else {
        init(data, len);
    }
}

void GLArrayBuffer::bind(int array_num, GLint size) {
    bind_array(array_num, this->array_buffer_id.opengl_id.value(), size);
}

void GLArrayBuffer::init(const GLfloat* data, int len) {
    this->len = len;
    this->array_buffer_id.opengl_id = create_array_buffer(data, len);
}

GLuint GL::load_shaders(const char * vertex_file_path, const char * fragment_file_path){
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

GLuint GL::load_cubemap(byte* sides[6], int size)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    byte* img = new byte[4*size*size];
    for (uint i = 0; i < 6; i++)
    {
        // Flip image
        // https://stackoverflow.com/questions/11685608/convention-of-faces-in-opengl-cubemapping
        for(int y = 0; y < size; y++) {
            memcpy(&img[y*size*4], &sides[i][(size-1-y)*size*4], size*4);
        }

        // Save Texture
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                     0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, img
        );
    }
    delete[] img;

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  

GLuint GL::create_array_buffer(const GLfloat* data, int len) {
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

void GL::reuse_array_buffer(GLuint array_buffer_id, const GLfloat* data, int len) {
    // Make GL_ARRAY_BUFFER point to vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_id);
    // Give our vertices to GL_ARRAY_BUFFER (ie, vertexbuffer)
    //void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    //memcpy(ptr, data, len);
    //glUnmapBuffer(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, len, data, GL_STATIC_DRAW);
}

void GL::bind_texture(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id) {
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

void GL::bind_texture_cubemap(int texture_num, GLuint shader_texture_pointer, GLuint opengl_texture_id) {
    if (texture_num < 0 || texture_num >= GL_MAX_TEXTURE_UNITS) {
        printf("BAD TEXTURE NUM: %d / %d", texture_num, GL_MAX_TEXTURE_UNITS);
    }

    glActiveTexture(GL_TEXTURE0 + texture_num);
    // gl_active_texture = texture_num;
    glBindTexture(GL_TEXTURE_CUBE_MAP, opengl_texture_id);
    // GL_TEXTURE_2D[gl_active_texture] = my_texture_id;
    glUniform1i(shader_texture_pointer, texture_num);
    // *shader_texture_pointer = GL_TEXTURE_2D[texture_num]
}

void GL::bind_array(int array_num, GLuint array_buffer, GLint size) {
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
    0.0f, 0.0f, // Triangle 0 (-x)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f, // Triangle 1
    1.0f, 1.0f,
    0.0f, 1.0f, 
    1.0f, 0.0f, // Triangle 2 (+x)
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 1.0f, // Triangle 3
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f, // Triangle 4 (-y)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f, // Triangle 5
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 1.0f, // Triangle 6 (+y)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f, // Triangle 7
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f, // Triangle 8 (-z)
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f, // Triangle 9
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f, // Triangle 10 (+z)
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f, // Triangle 11
    0.0f, 1.0f,
    1.0f, 0.0f
};

static GLfloat g_plane_vertex_buffer_data[] = {
   -1.0f,-1.0f, 0.0f, // triangle 1 : begin
    1.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 0.0f, // triangle 1 : end
   -1.0f,-1.0f, 0.0f, // triangle 2 : begin
    1.0f,-1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, // triangle 2 : end
};

static GLfloat g_plane_uv_buffer_data[] = {
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
};

pair<GLfloat*, int> GL::get_specific_cube_vertex_coordinates(int bitmask) {
 
    // 0 => Yellow (-x)
    // 1 => Magenta (+x)
    // 2 => Green (-y)
    // 3 => Cyan (+y)
    // 4 => Blue (-z)
    // 5 => Red (+z)

    static map<int, pair<GLfloat*, int>> memo;

    if (!memo.count(bitmask)) {
        GLfloat* memo_cube_buf = new GLfloat[18*6];
        int len = 0;
        for(int i = 0; i < 6; i++) {
            if ((bitmask >> i) & 1) {
                // src will contain a pointer to the i'th face
                GLfloat* src = &g_cube_vertex_buffer_data[18*i];
                // dst will contain a pointer to the top of cube_buf (Past the already saved data)
                GLfloat* dst = &memo_cube_buf[len];
                // Will copy the face into cube_buf
                memcpy(dst, src, 18*sizeof(GLfloat));
                len += 18;
            }
        }
        memo[bitmask] = {memo_cube_buf, len*sizeof(GLfloat)};
    }

    auto [buf, len] = memo[bitmask];
    
    static GLfloat cube_buf[18*6];
    memcpy(cube_buf, buf, len);

    return {cube_buf, len};
}

pair<GLfloat*, int> GL::get_specific_cube_uv_coordinates(int bitmask) {

    static map<int, pair<GLfloat*, int>> memo;

    if (!memo.count(bitmask)) {
        GLfloat* memo_cube_buf = new GLfloat[18*6];
        int len = 0;
        for(int i = 0; i < 6; i++) {
            if ((bitmask >> i) & 1) {
                // src will contain a pointer to the i'th face
                GLfloat* src = &g_cube_uv_buffer_data[12*i];
                // dst will contain a pointer to the top of cube_buf (Past the already saved data)
                GLfloat* dst = &memo_cube_buf[len];
                // Will copy the face into cube_buf
                memcpy(dst, src, 12*sizeof(GLfloat));
                len += 12;
            }
        }
        memo[bitmask] = {memo_cube_buf, len*sizeof(GLfloat)};
    }

    auto [buf, len] = memo[bitmask];
    
    static GLfloat cube_buf[12*6];
    memcpy(cube_buf, buf, len);

    return {cube_buf, len};
}

pair<const GLfloat*, int> GL::get_cube_vertex_coordinates() {
    return {g_cube_vertex_buffer_data, sizeof(g_cube_vertex_buffer_data)};
}

pair<const GLfloat*, int> GL::get_cube_uv_coordinates() {
    return {g_cube_uv_buffer_data, sizeof(g_cube_uv_buffer_data)};
}
pair<const GLfloat*, int> GL::get_plane_vertex_coordinates() {
    return {g_plane_vertex_buffer_data, sizeof(g_plane_vertex_buffer_data)};
}
pair<const GLfloat*, int> GL::get_plane_uv_coordinates() {
    return {g_plane_uv_buffer_data, sizeof(g_plane_uv_buffer_data)};
}

float g_skybox_vertex_data[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

pair<const GLfloat*, int> GL::get_skybox_vertex_coordinates() {
    return {g_skybox_vertex_data, sizeof(g_skybox_vertex_data)};
}
