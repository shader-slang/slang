// unit-test-enum-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that reflection API can correctly report info about enum declarations.

SLANG_UNIT_TEST(enumReflection)
{
    const char* userSourceBody = R"(
        enum Mode {
            A,
            B = 123 * 4,
            C = B + 5
        };
        [UnscopedEnum] enum Flags {
            kFlagA = 1 << 0,
        };
        Mode g_mode;
        RWStructuredBuffer<uint> g_results;

        [numthreads(1)] void computeMain(uint tid: SV_DispatchThreadID) {
            if (g_mode == Mode::A) g_results[tid] = tid * 123;
        }
        )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    auto reflection = module->getLayout();
    auto enumType = reflection->findTypeByName("Mode");

    SLANG_CHECK_ABORT(enumType != nullptr);
    SLANG_CHECK(enumType->getKind() == slang::TypeReflection::Kind::Enum);
    SLANG_CHECK(enumType->getFieldCount() == 3);
    int64_t value = -1;

    auto case0 = enumType->getFieldByIndex(0);
    case0->getDefaultValueInt(&value);
    SLANG_CHECK(strcmp(case0->getName(), "A") == 0 && value == 0);

    auto case1 = enumType->getFieldByIndex(1);
    case1->getDefaultValueInt(&value);
    SLANG_CHECK(strcmp(case1->getName(), "B") == 0 && value == 123 * 4);

    auto case2 = enumType->getFieldByIndex(2);
    case2->getDefaultValueInt(&value);
    SLANG_CHECK(strcmp(case2->getName(), "C") == 0 && value == 123 * 4 + 5);

    auto unscopedEnumType = reflection->findTypeByName("Flags");
    SLANG_CHECK_ABORT(unscopedEnumType != nullptr);
    SLANG_CHECK(unscopedEnumType->findAttributeByName("UnscopedEnum") != nullptr);
}
