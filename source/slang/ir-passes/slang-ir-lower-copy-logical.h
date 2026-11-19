#pragma once

namespace Slang
{

struct IRModule;

/// Lower copy-logical operations to simpler IR.
void lowerCopyLogical(IRModule* module);

} // namespace Slang
