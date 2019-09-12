// window.h
#pragma once

#include <slang.h>
#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {

class Window;

class WindowListener : public Slang::RefObject
{
public:
    virtual SlangResult update(Window* window) = 0;
};

class Window : public Slang::RefObject
{
public:
    virtual SlangResult initialize(int width, int height) = 0;

    virtual void show() = 0;
    virtual void* getHandle() const = 0;
    virtual void postQuit() { m_isQuitting = true; }
        
    /// Run the event loop. Events will be sent to the WindowListener
    virtual SlangResult runLoop(WindowListener* listener) = 0;

    bool isQuitting() const { return m_isQuitting; }
    int getQuitValue() const { return m_quitValue; }

    static Window* create();

    virtual ~Window() {}

protected:
    Window() {}

    bool m_isQuitting = false;
    int m_quitValue = 0;
};

Window* createWindow();

} // renderer_test
