// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "slangi/hgvm-input.h"

#include "unit-test/slang-unit-test.h"

#ifdef _WIN32
#include "platform/windows/win-window-input.h"
#endif

using namespace Slang;
using namespace Slang::HGVM;

SLANG_UNIT_TEST(hgvmMouseButtonInput)
{
    platform::MouseEventArgs args = {-17, 23, 0, platform::ButtonState::LeftButton};
    HGVMInputEvent event;
    SLANG_CHECK(SlangITool::translateMouseButtonEvent(args, true, event));
    SLANG_CHECK(event.type == HGVMInputEventType::MouseButtonDown);
    args.isDoubleClick = true;
    SLANG_CHECK(SlangITool::translateMouseButtonEvent(args, true, event));
    SLANG_CHECK(event.type == HGVMInputEventType::MouseDoubleClick);
    SLANG_CHECK(uint32_t(event.type) == 13);
    SLANG_CHECK(event.button == 0);
    SLANG_CHECK(event.position[0] == -17.0f && event.position[1] == 23.0f);
    SLANG_CHECK(SlangITool::translateMouseButtonEvent(args, false, event));
    SLANG_CHECK(event.type == HGVMInputEventType::MouseButtonUp);
    args.buttons = platform::ButtonState::RightButton;
    SLANG_CHECK(!SlangITool::translateMouseButtonEvent(args, true, event));
    SLANG_CHECK(event.type == HGVMInputEventType::None);
}

#ifdef _WIN32
SLANG_UNIT_TEST(hgvmWin32DoubleClickInput)
{
    // Exercise the real window decoder and native-host conversion without creating an HWND.
    // Each DBLCLK must survive as exactly one second press, including during repeated clicking.
    const UINT messages[] = {
        WM_LBUTTONDOWN,
        WM_LBUTTONUP,
        WM_LBUTTONDBLCLK,
        WM_LBUTTONUP,
        WM_LBUTTONDOWN,
        WM_LBUTTONUP,
        WM_LBUTTONDBLCLK,
        WM_LBUTTONUP,
    };
    bool held = false;
    uint32_t releases = 0;
    uint32_t doubleClicks = 0;
    for (UINT message : messages)
    {
        platform::MouseEventArgs args;
        bool pressed;
        SLANG_CHECK_ABORT(
            platform::translateMouseButtonMessage(message, MAKELPARAM(-7, 19), args, pressed));
        HGVMInputEvent event;
        SLANG_CHECK_ABORT(SlangITool::translateMouseButtonEvent(args, pressed, event));
        SLANG_CHECK(event.position[0] == -7.0f && event.position[1] == 19.0f);
        if (event.type == HGVMInputEventType::MouseButtonUp)
        {
            SLANG_CHECK(held);
            held = false;
            releases++;
        }
        else
        {
            SLANG_CHECK(!held);
            held = true;
            if (event.type == HGVMInputEventType::MouseDoubleClick)
                doubleClicks++;
            else
                SLANG_CHECK(event.type == HGVMInputEventType::MouseButtonDown);
        }
    }
    SLANG_CHECK(!held && releases == 4 && doubleClicks == 2);

    platform::MouseEventArgs args;
    bool pressed;
    SLANG_CHECK(platform::translateMouseButtonMessage(WM_MBUTTONDBLCLK, 0, args, pressed));
    SLANG_CHECK(
        pressed && args.isDoubleClick && args.buttons == platform::ButtonState::MiddleButton);
    SLANG_CHECK(platform::translateMouseButtonMessage(WM_RBUTTONDBLCLK, 0, args, pressed));
    SLANG_CHECK(
        pressed && args.isDoubleClick && args.buttons == platform::ButtonState::RightButton);
    SLANG_CHECK(!platform::translateMouseButtonMessage(WM_MOUSEMOVE, 0, args, pressed));
}
#endif
