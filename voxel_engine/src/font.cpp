#include "font.hpp"
#include "gl_utils.hpp"

Font::Font(const char* font_path) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path, 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }    

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        this->characters[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    this->shader_id = load_shaders("assets/shaders/font.vert", "assets/shaders/font.frag");
}

int Font::get_width(const char* text) {
    int width = 0;

    int len = strlen(text);
    for(int i = 0; i < len; i++) {
        char c = text[i];
        Character ch = this->characters[c];
        width += (ch.Advance >> 6);
    }

    return width;
}

void Font::render(ivec2 dimensions, ivec2 location, float scale, const char* text, ivec3 color) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 	
    glUseProgram(this->shader_id);

    glm::mat4 projection = glm::ortho(0.0f, (float)dimensions.x, 0.0f, (float)dimensions.y);

    // Create VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    // Set vertex buffer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // activate corresponding render state
    glUniform3i(glGetUniformLocation(this->shader_id, "textColor"), color.x, color.y, color.z);

    GLuint projection_shader_pointer = glGetUniformLocation(this->shader_id, "projection");
    glUniformMatrix4fv(projection_shader_pointer, 1, GL_FALSE, &projection[0][0]);
    
    GLuint texture_pointer = glGetUniformLocation(this->shader_id, "textTexture");

    // iterate through all characters
    int len = strlen(text);
    for (int i = 0; i < len; i++)
    {
        char c = text[i];
        Character ch = this->characters[c];

        float xpos = location.x + ch.Bearing.x * scale;
        float ypos = location.y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        bind_texture(0, texture_pointer, ch.texture_id);
        // update content of VBO memory
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        location.x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glDeleteBuffers(1, &VBO);
    glDisable(GL_BLEND);
}
