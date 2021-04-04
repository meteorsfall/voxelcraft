#include "html_renderer.hpp"

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
using namespace ultralight;

struct PrivateData {
    RefPtr<Renderer> renderer;
    RefPtr<View> view;
};

#define PRIVATE ((PrivateData*)(this->private_data))

void HTMLRenderer::init(GLFWwindow* window) {
    Config config;

    // We need to tell config where our resources are so it can 
    // load our bundled SSL certificates to make HTTPS requests.
    config.resource_path = "/home/npip99/downloads/ultralight/bin/resources";

    // The GPU renderer should be disabled to render Views to a 
    // pixel-buffer (Surface).
    config.use_gpu_renderer = false;

    // You can set a custom DPI scale here. Default is 1.0 (100%)
    config.device_scale = 1.0;

    // Pass our configuration to the Platform singleton so that
    // the library can use it.
    Platform::instance().set_config(config);

    // Use the OS's native font loader
    Platform::instance().set_font_loader(GetPlatformFontLoader());

    // Use the OS's native file loader, with a base directory of "."
    // All file:/// URLs will load relative to this base directory.
    Platform::instance().set_file_system(GetPlatformFileSystem("."));

    // Use the default logger (writes to a log file)
    Platform::instance().set_logger(GetDefaultLogger("ultralight.log"));

    // Initialize private data
    PRIVATE = new PrivateData();

    // Create a View
    PRIVATE->renderer = Renderer::Create();
    PRIVATE->view = PRIVATE->renderer->CreateView(500, 500, true, nullptr);
    PRIVATE->view->LoadHTML("<h1>Hello World!</h1>");
    PRIVATE->view->Focus();
}

bool HTMLRenderer::load_html(const WCHAR* path) {
    return true;
}

void HTMLRenderer::render(int width, int height) {
    // Resize the view to the current render dimensions
    PRIVATE->view->Resize(width, height);
    // Give the library a chance to handle any pending tasks and timers.
    PRIVATE->renderer->Update();

    // Get the pixel-buffer Surface for a View.
    Surface* surface = PRIVATE->view->surface();

    // Cast it to a BitmapSurface.
    BitmapSurface* bitmap_surface = (BitmapSurface*)surface;

    // Get the underlying bitmap.
    RefPtr<Bitmap> bitmap = bitmap_surface->bitmap();

    // Lock the Bitmap to retrieve the raw pixels.
    // The format is BGRA, 8-bpp, premultiplied alpha.
    void* pixels = bitmap->LockPixels();

    // Get the bitmap dimensions.
    uint32_t width = bitmap->width();
    uint32_t height = bitmap->height();
    uint32_t stride = bitmap->row_bytes();

    // Psuedo-code to upload our pixels to a GPU texture.
    CopyPixelsToTexture(pixels, width, height, stride);

    // Unlock the Bitmap when we are done.
    bitmap->UnlockPixels();
}
