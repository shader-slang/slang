// slang-ir-lower-matrix-swizzle-store.h
#pragma once

// This file defines an IR pass that lowers MatrixSwizzleStore operations
// into per-row SwizzledStore operations. This pass should run after autodiff
// passes are complete.

namespace Slang
{

struct IRModule;

void lowerMatrixSwizzleStores(IRModule* module);

} // namespace Slang
