// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "platform/window.h"
#include "slang-hgvm/slang-hgvm.h"

namespace SlangITool
{

/// Converts supported platform button transitions to the HGVM input protocol.
/// Preserve the native double-click as one press event, followed by an ordinary release.
inline bool translateMouseButtonEvent(
    const platform::MouseEventArgs& args,
    bool pressed,
    Slang::HGVM::HGVMInputEvent& outEvent)
{
    outEvent = {};
    if (!(args.buttons & platform::ButtonState::LeftButton))
        return false;
    using Slang::HGVM::HGVMInputEventType;
    outEvent.type = pressed ? (args.isDoubleClick ? HGVMInputEventType::MouseDoubleClick
                                                  : HGVMInputEventType::MouseButtonDown)
                            : HGVMInputEventType::MouseButtonUp;
    outEvent.button = 0;
    outEvent.position[0] = float(args.x);
    outEvent.position[1] = float(args.y);
    return true;
}

} // namespace SlangITool
