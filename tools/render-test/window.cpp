// window.cpp

#include "window.h"

namespace renderer_test {
using namespace Slang;

#if SLANG_WINDOWS_FAMILY
extern Window* createWinWindow();
#endif

/* static */Window* Window::create()
{
#if SLANG_WINDOWS_FAMILY
    return createWinWindow();
#else
    return nullptr;
#endif
}

} // renderer_test
