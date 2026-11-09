// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "platform/window.h"

#include <windows.h>
#include <windowsx.h>

namespace platform
{

/// Decodes a button transition without querying a window or the current physical button state.
/// Windows replaces the second DOWN with DBLCLK, so both messages describe a press.
inline bool translateMouseButtonMessage(
    UINT message,
    LPARAM position,
    MouseEventArgs& outEvent,
    bool& outPressed)
{
    outEvent = {};
    outPressed = false;
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        outEvent.buttons = ButtonState::LeftButton;
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
        outEvent.buttons = ButtonState::MiddleButton;
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
        outEvent.buttons = ButtonState::RightButton;
        break;
    default:
        return false;
    }
    outEvent.x = GET_X_LPARAM(position);
    outEvent.y = GET_Y_LPARAM(position);
    outEvent.isDoubleClick =
        message == WM_LBUTTONDBLCLK || message == WM_MBUTTONDBLCLK || message == WM_RBUTTONDBLCLK;
    outPressed = message != WM_LBUTTONUP && message != WM_MBUTTONUP && message != WM_RBUTTONUP;
    return true;
}

} // namespace platform
