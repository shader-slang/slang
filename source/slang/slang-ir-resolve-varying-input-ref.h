#pragma once

#include "slang-ir.h"

namespace Slang
{
/// Resolve recognized varying-input references in `func` to their parameter addresses.
///
/// Resolves a reference rooted in a parameter directly, or through a local or global variable
/// initialized by loading a parameter. Preserves any element-address projections. References
/// without a recognized parameter assignment remain unchanged.
void resolveVaryingInputRef(IRFunc* func);

/// Resolve varying-input references in every module-scope function, including ordinary clones of
/// entry points.
///
/// Requires completion of inlining that can expose interpolation operations, before target
/// entry-point legalization changes the varying-parameter representation.
void resolveVaryingInputRef(IRModule* module);

} // namespace Slang
