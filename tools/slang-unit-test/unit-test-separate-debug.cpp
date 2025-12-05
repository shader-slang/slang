// unit-test-translation-unit-import.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that separate debug info is generated when requested and contains the
// correct instructions.

SLANG_UNIT_TEST(separateDebug)
{
    // Source for a basic slang shader to compile to spirv.
    const char* userSourceBody = R"(
        struct TestType
        {
            float memberA;
            float3 memberB;
            RWStructuredBuffer<float> memberC;
            float getValue()
            {
                return memberA;
            }
        }
        RWStructuredBuffer<float> result;
        void main()
        {
            TestType t;
            t.memberA = 1.0;
            t.memberB = float3(1, 2, 3);
            t.memberC = result;
            var val = t.getValue();
            result[0] = val + t.memberB.x;
        }
    )";
    String userSource = userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Setup the target descriptor.
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // Setup the session descriptor.
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // Set the compile options.
    slang::CompilerOptionEntry compilerOptions[2];
    compilerOptions[0].name = slang::CompilerOptionName::DebugInformation;
    compilerOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[0].value.intValue0 = 2;

    compilerOptions[1].name = slang::CompilerOptionName::EmitSeparateDebug;
    compilerOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[1].value.intValue0 = 1;

    sessionDesc.compilerOptionEntries = compilerOptions;
    sessionDesc.compilerOptionEntryCount = 2;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // Compile the module.
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "main",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    // Use getEntryPointCompileResult to get the base and debug spirv, and metadata
    // containing the debug build identifier.
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> debugCode;
    ComPtr<slang::IMetadata> metadata;
    const char* debugBuildIdentifier = nullptr;

    // IComponentType2 is a separate interface from IComponentType, so we need
    // to query for it, and then use it to get the results.
    ComPtr<slang::ICompileResult> compileResult;
    ComPtr<slang::IComponentType2> linkedProgram2;
    SLANG_CHECK(
        linkedProgram->queryInterface(SLANG_IID_PPV_ARGS(linkedProgram2.writeRef())) == SLANG_OK);
    auto result = linkedProgram2->getEntryPointCompileResult(
        0,
        0,
        compileResult.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(result == SLANG_OK);
    SLANG_CHECK(compileResult != nullptr);
    SLANG_CHECK(compileResult->getItemCount() == 2);
    SLANG_CHECK(compileResult->getItemData(0, code.writeRef()) == SLANG_OK);
    SLANG_CHECK(compileResult->getItemData(1, debugCode.writeRef()) == SLANG_OK);
    SLANG_CHECK(compileResult->getMetadata(metadata.writeRef()) == SLANG_OK);

    debugBuildIdentifier = metadata->getDebugBuildIdentifier();
    SLANG_CHECK(debugBuildIdentifier != nullptr);

    // Get the data for the stripped SPIRV.
    // This is already verified by the separate-debug.slang test but we
    // check it here to again to verify that the API is working.
    String codeString = static_cast<const char*>(code->getBufferPointer());

    // Verify that the code contains DebugBuildIdentifier instruction
    // and the correct hash.
    SLANG_CHECK(codeString.indexOf("DebugBuildIdentifier") != -1);
    SLANG_CHECK(codeString.indexOf(debugBuildIdentifier) != -1);

    // Verify that it does not contain any other debug instructions.
    SLANG_CHECK(codeString.indexOf("DebugExpression") == -1);
    SLANG_CHECK(codeString.indexOf("DebugTypeMember") == -1);
    SLANG_CHECK(codeString.indexOf("DebugScope") == -1);
    SLANG_CHECK(codeString.indexOf("DebugLine") == -1);

    // Get the data for the non-stripped debug SPIRV and verify
    // that it contains the correct debug instructions.
    String debugCodeString = static_cast<const char*>(debugCode->getBufferPointer());
    SLANG_CHECK(debugCodeString.indexOf("DebugBuildIdentifier") != -1);
    SLANG_CHECK(debugCodeString.indexOf(debugBuildIdentifier) != -1);
    SLANG_CHECK(debugCodeString.indexOf("DebugExpression") != -1);
    SLANG_CHECK(debugCodeString.indexOf("DebugFunctionDefinition") != -1);
    SLANG_CHECK(debugCodeString.indexOf("DebugScope") != -1);
    SLANG_CHECK(debugCodeString.indexOf("DebugLine") != -1);
}
