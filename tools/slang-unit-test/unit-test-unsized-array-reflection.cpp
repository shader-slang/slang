// unit-test-unsized-array-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// We pass the program layout to permit link-time folding without changing these counts.
static void checkArrayElementCounts(
    slang::ProgramLayout* programLayout,
    slang::VariableLayoutReflection* param,
    size_t expected)
{
    auto typeLayout = param->getTypeLayout();
    auto type = typeLayout->getType();
    SLANG_CHECK_ABORT(type->getKind() == slang::TypeReflection::Kind::Array);

    SLANG_CHECK(type->getElementCount() == expected);
    SLANG_CHECK(type->getElementCount((SlangReflection*)programLayout) == expected);
    SLANG_CHECK(type->getTotalArrayElementCount() == expected);

    SLANG_CHECK(typeLayout->getElementCount() == expected);
    SLANG_CHECK(typeLayout->getElementCount(programLayout) == expected);
    SLANG_CHECK(typeLayout->getTotalArrayElementCount() == expected);
}

// Test that reflection reports `SLANG_UNBOUNDED_SIZE` for an unsized array and keeps 0 for a
// zero-sized array, so that the two can be told apart.
SLANG_UNIT_TEST(unsizedArrayReflection)
{
    const char* userSourceBody = R"(
        Texture2D<float4> zeroSizedArr[0];
        Texture2D<float4> unsizedArr[];
        Texture2D<float4> sizedArr[3];

        RWStructuredBuffer<float4> output;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main()
        {
            output[0] = unsizedArr[2277].Load(int3(0, 0, 0)) + sizedArr[1].Load(int3(0, 0, 0));
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "unsizedArrayReflection",
        "unsizedArrayReflection.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("main", entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);
    SLANG_CHECK_ABORT(programLayout->getParameterCount() == 4);

    auto zeroSizedArr = programLayout->getParameterByIndex(0);
    auto unsizedArr = programLayout->getParameterByIndex(1);
    auto sizedArr = programLayout->getParameterByIndex(2);
    SLANG_CHECK_ABORT(UnownedStringSlice(zeroSizedArr->getName()) == "zeroSizedArr");
    SLANG_CHECK_ABORT(UnownedStringSlice(unsizedArr->getName()) == "unsizedArr");
    SLANG_CHECK_ABORT(UnownedStringSlice(sizedArr->getName()) == "sizedArr");

    checkArrayElementCounts(programLayout, zeroSizedArr, 0);
    checkArrayElementCounts(programLayout, unsizedArr, SLANG_UNBOUNDED_SIZE);
    checkArrayElementCounts(programLayout, sizedArr, 3);
}
