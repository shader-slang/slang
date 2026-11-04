// unit-test-glsl-layout-late-entry-point.cpp

#include "core/slang-string-util.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>

using namespace Slang;

// GLSL declares a compute thread-group size, `early_fragment_tests` or a compute derivative
// group with a standalone `layout(...) in;` beside the entry point. A module loaded through the API
// is lowered to IR before an entry point without `[shader]` is found with `findAndCheckEntryPoint`,
// which is where the layout becomes an attribute of the entry point. These tests compile such entry
// points and check that the generated code still carries the layout.

static void printDiagnostics(slang::IBlob* diagnostics)
{
    if (diagnostics)
    {
        fprintf(
            stderr,
            "%.*s\n",
            (int)diagnostics->getBufferSize(),
            (const char*)diagnostics->getBufferPointer());
    }
}

// Load `source` as a GLSL module, find `entryPointName` in it with `findAndCheckEntryPoint`,
// and link the two into `outLinkedProgram`. The session is returned as well, because the
// linked program and its reflection need the session alive.
static void linkLateGLSLEntryPoint(
    UnitTestContext* unitTestContext,
    const char* source,
    const char* entryPointName,
    SlangStage stage,
    slang::TargetDesc const& targetDesc,
    ComPtr<slang::ISession>& outSession,
    ComPtr<slang::IComponentType>& outLinkedProgram)
{
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, outSession.writeRef())));

    // The `.glsl` extension selects GLSL as the source language of the module.
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = outSession->loadModuleFromSourceString(
        "glslLayoutLateEntryPoint",
        "glsl-layout-late-entry-point.glsl",
        source,
        diagnostics.writeRef());
    printDiagnostics(diagnostics);
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    diagnostics.setNull();
    module->findAndCheckEntryPoint(
        entryPointName,
        stage,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    printDiagnostics(diagnostics);
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        outSession->createCompositeComponentType(components, 2, composite.writeRef())));
    diagnostics.setNull();
    SlangResult linkResult = composite->link(outLinkedProgram.writeRef(), diagnostics.writeRef());
    printDiagnostics(diagnostics);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(linkResult));
}

static String compileLateGLSLEntryPoint(
    UnitTestContext* unitTestContext,
    const char* source,
    const char* entryPointName,
    SlangStage stage,
    slang::TargetDesc const& targetDesc,
    ComPtr<slang::ISession>& outSession,
    ComPtr<slang::IComponentType>& outLinkedProgram)
{
    linkLateGLSLEntryPoint(
        unitTestContext,
        source,
        entryPointName,
        stage,
        targetDesc,
        outSession,
        outLinkedProgram);

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult codeResult =
        outLinkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    printDiagnostics(diagnostics);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(codeResult) && code);
    return String(
        (const char*)code->getBufferPointer(),
        (const char*)code->getBufferPointer() + code->getBufferSize());
}

// Link the compute entry point `computeMain` of `source` for SPIR-V, check that generating its
// code fails, and return the diagnostics. An entry point validated after lowering has its
// derivative-group mode checked against its thread-group size only when its layout IR is lowered,
// so the error comes from code generation, not from `findAndCheckEntryPoint`.
static String getLateGLSLComputeEntryPointErrors(
    UnitTestContext* unitTestContext,
    const char* source)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = unitTestContext->slangGlobalSession->findProfile("spirv_1_5");

    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    linkLateGLSLEntryPoint(
        unitTestContext,
        source,
        "computeMain",
        SLANG_STAGE_COMPUTE,
        targetDesc,
        session,
        linkedProgram);

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult codeResult =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(codeResult));
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    return String(
        (const char*)diagnostics->getBufferPointer(),
        (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
}

SLANG_UNIT_TEST(glslLocalSizeOnLateEntryPoint)
{
    const char* source = R"(
        #version 450
        buffer OutputBlock { uint data[]; } outputBuffer;
        layout(local_size_x = 16, local_size_y = 2) in;
        void computeMain()
        {
            outputBuffer.data[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
        }
    )";

    struct TargetCase
    {
        SlangCompileTarget format;
        const char* profile;
        const char* expectedThreadGroupSize;
    };
    const TargetCase targetCases[] = {
        {SLANG_SPIRV_ASM, "spirv_1_5", "LocalSize 16 2 1"},
        {SLANG_HLSL, "sm_6_0", "[numthreads(16, 2, 1)]"},
        {SLANG_WGSL, "", "@workgroup_size(16, 2, 1)"},
    };

    for (auto& targetCase : targetCases)
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = targetCase.format;
        targetDesc.profile = unitTestContext->slangGlobalSession->findProfile(targetCase.profile);

        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> linkedProgram;
        String code = compileLateGLSLEntryPoint(
            unitTestContext,
            source,
            "computeMain",
            SLANG_STAGE_COMPUTE,
            targetDesc,
            session,
            linkedProgram);

        SLANG_CHECK(code.indexOf(UnownedStringSlice(targetCase.expectedThreadGroupSize)) >= 0);

        // Reflection reads the AST; code generation reads linked IR, so assert that their
        // thread-group sizes agree.
        auto entryPointReflection = linkedProgram->getLayout()->getEntryPointByIndex(0);
        SlangUInt sizes[3] = {};
        entryPointReflection->getComputeThreadGroupSize(3, sizes);
        SLANG_CHECK(sizes[0] == 16 && sizes[1] == 2 && sizes[2] == 1);
    }
}

SLANG_UNIT_TEST(glslLocalSizeIdOnLateEntryPoint)
{
    // The thread-group size refers to specialization constants, so the decoration that the
    // generated code takes from the program's layout IR has operands of its own.
    const char* source = R"(
        #version 450
        [vk::constant_id(1)]
        const int sizeX = 4;
        layout(local_size_x_id = 1, local_size_y_id = 0, local_size_z = 2) in;
        void computeMain()
        {
        }
    )";

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = unitTestContext->slangGlobalSession->findProfile("spirv_1_5");

    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    String spirvAsm = compileLateGLSLEntryPoint(
        unitTestContext,
        source,
        "computeMain",
        SLANG_STAGE_COMPUTE,
        targetDesc,
        session,
        linkedProgram);

    // The copied decoration's operands must resolve to the module's specialization constants.
    // Expect `OpExecutionModeId %computeMain LocalSizeId %sizeX %<y> %int_2`: x is the declared
    // constant, y is the constant synthesized for `local_size_y_id = 0`, and z is the literal 2.
    Index modeIndex = spirvAsm.indexOf(UnownedStringSlice("LocalSizeId "));
    SLANG_CHECK_ABORT(modeIndex >= 0);
    UnownedStringSlice mode = spirvAsm.getUnownedSlice().tail(modeIndex);
    mode = mode.head(mode.indexOf('\n')).trim();
    List<UnownedStringSlice> operands;
    StringUtil::split(mode, ' ', operands);
    SLANG_CHECK_ABORT(operands.getCount() == 4);
    SLANG_CHECK(operands[1] == toSlice("%sizeX"));
    SLANG_CHECK(operands[3] == toSlice("%int_2"));
    SLANG_CHECK(spirvAsm.indexOf(UnownedStringSlice("OpDecorate %sizeX SpecId 1")) >= 0);
    String yDecoration = String("OpDecorate ") + operands[2] + " SpecId 0";
    SLANG_CHECK(spirvAsm.indexOf(yDecoration.getUnownedSlice()) >= 0);
}

SLANG_UNIT_TEST(glslEarlyFragmentTestsOnLateEntryPoint)
{
    const char* source = R"(
        #version 450
        layout(early_fragment_tests) in;
        layout(location = 0) out vec4 color;
        void fragMain()
        {
            color = vec4(1.0);
        }
    )";

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = unitTestContext->slangGlobalSession->findProfile("spirv_1_5");

    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    String spirvAsm = compileLateGLSLEntryPoint(
        unitTestContext,
        source,
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        targetDesc,
        session,
        linkedProgram);

    SLANG_CHECK(spirvAsm.indexOf(UnownedStringSlice("EarlyFragmentTests")) >= 0);
}

SLANG_UNIT_TEST(glslDerivativeGroupOnLateEntryPoint)
{
    // Without a mode, `inferDerivativeGroupMode` picks quads for a 4x4 group, so the
    // generated code uses the linear mode only if `layout(derivative_group_linearNV) in;`
    // reaches the entry point.
    const char* source = R"(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        buffer OutputBlock { float data[]; } outputBuffer;
        layout(local_size_x = 4, local_size_y = 4) in;
        layout(derivative_group_linearNV) in;
        void computeMain()
        {
            float v = float(gl_LocalInvocationIndex);
            outputBuffer.data[gl_LocalInvocationIndex] = dFdx(v);
        }
    )";

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = unitTestContext->slangGlobalSession->findProfile("spirv_1_5");

    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    String spirvAsm = compileLateGLSLEntryPoint(
        unitTestContext,
        source,
        "computeMain",
        SLANG_STAGE_COMPUTE,
        targetDesc,
        session,
        linkedProgram);

    SLANG_CHECK(spirvAsm.indexOf(UnownedStringSlice("DerivativeGroupLinearKHR")) >= 0);
    SLANG_CHECK(spirvAsm.indexOf(UnownedStringSlice("DerivativeGroupQuadsKHR")) < 0);
}

SLANG_UNIT_TEST(glslDerivativeGroupQuadsMismatchOnLateEntryPoint)
{
    // Quads need an even X and Y. Without the lifted mode, `inferDerivativeGroupMode` picks
    // linear for a group with Y == 1 and the shader compiles, so only the lifted
    // `layout(derivative_group_quadsNV) in;` produces the error that `-entry` reports.
    const char* source = R"(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        buffer OutputBlock { float data[]; } outputBuffer;
        layout(local_size_x = 16) in;
        layout(derivative_group_quadsNV) in;
        void computeMain()
        {
            float v = float(gl_LocalInvocationIndex);
            outputBuffer.data[gl_LocalInvocationIndex] = dFdx(v);
        }
    )";

    String errors = getLateGLSLComputeEntryPointErrors(unitTestContext, source);
    SLANG_CHECK(errors.indexOf(UnownedStringSlice("E31210")) >= 0);
}

SLANG_UNIT_TEST(glslDerivativeGroupConflictOnLateEntryPoint)
{
    const char* source = R"(
        #version 450
        #extension GL_NV_compute_shader_derivatives : require
        buffer OutputBlock { float data[]; } outputBuffer;
        layout(local_size_x = 4, local_size_y = 4) in;
        layout(derivative_group_quadsNV) in;
        layout(derivative_group_linearNV) in;
        void computeMain()
        {
            float v = float(gl_LocalInvocationIndex);
            outputBuffer.data[gl_LocalInvocationIndex] = dFdx(v);
        }
    )";

    String errors = getLateGLSLComputeEntryPointErrors(unitTestContext, source);
    SLANG_CHECK(errors.indexOf(UnownedStringSlice("E31212")) >= 0);
}
