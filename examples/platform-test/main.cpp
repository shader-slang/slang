#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "platform/window.h"
#include "slang-com-ptr.h"
#include "slang-rhi.h"
#include "slang.h"

#include <cstdio>

using namespace rhi;
using namespace Slang;

struct PlatformTest : public WindowedAppBase
{

    void onSizeChanged()
    {
        printf("onSizeChanged\n");
        fflush(stdout);
    }

    void onFocus()
    {
        printf("onFocus\n");
        fflush(stdout);
    }

    void onLostFocus()
    {
        printf("onLostFocus\n");
        fflush(stdout);
    }

    void onKeyDown(platform::KeyEventArgs args)
    {
        printf("onKeyDown(key=0x%02x, buttons=0x%02x)\n", (uint32_t)args.key, args.buttons);
        fflush(stdout);
    }

    void onKeyUp(platform::KeyEventArgs args)
    {
        printf("onKeyUp(key=0x%02x, buttons=0x%02x)\n", (uint32_t)args.key, args.buttons);
        fflush(stdout);
    }

    void onKeyPress(platform::KeyEventArgs args)
    {
        printf("onKeyPress(keyChar=0x%02x)\n", args.keyChar);
        fflush(stdout);
    }

    void onMouseMove(platform::MouseEventArgs args)
    {
        // Throttle mouse move events using a simple counter
        static int mouseMoveCounter = 0;
        mouseMoveCounter++;
        if (mouseMoveCounter % 50 == 0) // Only print every 50th mouse move event
        {
            printf(
                "onMouseMove(x=%d, y=%d, delta=%d, buttons=0x%02x)\n",
                args.x,
                args.y,
                args.delta,
                args.buttons);
            fflush(stdout);
        }
    }

    void onMouseDown(platform::MouseEventArgs args)
    {
        printf(
            "onMouseDown(x=%d, y=%d, delta=%d, buttons=0x%02x)\n",
            args.x,
            args.y,
            args.delta,
            args.buttons);
        fflush(stdout);
    }

    void onMouseUp(platform::MouseEventArgs args)
    {
        printf(
            "onMouseUp(x=%d, y=%d, delta=%d, buttons=0x%02x)\n",
            args.x,
            args.y,
            args.delta,
            args.buttons);
        fflush(stdout);
    }

    void onMouseWheel(platform::MouseEventArgs args)
    {
        printf(
            "onMouseWheel(x=%d, y=%d, delta=%d, buttons=0x%02x\n",
            args.x,
            args.y,
            args.delta,
            args.buttons);
        fflush(stdout);
    }

    Slang::Result initialize()
    {
        SLANG_RETURN_ON_FAIL(initializeBase("platform-test", 1024, 768, getDeviceType()));

        // We may not have a window if we're running in test mode
        SLANG_ASSERT(isTestMode() || gWindow);
        if (gWindow)
        {
            printf("Setting up event handlers...\n");
            fflush(stdout);

            gWindow->events.sizeChanged = [this]() { onSizeChanged(); };
            gWindow->events.focus = [this]() { onFocus(); };
            gWindow->events.lostFocus = [this]() { onLostFocus(); };
            gWindow->events.keyDown = [this](const platform::KeyEventArgs& e) { onKeyDown(e); };
            gWindow->events.keyUp = [this](const platform::KeyEventArgs& e) { onKeyUp(e); };
            gWindow->events.keyPress = [this](const platform::KeyEventArgs& e) { onKeyPress(e); };
            gWindow->events.mouseMove = [this](const platform::MouseEventArgs& e)
            { onMouseMove(e); };
            gWindow->events.mouseDown = [this](const platform::MouseEventArgs& e)
            { onMouseDown(e); };
            gWindow->events.mouseUp = [this](const platform::MouseEventArgs& e) { onMouseUp(e); };
            gWindow->events.mouseWheel = [this](const platform::MouseEventArgs& e)
            { onMouseWheel(e); };
            printf("Event handlers set up successfully.\n");
        }
        else
        {
            printf("No window available for event setup.\n");
            fflush(stdout);
        }

        return SLANG_OK;
    }

    virtual void renderFrame(ITexture* texture) override
    {
        auto commandEncoder = gQueue->createCommandEncoder();

        ComPtr<ITextureView> textureView = gDevice->createTextureView(texture, {});
        RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = LoadOp::Clear;

        RenderPassDesc renderPass = {};
        renderPass.colorAttachments = &colorAttachment;
        renderPass.colorAttachmentCount = 1;

        auto renderEncoder = commandEncoder->beginRenderPass(renderPass);

        RenderState renderState = {};
        renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
        renderState.viewportCount = 1;
        renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
        renderState.scissorRectCount = 1;

        renderEncoder->setRenderState(renderState);

        renderEncoder->end();
        gQueue->submit(commandEncoder->finish());

        if (!isTestMode())
        {
            gSurface->present();
        }
    }
};

// This macro instantiates an appropriate main function to
// run the application defined above.
EXAMPLE_MAIN(innerMain<PlatformTest>);
