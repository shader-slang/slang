#pragma once

#include "slang-gfx.h"
#include "tools/platform/window.h"
#include "source/core/slang-basic.h"
#include "source/core/slang-io.h"

#ifdef _WIN32
void _Win32OutputDebugString(const char* str);
#endif

struct WindowedAppBase
{
protected:
    static const int kSwapchainImageCount = 2;

    Slang::RefPtr<platform::Window> gWindow;
    uint32_t windowWidth;
    uint32_t windowHeight;

    Slang::ComPtr<gfx::IDevice> gDevice;

    Slang::ComPtr<gfx::ISwapchain> gSwapchain;
    Slang::ComPtr<gfx::IFramebufferLayout> gFramebufferLayout;
    Slang::List<Slang::ComPtr<gfx::IFramebuffer>> gFramebuffers;
    Slang::List<Slang::ComPtr<gfx::ITransientResourceHeap>> gTransientHeaps;
    Slang::ComPtr<gfx::IRenderPassLayout> gRenderPass;
    Slang::ComPtr<gfx::ICommandQueue> gQueue;

    Slang::Result initializeBase(
        const char* title,
        int width,
        int height,
        gfx::DeviceType deviceType = gfx::DeviceType::Default);
    void createSwapchainFramebuffers();
    void mainLoop();
    Slang::ComPtr<gfx::IResourceView> createTextureFromFile(Slang::String fileName, int& textureWidth, int& textureHeight);
    virtual void windowSizeChanged();

protected:
    virtual void renderFrame(int framebufferIndex) = 0;
public:
    platform::Window* getWindow() { return gWindow.Ptr(); }
    virtual void finalize() { gQueue->waitOnHost(); }
};

struct ExampleResources {
    Slang::String baseDir;

    ExampleResources(const Slang::String &dir) : baseDir(dir) {}
    
    Slang::String resolveResource(const char* fileName) const {
        static const Slang::List<Slang::String> directories {
            "examples",
            "../examples",
            "../../examples",
        };
    
        for (const Slang::String& dir : directories) {      
            Slang::StringBuilder pathSb;
            pathSb << dir  << "/" << baseDir << "/" << fileName;
            if (Slang::File::exists(pathSb.getBuffer()))
                return pathSb.toString();
        }

        return fileName;
    }
};

int64_t getCurrentTime();
int64_t getTimerFrequency();

template<typename ... TArgs> inline void reportError(const char* format, TArgs... args)
{
    printf(format, args...);
#ifdef _WIN32
    char buffer[4096];
    sprintf_s(buffer, format, args...);
    _Win32OutputDebugString(buffer);
#endif
}

template <typename... TArgs> inline void log(const char* format, TArgs... args)
{
    reportError(format, args...);
}

// Many Slang API functions return detailed diagnostic information
// (error messages, warnings, etc.) as a "blob" of data, or return
// a null blob pointer instead if there were no issues.
//
// For convenience, we define a subroutine that will dump the information
// in a diagnostic blob if one is produced, and skip it otherwise.
//
inline void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        reportError("%s", (const char*)diagnosticsBlob->getBufferPointer());
    }
}

void initDebugCallback();

template<typename TApp>
int innerMain()
{
    initDebugCallback();

    TApp app;

    if (SLANG_FAILED(app.initialize()))
    {
        return -1;
    }

    platform::Application::run(app.getWindow());

    app.finalize();
    return 0;
}
