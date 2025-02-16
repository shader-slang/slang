// slang-ir-translate-in-out-global-var.h
// #pragma once

namespace Slang
{

struct IRModule;
struct CodeGenContext;

/// Translate GLSL-flavored global in/out variables into
/// entry point parameters with system value semantics.
void translateInOutGlobalVar(CodeGenContext* context, IRModule* module);

} // namespace Slang
