// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.window.simple_window;

enum InputEventType : uint
{
    none = 0,
    keyPress = 1,
    keyRelease = 2,
    mouseButtonDown = 3,
    mouseButtonUp = 4,
    mouseMove = 5,
    mouseScroll = 6,
    windowResize = 7,
    mouseEnter = 8,
    mouseLeave = 9,
    windowFocusGained = 10,
    windowFocusLost = 11,
    mouseCaptureLost = 12,
    // Replaces mouseButtonDown for the second press; mouseButtonUp still follows it.
    mouseDoubleClick = 13,
}

// Both events begin a press. Consumers can still distinguish double-clicks using the event type.
bool isMouseButtonPress(InputEventType type)
{
    return type == InputEventType.mouseButtonDown || type == InputEventType.mouseDoubleClick;
}

enum KeyCode : uint
{
    unknown = 0,

    // Printable keys use their ASCII values.
    space = 32,
    quote = 39,
    comma = 44,
    minus = 45,
    period = 46,
    slash = 47,
    key0 = 48,
    key1 = 49,
    key2 = 50,
    key3 = 51,
    key4 = 52,
    key5 = 53,
    key6 = 54,
    key7 = 55,
    key8 = 56,
    key9 = 57,
    semicolon = 59,
    plus = 61,
    a = 65,
    b = 66,
    c = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,
    leftBracket = 91,
    backslash = 92,
    rightBracket = 93,
    tilde = 96,

    // Non-printable keys use GLFW-compatible values.
    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,
    insert = 260,
    delete = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    pageUp = 266,
    pageDown = 267,
    home = 268,
    end = 269,
    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,

    // The platform abstraction reports modifiers without distinguishing left from right.
    shift = 340,
    ctrl = 341,
    alt = 342,
}

enum MouseButton : uint
{
    left = 0,
    unknown = 3,
}

struct InputEvent
{
    InputEventType type;
    KeyCode key;
    MouseButton button;
    // Client coordinates for mouse enter/move/button events; may be negative during capture.
    // Mouse leave invalidates the last position. Scroll/focus events do not carry a position.
    float2 pos;
    float2 scroll;
    // Latest client-area extent for windowResize; zero while minimized.
    uint2 size;
}

// Returns the oldest pending supported event, or InputEventType.none when the queue is empty.
InputEvent pollInputEvent(Window window)
{
    return hgbc_asm
    {
    result:
        $$InputEvent = OpPollInputEvent $window;
    };
}

// Requests closure while keeping the window alive until its graph values are released.
void closeWindow(Window window)
{
    hgbc_asm
    {
        OpCloseWindow $window;
    };
}
