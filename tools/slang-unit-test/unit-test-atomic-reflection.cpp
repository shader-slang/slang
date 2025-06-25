// unit-test-atomic-reflection.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

SLANG_UNIT_TEST(atomicReflection)
{
    const char* userSourceBody = R"(
        RWStructuredBuffer<Atomic<uint>> barA;
        RWStructuredBuffer<uint> barB;
        struct TestStruct {
            Atomic<uint> fieldA;
            Atomic<uint> fieldB[2];
            vector<Atomic<uint>, 2> fieldC;
        };
        RWStructuredBuffer<TestStruct> barC;

        [shader("compute")]
        [numthreads(64, 1, 1)]
        void main(uint2 dispatchThreadId : SV_DispatchThreadID)
        {
        }
        )";
    String userSource = userSourceBody;
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

    auto barAVariableLayout = reflection->getParameterByIndex(0);
    auto barBVariableLayout = reflection->getParameterByIndex(1);
    auto barCVariableLayout = reflection->getParameterByIndex(2);

    SLANG_CHECK(barAVariableLayout != nullptr);
    SLANG_CHECK(barBVariableLayout != nullptr);
    SLANG_CHECK(barCVariableLayout != nullptr);

    auto barAVariable = barAVariableLayout->getVariable();
    auto barBVariable = barBVariableLayout->getVariable();
    auto barCVariable = barCVariableLayout->getVariable();

    SLANG_CHECK(barAVariable != nullptr);
    SLANG_CHECK(barBVariable != nullptr);
    SLANG_CHECK(barCVariable != nullptr);

    auto barAType = barAVariable->getType();
    auto barBType = barBVariable->getType();
    auto barCType = barCVariable->getType();

    SLANG_CHECK(barAType->getKind() == slang::TypeReflection::Kind::Resource);
    SLANG_CHECK(barBType->getKind() == slang::TypeReflection::Kind::Resource);
    SLANG_CHECK(barCType->getKind() == slang::TypeReflection::Kind::Resource);

    auto barAElementType = barAType->getElementType();
    auto barBElementType = barBType->getElementType();
    auto barCElementType = barCType->getElementType();

    SLANG_CHECK(barAElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(barBElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(barAElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
    SLANG_CHECK(barBElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    auto barAResourceResultType = barAType->getResourceResultType();
    auto barBResourceResultType = barBType->getResourceResultType();

    SLANG_CHECK(barAResourceResultType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(barBResourceResultType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(barAResourceResultType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
    SLANG_CHECK(barBResourceResultType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    // Atomics embedded in structs require traversing the fields
    SLANG_CHECK(barCElementType->getKind() == slang::TypeReflection::Kind::Struct);

    auto barCFieldCount = barCElementType->getFieldCount();
    SLANG_CHECK(barCFieldCount == 3);

    auto fieldA = barCElementType->getFieldByIndex(0);
    auto fieldB = barCElementType->getFieldByIndex(1);
    auto fieldC = barCElementType->getFieldByIndex(2);

    SLANG_CHECK(fieldA != nullptr);
    SLANG_CHECK(fieldB != nullptr);
    SLANG_CHECK(fieldC != nullptr);

    auto fieldAType = fieldA->getType();
    auto fieldBType = fieldB->getType();
    auto fieldCType = fieldC->getType();

    std::cout << "fieldAType: " << (int) fieldAType->getKind() << std::endl;
    std::cout << "fieldBType: " << (int) fieldBType->getKind() << std::endl;
    std::cout << "fieldCType: " << (int) fieldCType->getKind() << std::endl;

    SLANG_CHECK(fieldAType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(fieldBType->getKind() == slang::TypeReflection::Kind::Array);
    SLANG_CHECK(fieldCType->getKind() == slang::TypeReflection::Kind::Vector);

}
