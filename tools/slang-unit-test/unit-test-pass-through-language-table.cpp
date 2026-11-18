// unit-test-pass-through-language-table.cpp

#include "compiler-core/slang-downstream-compiler-util.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// The pass-through source-language gate answers from DownstreamCompilerInfo's table, so a wrong
// entry silently regresses (or blocks) a working pass-through spelling. Glslang and spirv-opt
// are both served by the slang-glslang wrapper, which compiles GLSL to SPIR-V and optimizes
// SPIR-V, so both spellings must accept both languages; the C/C++ compilers must keep rejecting
// shader source; Slang input is validated as HLSL for the pass-through language gate because no
// downstream compiler consumes Slang directly. The compilers with no recorded languages are
// skipped by the gate rather than assumed to reject everything, so their entries must stay
// zero-flag.
SLANG_UNIT_TEST(passThroughLanguageTable)
{
    // The slang-glslang wrapper serves both spellings and takes GLSL or SPIR-V.
    for (auto compiler : {SLANG_PASS_THROUGH_GLSLANG, SLANG_PASS_THROUGH_SPIRV_OPT})
    {
        SLANG_CHECK(DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_GLSL));
        SLANG_CHECK(DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_SPIRV));
        SLANG_CHECK(!DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_HLSL));
    }

    // The C/C++ compilers accept C and C++ and reject every shader language.
    for (auto compiler :
         {SLANG_PASS_THROUGH_CLANG,
          SLANG_PASS_THROUGH_GCC,
          SLANG_PASS_THROUGH_VISUAL_STUDIO,
          SLANG_PASS_THROUGH_LLVM,
          SLANG_PASS_THROUGH_GENERIC_C_CPP})
    {
        SLANG_CHECK(DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_C));
        SLANG_CHECK(DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_CPP));
        for (auto language :
             {SLANG_SOURCE_LANGUAGE_SLANG,
              SLANG_SOURCE_LANGUAGE_HLSL,
              SLANG_SOURCE_LANGUAGE_GLSL,
              SLANG_SOURCE_LANGUAGE_CUDA})
        {
            SLANG_CHECK(!DownstreamCompilerInfo::canCompile(compiler, language));
        }
    }

    // The HLSL compilers accept HLSL and reject the other shader languages.
    for (auto compiler : {SLANG_PASS_THROUGH_DXC, SLANG_PASS_THROUGH_FXC})
    {
        SLANG_CHECK(DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_HLSL));
        SLANG_CHECK(!DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_GLSL));
        SLANG_CHECK(!DownstreamCompilerInfo::canCompile(compiler, SLANG_SOURCE_LANGUAGE_C));
    }

    // NVRTC compiles CUDA and nothing else.
    SLANG_CHECK(
        DownstreamCompilerInfo::canCompile(SLANG_PASS_THROUGH_NVRTC, SLANG_SOURCE_LANGUAGE_CUDA));
    SLANG_CHECK(
        !DownstreamCompilerInfo::canCompile(SLANG_PASS_THROUGH_NVRTC, SLANG_SOURCE_LANGUAGE_C));

    // The compilers whose languages are not recorded are skipped by the gate, so their entries
    // must stay zero-flag.
    for (auto compiler :
         {SLANG_PASS_THROUGH_METAL,
          SLANG_PASS_THROUGH_TINT,
          SLANG_PASS_THROUGH_SPIRV_DIS,
          SLANG_PASS_THROUGH_SPIRV_LINK})
    {
        SLANG_CHECK(DownstreamCompilerInfo::getInfo(compiler).sourceLanguageFlags == 0);
    }

    // Slang input is validated as HLSL for the pass-through language gate; everything else
    // passes through.
    SLANG_CHECK(
        DownstreamCompilerInfo::getPassThroughCompileLanguage(SLANG_SOURCE_LANGUAGE_SLANG) ==
        SLANG_SOURCE_LANGUAGE_HLSL);
    SLANG_CHECK(
        DownstreamCompilerInfo::getPassThroughCompileLanguage(SLANG_SOURCE_LANGUAGE_GLSL) ==
        SLANG_SOURCE_LANGUAGE_GLSL);

    // The generic C/C++ entry is load-bearing for `setDefaultDownstreamCompiler` too.
    ComPtr<slang::IGlobalSession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(slang::createGlobalSession(session.writeRef())));
    SLANG_CHECK(SLANG_SUCCEEDED(session->setDefaultDownstreamCompiler(
        SLANG_SOURCE_LANGUAGE_C,
        SLANG_PASS_THROUGH_GENERIC_C_CPP)));
}
