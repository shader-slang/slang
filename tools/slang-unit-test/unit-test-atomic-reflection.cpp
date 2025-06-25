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
        RWStructuredBuffer<uint> bufA;  // for reference
        RWStructuredBuffer<Atomic<uint>> bufB;
        struct TestStruct {
            Atomic<uint> fieldA;
            Atomic<uint> fieldB[2];
            vector<Atomic<uint>, 2> fieldC;
        };
        RWStructuredBuffer<TestStruct> bufC;

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

    auto bufAVariableLayout = reflection->getParameterByIndex(0);
    auto bufBVariableLayout = reflection->getParameterByIndex(1);
    auto bufCVariableLayout = reflection->getParameterByIndex(2);

    SLANG_CHECK(bufAVariableLayout != nullptr);
    SLANG_CHECK(bufBVariableLayout != nullptr);
    SLANG_CHECK(bufCVariableLayout != nullptr);

    // This test walks down the parameters in a different way from the JSON
    // option. The slangc option "-reflection-json" walks:
    //   param->getTypeLayout()->getElementTypeLayout()->getType()->getKind()
    // This test uses an alternate walk:
    //   param->getVariable()->getType()->getElementType()->getKind()

    auto bufAVariable = bufAVariableLayout->getVariable();
    auto bufBVariable = bufBVariableLayout->getVariable();
    auto bufCVariable = bufCVariableLayout->getVariable();

    SLANG_CHECK(bufAVariable != nullptr);
    SLANG_CHECK(bufBVariable != nullptr);
    SLANG_CHECK(bufCVariable != nullptr);

    auto bufAType = bufAVariable->getType();
    auto bufBType = bufBVariable->getType();
    auto bufCType = bufCVariable->getType();

    SLANG_CHECK(bufAType->getKind() == slang::TypeReflection::Kind::Resource);
    SLANG_CHECK(bufBType->getKind() == slang::TypeReflection::Kind::Resource);
    SLANG_CHECK(bufCType->getKind() == slang::TypeReflection::Kind::Resource);

    auto bufAElementType = bufAType->getElementType();
    auto bufBElementType = bufBType->getElementType();
    auto bufCElementType = bufCType->getElementType();

    SLANG_CHECK(bufAElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(bufBElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(bufAElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
    SLANG_CHECK(bufBElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    auto bufAResResultType = bufAType->getResourceResultType();
    auto bufBResResultType = bufBType->getResourceResultType();

    SLANG_CHECK(bufAResResultType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(bufBResResultType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(bufAResResultType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
    SLANG_CHECK(bufBResResultType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    // Atomics embedded in structs require traversing the fields
    SLANG_CHECK(bufCElementType->getKind() == slang::TypeReflection::Kind::Struct);

    auto bufCFieldCount = bufCElementType->getFieldCount();
    SLANG_CHECK(bufCFieldCount == 3);

    auto fieldA = bufCElementType->getFieldByIndex(0);
    auto fieldB = bufCElementType->getFieldByIndex(1);
    auto fieldC = bufCElementType->getFieldByIndex(2);

    SLANG_CHECK(fieldA != nullptr);
    SLANG_CHECK(fieldB != nullptr);
    SLANG_CHECK(fieldC != nullptr);

    auto fieldAType = fieldA->getType();
    auto fieldBType = fieldB->getType();
    auto fieldCType = fieldC->getType();

    SLANG_CHECK(fieldAType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(fieldAType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    SLANG_CHECK(fieldBType->getKind() == slang::TypeReflection::Kind::Array);
    SLANG_CHECK(fieldBType->getElementCount() == 2);
    auto fieldBElementType = fieldBType->getElementType();
    SLANG_CHECK(fieldBElementType != nullptr);
    SLANG_CHECK(fieldBElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(fieldBElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    SLANG_CHECK(fieldCType->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK(fieldCType->getElementCount() == 2);
    auto fieldCElementType = fieldCType->getElementType();
    SLANG_CHECK(fieldCElementType != nullptr);
    SLANG_CHECK(fieldCElementType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(fieldCElementType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
}
