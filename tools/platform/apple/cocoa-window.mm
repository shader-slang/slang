#ifdef __APPLE__

#include "../window.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

using namespace Slang;

static bool g_should_close = false;

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (BOOL)windowShouldClose:(id)window
{
    g_should_close = true;
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    g_should_close = true;
    return NSTerminateCancel;
}

@end

@interface ContentView : NSView
@end

@implementation ContentView

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent *)event
{
    if ([event keyCode] == 53)
    {
        g_should_close = true;
    }
}

@end

namespace platform
{

static NSApplication *_application;


void Application::init()
{
    _application = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
}

void Application::doEvents() {
    NSEvent *event;
    do {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                   untilDate:[NSDate distantPast]
                                      inMode:NSDefaultRunLoopMode
                                     dequeue:YES];
        if (event) {
            [NSApp sendEvent:event];
        }
    } while (event);    
}

void Application::quit() { }

void Application::dispose()
{
}

void Application::run(Window* mainWindow, bool waitForEvents)
{
    do {
        NSEvent *event;
        do {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                    untilDate:[NSDate distantPast]
                                        inMode:NSDefaultRunLoopMode
                                        dequeue:YES];
            if (event) {
                [NSApp sendEvent:event];
            }
        } while (event);    
        if (mainWindow)
        {
            mainWindow->events.mainLoop();
        }
    } while (!g_should_close);

    // doEvents();
    // if (mainWindow)
    // {
    //     Win32AppContext::mainWindow = mainWindow;
    //     Win32AppContext::mainWindowHandle = (HWND)mainWindow->getNativeHandle().handleValues[0];
    //     ShowWindow(Win32AppContext::mainWindowHandle, SW_SHOW);
    //     UpdateWindow(Win32AppContext::mainWindowHandle);
    // }
    // while (!Win32AppContext::isTerminated)
    // {
    //     doEventsImpl(waitForEvents);
    //     if (Win32AppContext::isTerminated)
    //         break;
    //     if (mainWindow)
    //     {
    //         mainWindow->events.mainLoop();
    //     }
    // }
}

class ApplePlatformWindow : public Window
{
public:
    NSWindow* window;
    ContentView* view;
    CAMetalLayer* layer;

    ApplePlatformWindow(const WindowDesc& desc)
    {
        // Create a reference rectangle
        NSRect rect = NSMakeRect(0.0f, 0.0f, desc.width, desc.height);

        // Allocate window
        window = [[NSWindow alloc] initWithContentRect:rect
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                backing:NSBackingStoreBuffered
                                                   defer:NO];

        const NSWindowCollectionBehavior behavior
            = NSWindowCollectionBehaviorFullScreenPrimary | NSWindowCollectionBehaviorManaged;
        [window setCollectionBehavior:behavior];

        if (desc.style == WindowStyle::Default)
        {
            [window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable];
        }
        else if (desc.style == WindowStyle::FixedSize)
        {
            [window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable];
        }

        // Allocate view
        rect = [window backingAlignedRect:rect options:NSAlignAllEdgesOutward];
        view = [[ContentView alloc] initWithFrame:rect];
        [view setHidden:NO];
        [view setNeedsDisplay:YES];
        [view setWantsLayer:YES];

        // [window setDelegate:[WindowDelegate alloc]];
        [window setDelegate:(id<NSWindowDelegate>)[NSApp delegate]];
        [window setContentView:view];

        NSString* title = [NSString stringWithUTF8String:desc.title];
        [window setTitle:title];

        [window center];
        [window makeKeyAndOrderFront:nil];

        // Setup layer
        layer = [[CAMetalLayer alloc] init];
        [view setLayer:layer];        
    }

    ~ApplePlatformWindow() { close(); }

    virtual void setClientSize(uint32_t width, uint32_t height) override
    {
        NSSize size = NSMakeSize(width, height);
        [window setContentSize:size];
    }

    virtual Rect getClientRect() override
    {
        NSRect rect = [window contentRectForFrameRect:[window frame]];
        return { (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height };
    }

    virtual void centerScreen() override
    {
        [window center];
    }

    virtual void close() override
    {
        [window release];
        [view release];
        [layer release];

        window = nullptr;
        view = nullptr;
        layer = nullptr;
    }

    virtual bool getFocused() override { return [window isKeyWindow]; }
    virtual bool getVisible() override { return [window isVisible]; }

    virtual WindowHandle getNativeHandle() override
    {
        return WindowHandle::fromNSView(view);
    }

    virtual void setText(Slang::String text) override
    {
        NSString* title = [NSString stringWithUTF8String:text.begin()];
        [window setTitle:title];
    }

    virtual void show() override
    {
        [window setIsVisible:YES];
    }

    virtual void hide() override
    {
        [window setIsVisible:NO];
    }

    virtual int getCurrentDpi() override
    {
        // There seems to be no API to get the actual DPI of the screen.
        return 0;
    }
};

Window* Application::createWindow(const WindowDesc& desc) { return new ApplePlatformWindow(desc); }


} // namespace platform

#endif // __APPLE__
