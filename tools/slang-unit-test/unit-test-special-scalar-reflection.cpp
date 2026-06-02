// unit-test-special-scalar-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(specialScalarReflection)
{
    const char* userSourceBody = R"(
        struct TestStruct
        {
            BFloat16 bf;
            FloatE4M3 e4;
            FloatE5M2 e5;
            intptr_t ip;
            uintptr_t up;
            vector<BFloat16, 2> vbf;
            vector<FloatE4M3, 2> ve4;
            vector<FloatE5M2, 2> ve5;
            vector<intptr_t, 2> vip;
            vector<uintptr_t, 2> vup;
        };

        StructuredBuffer<TestStruct> gData;
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CUDA_SOURCE;

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
    SLANG_CHECK(reflection != nullptr);

    auto gDataLayout = reflection->getParameterByIndex(0);
    SLANG_CHECK(gDataLayout != nullptr);

    auto gDataType = gDataLayout->getType();
    SLANG_CHECK(gDataType != nullptr);
    SLANG_CHECK(gDataType->getKind() == slang::TypeReflection::Kind::Resource);

    auto resultType = gDataType->getResourceResultType();
    SLANG_CHECK(resultType != nullptr);
    SLANG_CHECK(resultType->getKind() == slang::TypeReflection::Kind::Struct);
    SLANG_CHECK_ABORT(resultType->getFieldCount() == 10);

    auto bfField = resultType->getFieldByIndex(0);
    auto e4Field = resultType->getFieldByIndex(1);
    auto e5Field = resultType->getFieldByIndex(2);
    auto ipField = resultType->getFieldByIndex(3);
    auto upField = resultType->getFieldByIndex(4);
    auto vbfField = resultType->getFieldByIndex(5);
    auto ve4Field = resultType->getFieldByIndex(6);
    auto ve5Field = resultType->getFieldByIndex(7);
    auto vipField = resultType->getFieldByIndex(8);
    auto vupField = resultType->getFieldByIndex(9);

    SLANG_CHECK(bfField != nullptr);
    SLANG_CHECK(e4Field != nullptr);
    SLANG_CHECK(e5Field != nullptr);
    SLANG_CHECK(ipField != nullptr);
    SLANG_CHECK(upField != nullptr);
    SLANG_CHECK(vbfField != nullptr);
    SLANG_CHECK(ve4Field != nullptr);
    SLANG_CHECK(ve5Field != nullptr);
    SLANG_CHECK(vipField != nullptr);
    SLANG_CHECK(vupField != nullptr);

    auto bfType = bfField->getType();
    auto e4Type = e4Field->getType();
    auto e5Type = e5Field->getType();
    auto ipType = ipField->getType();
    auto upType = upField->getType();
    auto vbfType = vbfField->getType();
    auto ve4Type = ve4Field->getType();
    auto ve5Type = ve5Field->getType();
    auto vipType = vipField->getType();
    auto vupType = vupField->getType();

    SLANG_CHECK(bfType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(e4Type->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(e5Type->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(ipType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK(upType->getKind() == slang::TypeReflection::Kind::Scalar);

    SLANG_CHECK(bfType->getScalarType() == slang::TypeReflection::ScalarType::BFloat16);
    SLANG_CHECK(e4Type->getScalarType() == slang::TypeReflection::ScalarType::FloatE4M3);
    SLANG_CHECK(e5Type->getScalarType() == slang::TypeReflection::ScalarType::FloatE5M2);
    SLANG_CHECK(ipType->getScalarType() == slang::TypeReflection::ScalarType::IntPtr);
    SLANG_CHECK(upType->getScalarType() == slang::TypeReflection::ScalarType::UIntPtr);

    SLANG_CHECK(bfType->getRowCount() == 1);
    SLANG_CHECK(bfType->getColumnCount() == 1);
    SLANG_CHECK(e4Type->getRowCount() == 1);
    SLANG_CHECK(e4Type->getColumnCount() == 1);
    SLANG_CHECK(e5Type->getRowCount() == 1);
    SLANG_CHECK(e5Type->getColumnCount() == 1);
    SLANG_CHECK(ipType->getRowCount() == 1);
    SLANG_CHECK(ipType->getColumnCount() == 1);
    SLANG_CHECK(upType->getRowCount() == 1);
    SLANG_CHECK(upType->getColumnCount() == 1);

    SLANG_CHECK(vbfType->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK(ve4Type->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK(ve5Type->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK(vipType->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK(vupType->getKind() == slang::TypeReflection::Kind::Vector);

    auto vbfElementType = vbfType->getElementType();
    auto ve4ElementType = ve4Type->getElementType();
    auto ve5ElementType = ve5Type->getElementType();
    auto vipElementType = vipType->getElementType();
    auto vupElementType = vupType->getElementType();

    SLANG_CHECK(vbfElementType != nullptr);
    SLANG_CHECK(ve4ElementType != nullptr);
    SLANG_CHECK(ve5ElementType != nullptr);
    SLANG_CHECK(vipElementType != nullptr);
    SLANG_CHECK(vupElementType != nullptr);

    SLANG_CHECK(vbfElementType->getScalarType() == slang::TypeReflection::ScalarType::BFloat16);
    SLANG_CHECK(ve4ElementType->getScalarType() == slang::TypeReflection::ScalarType::FloatE4M3);
    SLANG_CHECK(ve5ElementType->getScalarType() == slang::TypeReflection::ScalarType::FloatE5M2);
    SLANG_CHECK(vipElementType->getScalarType() == slang::TypeReflection::ScalarType::IntPtr);
    SLANG_CHECK(vupElementType->getScalarType() == slang::TypeReflection::ScalarType::UIntPtr);
}
