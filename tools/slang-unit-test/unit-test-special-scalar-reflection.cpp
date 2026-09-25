// unit-test-special-scalar-reflection.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(specialScalarReflection)
{
    const char* userSourceBody = R"(
        struct TestStruct
        {
            BFloat16 bf = BFloat16(1.0);
            FloatE4M3 e4 = FloatE4M3(1.0);
            FloatE5M2 e5 = FloatE5M2(1.0);
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

    ComPtr<slang::IBlob> bfDefault;
    SLANG_CHECK(SLANG_SUCCEEDED(bfField->getDefaultValueBlob(bfDefault.writeRef())));
    SLANG_CHECK(bfDefault->getBufferSize() == sizeof(uint16_t));
    SLANG_CHECK(((const uint16_t*)bfDefault->getBufferPointer())[0] == 0x3f80);

    ComPtr<slang::IBlob> e4Default;
    SLANG_CHECK(SLANG_SUCCEEDED(e4Field->getDefaultValueBlob(e4Default.writeRef())));
    SLANG_CHECK(e4Default->getBufferSize() == sizeof(uint8_t));
    SLANG_CHECK(((const uint8_t*)e4Default->getBufferPointer())[0] == 0x38);

    ComPtr<slang::IBlob> e5Default;
    SLANG_CHECK(SLANG_SUCCEEDED(e5Field->getDefaultValueBlob(e5Default.writeRef())));
    SLANG_CHECK(e5Default->getBufferSize() == sizeof(uint8_t));
    SLANG_CHECK(((const uint8_t*)e5Default->getBufferPointer())[0] == 0x3c);
}

// Check the host-visible record ABI against the CUDA prelude's component and native vector types.
SLANG_UNIT_TEST(cudaSpecialScalarLayout)
{
    const char* source = R"(
        struct Wrapped<T>
        {
            uint16_t prefix;
            T value;
            uint16_t suffix;
        };
        struct Holder<T>
        {
            Wrapped<T> values[3];
            uint16_t tail;
        };
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CUDA_SOURCE;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "cudaSpecialScalarLayout",
        "cudaSpecialScalarLayout.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    auto reflection = module->getLayout();
    SLANG_CHECK_ABORT(reflection != nullptr);

    // These sizes, alignments and offsets are the actual CUDA prelude ABI. In particular,
    // BF4 uses four 2-byte component fields, while native ushort4 is 8-byte aligned.
    const struct
    {
        const char* typeName;
        size_t size;
        int alignment;
        size_t wrappedSize;
        size_t valueOffset;
        size_t suffixOffset;
        size_t holderSize;
    } cases[] = {
        {"BFloat16", 2, 2, 6, 2, 4, 20},
        {"vector<BFloat16,2>", 4, 4, 12, 4, 8, 40},
        {"vector<BFloat16,3>", 6, 2, 10, 2, 8, 32},
        {"vector<BFloat16,4>", 8, 2, 12, 2, 10, 38},
        {"half2", 4, 4, 12, 4, 8, 40},
        {"half3", 8, 4, 16, 4, 12, 52},
        {"half4", 8, 4, 16, 4, 12, 52},
        {"uint16_t2", 4, 4, 12, 4, 8, 40},
        {"uint16_t3", 6, 2, 10, 2, 8, 32},
        {"uint16_t4", 8, 8, 24, 8, 16, 80},
        {"float3", 12, 4, 20, 4, 16, 64},
        {"float4", 16, 16, 48, 16, 32, 160},
        {"matrix<BFloat16,2,4>", 16, 2, 20, 2, 18, 62},
        {"matrix<BFloat16,4,2>", 16, 4, 24, 4, 20, 76},
        {"matrix<BFloat16,3,4>", 24, 2, 28, 2, 26, 86},
        {"matrix<BFloat16,4,3>", 24, 2, 28, 2, 26, 86},
    };
    for (const auto& testCase : cases)
    {
        auto type = reflection->findTypeByName(testCase.typeName);
        SLANG_CHECK_ABORT(type != nullptr);
        auto layout = reflection->getTypeLayout(type);
        SLANG_CHECK_ABORT(layout != nullptr);
        SLANG_CHECK(layout->getSize() == testCase.size);
        SLANG_CHECK(layout->getAlignment() == testCase.alignment);
        SLANG_CHECK(layout->getStride() == testCase.size);

        String wrappedName = String("Wrapped<") + testCase.typeName + ">";
        auto wrappedType = reflection->findTypeByName(wrappedName.getBuffer());
        SLANG_CHECK_ABORT(wrappedType != nullptr);
        auto wrapped = reflection->getTypeLayout(wrappedType);
        SLANG_CHECK_ABORT(wrapped != nullptr);
        SLANG_CHECK(wrapped->getSize() == testCase.wrappedSize);
        SLANG_CHECK(wrapped->getAlignment() == testCase.alignment);
        SLANG_CHECK(wrapped->getStride() == testCase.wrappedSize);
        SLANG_CHECK_ABORT(wrapped->getFieldCount() == 3);
        SLANG_CHECK(wrapped->getFieldByIndex(0)->getOffset() == 0);
        SLANG_CHECK(wrapped->getFieldByIndex(1)->getOffset() == testCase.valueOffset);
        SLANG_CHECK(wrapped->getFieldByIndex(2)->getOffset() == testCase.suffixOffset);

        String holderName = String("Holder<") + testCase.typeName + ">";
        auto holderType = reflection->findTypeByName(holderName.getBuffer());
        SLANG_CHECK_ABORT(holderType != nullptr);
        auto holder = reflection->getTypeLayout(holderType);
        SLANG_CHECK_ABORT(holder != nullptr);
        SLANG_CHECK(holder->getSize() == testCase.holderSize);
        SLANG_CHECK(holder->getAlignment() == testCase.alignment);
        SLANG_CHECK(holder->getStride() == testCase.holderSize);
        SLANG_CHECK_ABORT(holder->getFieldCount() == 2);
        SLANG_CHECK(holder->getFieldByIndex(0)->getOffset() == 0);
        SLANG_CHECK(holder->getFieldByIndex(1)->getOffset() == 3 * testCase.wrappedSize);
    }
}
