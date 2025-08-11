// slang-pass-through.h
#pragma once

//
// This file gathers together declarations for utility code
// related to the concept of "pass-through" compilation.
//
// Note that in the Slang codebase there is an unfortunate
// conflation of terminology (and a resulting conflation
// in a lot of the implementation logic) between the cases
// of true *pass-through* compilation and the logic related
// to *downstream* compilers:
//
// * While the Slang compiler is architected to support direct
//   generation of binary output code (e.g., there is support
//   for emitting SPIR-V directly from Slang IR), many targets
//   are supported by first generating intermediate source code
//   from Slang IR and then invoking a *downstream* compiler on
//   that intermediate source code to produce output binaries.
//   This is an important kind of compilation flow that Slang
//   was designed to support.
//
// * In contrast, true *pass-through* compilation is a legacy
//   feature of `slangc` that exists almost entirely to support
//   some of the earliest test cases that were written for the
//   compiler. In true pass-through mode, `slangc` skips invoking
//   large parts of the Slang compiler (the entire front-end,
//   along with all of the back-end up to the point that intermediate
//   source code would be generated) and then uses the *input*
//   source as if it was the *intermediate* code for the "last mile"
//   of code generation (invoking a downstream compiler). This
//   feature can and *should* be deprecate and removed, because
//   the complexity it creates for the rest of the compiler is
//   no longer worth it.
//
// This file may contain a mix of declarations used for one or
// both of the above purposes, just because the terminology used
// in the codebase isn't always precise or clear. Over time the
// declarations here can and should be more clearly partitioned
// so that we can distinguish the essential (downstream compilation)
// parts, and the parts that should eventually get removed (true
// pass-through compilation).
//

#include "../core/slang-string.h"
#include "slang-target.h"

namespace Slang
{

enum class PassThroughMode : SlangPassThroughIntegral
{
    None = SLANG_PASS_THROUGH_NONE,                  ///< don't pass through: use Slang compiler
    Fxc = SLANG_PASS_THROUGH_FXC,                    ///< pass through HLSL to `D3DCompile` API
    Dxc = SLANG_PASS_THROUGH_DXC,                    ///< pass through HLSL to `IDxcCompiler` API
    Glslang = SLANG_PASS_THROUGH_GLSLANG,            ///< pass through GLSL to `glslang` library
    SpirvDis = SLANG_PASS_THROUGH_SPIRV_DIS,         ///< pass through spirv-dis
    Clang = SLANG_PASS_THROUGH_CLANG,                ///< Pass through clang compiler
    VisualStudio = SLANG_PASS_THROUGH_VISUAL_STUDIO, ///< Visual studio compiler
    Gcc = SLANG_PASS_THROUGH_GCC,                    ///< Gcc compiler
    GenericCCpp = SLANG_PASS_THROUGH_GENERIC_C_CPP,  ///< Generic C/C++ compiler
    NVRTC = SLANG_PASS_THROUGH_NVRTC,                ///< NVRTC CUDA compiler
    LLVM = SLANG_PASS_THROUGH_LLVM,                  ///< LLVM 'compiler'
    SpirvOpt = SLANG_PASS_THROUGH_SPIRV_OPT,         ///< pass thorugh spirv to spirv-opt
    MetalC = SLANG_PASS_THROUGH_METAL,
    Tint = SLANG_PASS_THROUGH_TINT,            ///< pass through spirv to Tint API
    SpirvLink = SLANG_PASS_THROUGH_SPIRV_LINK, ///< pass through spirv to spirv-link
    CountOf = SLANG_PASS_THROUGH_COUNT_OF,
};
void printDiagnosticArg(StringBuilder& sb, PassThroughMode val);

/// Given a target returns the required downstream compiler
PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target);

/// Given a target returns a downstream compiler the prelude should be taken from.
SourceLanguage getDefaultSourceLanguageForDownstreamCompiler(PassThroughMode compiler);

/* Report an error appearing from external compiler to the diagnostic sink error to the diagnostic
sink.
@param compilerName The name of the compiler the error came for (or nullptr if not known)
@param res Result associated with the error. The error code will be reported. (Can take HRESULT -
and will expand to string if known)
@param diagnostic The diagnostic string associated with the compile failure
@param sink The diagnostic sink to report to */
void reportExternalCompileError(
    const char* compilerName,
    SlangResult res,
    const UnownedStringSlice& diagnostic,
    DiagnosticSink* sink);

} // namespace Slang
