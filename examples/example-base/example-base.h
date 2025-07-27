#pragma once

#include "core/slang-basic.h"
#include "core/slang-io.h"
#include "platform/window.h"
#include "slang-rhi.h"
#include "test-base.h"

#ifdef _WIN32
void _Win32OutputDebugString(const char* str);
#endif

#define SLANG_STRINGIFY(x) #x
#define SLANG_EXPAND_STRINGIFY(x) SLANG_STRINGIFY(x)

#ifdef _WIN32
#define EXAMPLE_MAIN(innerMain)                                   \
    extern const char* const g_logFileName =                      \
        "log-" SLANG_EXPAND_STRINGIFY(SLANG_EXAMPLE_NAME) ".txt"; \
    PLATFORM_UI_MAIN(innerMain);

#else
#define EXAMPLE_MAIN(innerMain) PLATFORM_UI_MAIN(innerMain)
#endif // _WIN32

static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

struct WindowedAppBase : public TestBase
{
protected:
    static const int kSwapchainImageCount = 2;

    Slang::RefPtr<platform::Window> gWindow;
    uint32_t windowWidth;
    uint32_t windowHeight;

    Slang::ComPtr<rhi::IDevice> gDevice;
    Slang::ComPtr<rhi::ICommandQueue> gQueue;
    Slang::ComPtr<rhi::ISurface> gSurface;

    Slang::List<Slang::ComPtr<rhi::ITexture>> gOfflineTextures;

    Slang::Result initializeBase(
        const char* title,
        int width,
        int height,
        rhi::DeviceType deviceType = rhi::DeviceType::Default);

    void mainLoop();

    Slang::ComPtr<rhi::ITextureView> createTextureFromFile(
        Slang::String fileName,
        int& textureWidth,
        int& textureHeight);

    void createOfflineTextures();

    virtual void windowSizeChanged();

protected:
    virtual void renderFrame(rhi::ITexture* texture) = 0;

public:
    platform::Window* getWindow() { return gWindow.Ptr(); }
    virtual void finalize() { gQueue->waitOnHost(); }
    void offlineRender();
};

struct ExampleResources
{
    Slang::String baseDir;

    ExampleResources(const Slang::String& dir)
        : baseDir(dir)
    {
    }

    Slang::String resolveResource(const char* fileName) const
    {
        static const Slang::List<Slang::String> directories{
            "examples",
            "../examples",
            "../../examples",
        };

        for (const Slang::String& dir : directories)
        {
            Slang::StringBuilder pathSb;
            pathSb << dir << "/" << baseDir << "/" << fileName;
            if (Slang::File::exists(pathSb.getBuffer()))
                return pathSb.toString();
        }

        return fileName;
    }
};

int64_t getCurrentTime();
int64_t getTimerFrequency();

template<typename... TArgs>
inline void reportError(const char* format, TArgs... args)
{
    printf(format, std::forward<TArgs>(args)...);
#ifdef _WIN32
    char buffer[4096];
    sprintf_s(buffer, format, std::forward<TArgs>(args)...);
    _Win32OutputDebugString(buffer);
#endif
}

template<typename... TArgs>
inline void log(const char* format, TArgs... args)
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

template<typename TApp>
int innerMain(int argc, char** argv)
{
    TApp app;

    app.parseOption(argc, argv);

    if (app.shouldShowHelp())
    {
        app.printUsage(argc > 0 ? argv[0] : "example");
        return 0;
    }

    if (SLANG_FAILED(app.initialize()))
    {
        return -1;
    }

    if (!app.isTestMode())
    {
        platform::Application::run(app.getWindow());
    }
    else
    {
        app.offlineRender();
    }

    app.finalize();
    return 0;
}
