// slang-ir-translate-glsl-global-var.h
#ifndef SLANG_IR_TRANSLATE_GLSL_GLOBAL_VAR_H
#define SLANG_IR_TRANSLATE_GLSL_GLOBAL_VAR_H

namespace Slang
{

    struct IRModule;
    struct CodeGenContext;

    /// Translate global in/out variables defined in GLSL-flavored code
    /// into entry point parameters with system value semantics.
    void translateGLSLGlobalVar(CodeGenContext* context, IRModule* module);

}

#endif // SLANG_IR_TRANSLATE_GLSL_GLOBAL_VAR_H
