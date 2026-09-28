// unit-test-bitfield-packing-rules.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(bitfieldPackingRulesCompilerOption)
{
    // Consider this example: Default puts both fields in one uint16_t backing word,
    // while the two type-size-splitting rules allocate a uint8_t and a uint16_t word.
    // A checked entry-point assertion exposes the resulting sizeof through the public API.
    const char* source = R"(
        struct Mixed
        {
            uint8_t a : 4;
            uint16_t b : 4;
        };

        [numthreads(1, 1, 1)]
        void computeMain()
        {
            static_assert(sizeof(Mixed) == EXPECTED_SIZE, "unexpected bitfield backing size");
        }
    )";

    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;
    SLANG_CHECK_ABORT(globalSession != nullptr);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    struct TestCase
    {
        bool setRules;
        slang::BitfieldPackingRules rules;
        bool setLegacy;
        unsigned expectedSize;
    };
    const TestCase cases[] = {
        {false, slang::BitfieldPackingRules::Default, false, 2},
        {true, slang::BitfieldPackingRules::Default, false, 2},
        {true, slang::BitfieldPackingRules::MSVC, false, 4},
        {true, slang::BitfieldPackingRules::MSBFirstMSVC, false, 4},
        {false, slang::BitfieldPackingRules::Default, true, 4},
        // An explicit new rule must override the legacy boolean through the API.
        {true, slang::BitfieldPackingRules::Default, true, 2},
    };

    for (const auto& testCase : cases)
    {
        slang::CompilerOptionEntry options[2] = {};
        SlangInt optionCount = 0;
        if (testCase.setRules)
        {
            auto& option = options[optionCount++];
            option.name = slang::CompilerOptionName::BitfieldPackingRules;
            option.value.kind = slang::CompilerOptionValueKind::Int;
            option.value.intValue0 = static_cast<int32_t>(testCase.rules);
        }
        if (testCase.setLegacy)
        {
            auto& option = options[optionCount++];
            option.name = slang::CompilerOptionName::UseMSVCStyleBitfieldPacking;
            option.value.kind = slang::CompilerOptionValueKind::Int;
            option.value.intValue0 = 1;
        }

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntryCount = optionCount;
        sessionDesc.compilerOptionEntries = options;
        slang::PreprocessorMacroDesc expectedSizeMacro = {};
        expectedSizeMacro.name = "EXPECTED_SIZE";
        expectedSizeMacro.value = testCase.expectedSize == 2 ? "2" : "4";
        sessionDesc.preprocessorMacroCount = 1;
        sessionDesc.preprocessorMacros = &expectedSizeMacro;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "bitfieldPackingRules",
            "bitfield-packing-rules.slang",
            source,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(
            module->findAndCheckEntryPoint(
                "computeMain",
                SLANG_STAGE_COMPUTE,
                entryPoint.writeRef(),
                diagnostics.writeRef()) == SLANG_OK);
        SLANG_CHECK(entryPoint != nullptr);
    }
}

SLANG_UNIT_TEST(bitfieldPackingRulesDebugInfo)
{
    auto globalSession = unitTestContext->slangGlobalSession;
    SLANG_CHECK_ABORT(globalSession != nullptr);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry options[3] = {};
    options[0].name = slang::CompilerOptionName::UseMSVCStyleBitfieldPacking;
    options[0].value.kind = slang::CompilerOptionValueKind::Int;
    options[0].value.intValue0 = 1;
    options[1].name = slang::CompilerOptionName::BitfieldPackingRules;
    options[1].value.kind = slang::CompilerOptionValueKind::Int;
    options[1].value.intValue0 = static_cast<int32_t>(slang::BitfieldPackingRules::Default);
    options[2].name = slang::CompilerOptionName::DebugInformation;
    options[2].value.kind = slang::CompilerOptionValueKind::Int;
    options[2].value.intValue0 = 2;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntryCount = 3;
    sessionDesc.compilerOptionEntries = options;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source = R"(
        RWStructuredBuffer<uint> outputBuffer;
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            outputBuffer[0] = 1;
        }
    )";
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "bitfieldPackingDebugInfo",
        "bitfield-packing-debug-info.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        entryPoint->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> code;
    SLANG_CHECK_ABORT(
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);
    auto assembly =
        UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize());
    SLANG_CHECK(assembly.indexOf(toSlice("-bitfield-packing-rules default")) != -1);
    SLANG_CHECK(assembly.indexOf(toSlice("-msvc-style-bitfield-packing")) == -1);
}
