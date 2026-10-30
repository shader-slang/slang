// unit-test-find-pointer-type-by-name.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test that the reflection API finds pointer types by name. A type string such as "int**" ends
// with its run of `*`, so the parser has to read a run of `*` followed by the end of the input
// as a pointer type rather than as a multiplication with a missing operand.

SLANG_UNIT_TEST(findPointerTypeByName)
{
    const char* userSourceBody = R"(
        public struct S { int x; };
        )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
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
    SLANG_CHECK_ABORT(module != nullptr);

    auto reflection = module->getLayout();

    const char* pointerTypeNames[] = {"int*", "int**", "int***", "S**"};
    for (auto name : pointerTypeNames)
    {
        auto type = reflection->findTypeByName(name);
        SLANG_CHECK_ABORT(type != nullptr);
        SLANG_CHECK(type->getKind() == slang::TypeReflection::Kind::Pointer);
    }
}
