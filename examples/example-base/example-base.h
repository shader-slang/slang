#pragma once

#include "slang-gfx.h"
#include "tools/platform/window.h"
#include "source/core/slang-basic.h"

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

    Slang::Result initializeBase(const char* titile, int width, int height);
    void createSwapchainFramebuffers();
    void mainLoop();

    virtual void windowSizeChanged();

protected:
    virtual void renderFrame(int framebufferIndex) = 0;
public:
    platform::Window* getWindow() { return gWindow.Ptr(); }
    virtual void finalize() { gQueue->wait(); }
};

template<typename TApp>
int innerMain()
{
    TApp app;

    if (SLANG_FAILED(app.initialize()))
    {
        return -1;
    }

    platform::Application::run(app.getWindow());

    app.finalize();
    return 0;
}
