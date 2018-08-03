// window.cpp
#include "window.h"
#pragma once

#include <stdio.h>

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif

#include <stdint.h>


#if _WIN32
#include <Windows.h>
#else
#error "The slang-graphics library currently only supports Windows platforms"
#endif

namespace gfx {

#if _WIN32

struct OSString
{
    OSString(char const* begin, char const* end)
    {
        _initialize(begin, end - begin);
    }

    OSString(char const* begin)
    {
        _initialize(begin, strlen(begin));
    }

    ~OSString()
    {
        free(mBegin);
    }

    operator WCHAR const*()
    {
        return mBegin;
    }

private:
    WCHAR* mBegin;
    WCHAR* mEnd;

    void _initialize(char const* input, size_t inputSize)
    {
        const DWORD dwFlags = 0;
        int outputCodeUnitCount = ::MultiByteToWideChar(CP_UTF8, dwFlags, input, int(inputSize), nullptr, 0);

        WCHAR* buffer = (WCHAR*)malloc(sizeof(WCHAR) * (outputCodeUnitCount + 1));

        ::MultiByteToWideChar(CP_UTF8, dwFlags, input, int(inputSize), buffer, outputCodeUnitCount);
        buffer[outputCodeUnitCount] = 0;

        mBegin = buffer;
        mEnd = buffer + outputCodeUnitCount;
    }
};

struct ApplicationContext
{
    HINSTANCE instance;
    int showCommand = SW_SHOWDEFAULT;
    int resultCode = 0;
};

static uint64_t gTimerFrequency;


static void initApplication(ApplicationContext* context)
{
    LARGE_INTEGER timerFrequency;
    QueryPerformanceFrequency(&timerFrequency);
    gTimerFrequency = timerFrequency.QuadPart;
}

/// Run an application given the specified callback and command-line arguments.
int runApplication(
    ApplicationFunc     func,
    int                 argc,
    char const* const*  argv)
{
    ApplicationContext context;
    context.instance = (HINSTANCE) GetModuleHandle(0);
    initApplication(&context);
    func(&context);
    return context.resultCode;
}

int runWindowsApplication(
    ApplicationFunc     func,
    void*               instance,
    int                 showCommand)
{
    ApplicationContext context;
    context.instance = (HINSTANCE) instance;
    context.showCommand = showCommand;
    initApplication(&context);
    func(&context);
    return context.resultCode;
}

struct Window
{
    HWND handle;
    WNDPROC nativeHook;
};

void setNativeWindowHook(Window* window, WNDPROC proc)
{
    window->nativeHook = proc;
}

static LRESULT CALLBACK windowProc(
    HWND    windowHandle,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    Window* window = (Window*) GetWindowLongPtrW(windowHandle, GWLP_USERDATA);

    // Give the installed filter a chance to intercept messages.
    // (This is used for ImGui)
    if( window )
    {
        if(auto nativeHook = window->nativeHook)
        {
            auto result = nativeHook(windowHandle, message, wParam, lParam);
            if(result)
                return result;
        }
    }

    // TODO: Actually implement some reasonable logic here.
    switch (message)
    {
    case WM_CREATE:
        {
            auto createInfo = (CREATESTRUCTW*) lParam;
            window = (Window*) createInfo->lpCreateParams;
            window->handle = windowHandle;

            SetWindowLongPtrW(windowHandle, GWLP_USERDATA, (LONG)window);
        }
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }


    return DefWindowProcW(windowHandle, message, wParam, lParam);
}


static ATOM createWindowClassAtom()
{
    WNDCLASSEXW windowClassDesc;
    windowClassDesc.cbSize = sizeof(windowClassDesc);
    windowClassDesc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClassDesc.lpfnWndProc = &windowProc;
    windowClassDesc.cbClsExtra = 0;
    windowClassDesc.cbWndExtra = 0;
    windowClassDesc.hInstance = (HINSTANCE) GetModuleHandle(0);
    windowClassDesc.hIcon = 0;
    windowClassDesc.hCursor = 0;
    windowClassDesc.hbrBackground = 0;
    windowClassDesc.lpszMenuName = 0;
    windowClassDesc.lpszClassName = L"SlangGraphicsWindow";
    windowClassDesc.hIconSm = 0;
    ATOM windowClassAtom = RegisterClassExW(&windowClassDesc);
    return windowClassAtom;
}

static ATOM getWindowClassAtom()
{
    static ATOM windowClassAtom = createWindowClassAtom();
    return windowClassAtom;
}

Window* createWindow(WindowDesc const& desc)
{
    Window* window = new Window();
    window->handle = nullptr;
    window->nativeHook = nullptr;

    OSString windowTitle(desc.title);

    DWORD windowExtendedStyle = 0;
    DWORD windowStyle = 0;

    HINSTANCE instance = (HINSTANCE) GetModuleHandle(0);

    HWND windowHandle = CreateWindowExW(
        windowExtendedStyle,
        (LPWSTR) getWindowClassAtom(),
        windowTitle,
        windowStyle,
        0, 0, // x, y
        desc.width, desc.height,
        NULL, // parent
        NULL, // menu
        instance,
        window);

    if(!windowHandle)
    {
        delete window;
        return nullptr;
    }

    window->handle = windowHandle;
    return window;
}

void showWindow(Window* window)
{
    ShowWindow(window->handle, SW_SHOW);
}

void* getPlatformWindowHandle(Window* window)
{
    return window->handle;
}

bool dispatchEvents(ApplicationContext* context)
{
    for(;;)
    {
        MSG message;

        int result = PeekMessageW(&message, NULL, 0, 0, PM_REMOVE);
        if (result != 0)
        {
            if (message.message == WM_QUIT)
            {
                context->resultCode = (int)message.wParam;
                return false;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        else
        {
            return true;
        }
    }

}

void exitApplication(ApplicationContext* context, int resultCode)
{
    ExitProcess(resultCode);
}

void log(char const* message, ...)
{
    va_list args;
    va_start(args, message);

    static const int kBufferSize = 1024;
    char messageBuffer[kBufferSize];
    vsnprintf(messageBuffer, kBufferSize - 1, message, args);
    messageBuffer[kBufferSize - 1] = 0;

    va_end(args);

    fputs(messageBuffer, stderr);

    OSString wideMessageBuffer(messageBuffer);
    OutputDebugStringW(wideMessageBuffer);
}

int reportError(char const* message, ...)
{
    va_list args;
    va_start(args, message);

    static const int kBufferSize = 1024;
    char messageBuffer[kBufferSize];
    vsnprintf(messageBuffer, kBufferSize - 1, message, args);
    messageBuffer[kBufferSize - 1] = 0;

    va_end(args);

    fputs(messageBuffer, stderr);

    OSString wideMessageBuffer(messageBuffer);
    OutputDebugStringW(wideMessageBuffer);

    return 1;
}

uint64_t getCurrentTime()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

uint64_t getTimerFrequency()
{
    return gTimerFrequency;
}

#else

// TODO: put an SDL version here

#endif

} // gfx
