#include "html_renderer.hpp"

static UINT SC_CALLBACK sciter_handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam);

void HTMLRenderer::init(GLFWwindow* window) {
    this->window = window;
    // Initializer Sciter
    SciterSetOption(NULL, SCITER_SET_UX_THEMING, TRUE); // Removes system-dependent CSS
    SciterProcX(window, SCITER_X_MSG_CREATE(GFX_LAYER_SKIA_OPENGL, FALSE));
    // Set Sciter DPI
    float xscale;
    glfwGetWindowContentScale(window, &xscale, NULL);
    SciterProcX(window, SCITER_X_MSG_RESOLUTION(96*xscale));
    // Set Sciter callback
    SciterSetCallback(window, sciter_handle_notification, nullptr);
}

bool HTMLRenderer::load_html(const WCHAR* path) {
    // Load HTML
    if (SciterLoadFile(window, path) == TRUE) {
        return true;
    } else {
        return false;
    }
}

void HTMLRenderer::render(int width, int height) {
    if (width != last_width || height != last_height) {
        // Update Sciter width/height
        SciterProcX(window, SCITER_X_MSG_SIZE(width, height));
        last_width = width;
        last_height = height;
    }
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

void HTMLRenderer::destroy() {
    // Close Sciter
    SciterProcX(window, SCITER_X_MSG_DESTROY());
    window = NULL;
}

// ================
// Sciter Functions
// ================

// See sciter-sdk/demos.lite/sciter-glfw-opengl/basic.cpp for examples

UINT on_load_data(LPSCN_LOAD_DATA pnmld) {
    aux::wchars chars = aux::chars_of(pnmld->uri);
    char* str = new char[chars.length];
    int len = 0;
    for(wchar_t c : chars) {
        str[len++] = c;
    }
    str[len] = '\0';
    std::string s(str);
    delete[] str;

    std::string prefix = "file://";
    if (s.rfind(prefix, 0) == 0) {
        s = s.substr(prefix.size());
    } else {
        dbg("File %s did not start with file://", s.c_str());
        return LOAD_DISCARD;
    }

    dbg("URI Request: %s", s.c_str());

    std::string filepath = std::string("assets/") + s;
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        dbg("Could not open file %s", filepath.c_str());
        return LOAD_DISCARD;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size))
    {
        /* worked! */
        ::SciterDataReady(pnmld->hwnd, pnmld->uri, (LPCBYTE)buffer.data(), size);
        return LOAD_OK;
    } else {
        dbg("Could not open file %s", filepath.c_str());
        return LOAD_DISCARD;
    }
}

UINT SC_CALLBACK sciter_handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam)
{
    UNUSED(pnm);
    UNUSED(callbackParam);
    
    switch (pnm->code) {
        case SC_LOAD_DATA:
            // Load image
            return on_load_data((LPSCN_LOAD_DATA)pnm);
        case SC_DATA_LOADED:
            dbg("Sciter Data Loaded");
            return 0; // Unimplemented
        case SC_ATTACH_BEHAVIOR:
            dbg("Sciter Attach");
            return 0; // Unimplemented
        case SC_INVALIDATE_RECT:
            // We draw on every frame, so the rectangle being invalidated isn't relevant to us
            return 0;
        case SC_ENGINE_DESTROYED:
            break;
        default:
            dbg("Unknown Sciter Notification: %d", pnm->code);
            break;
    }
    return 0;
}
