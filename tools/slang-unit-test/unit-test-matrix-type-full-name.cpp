// unit-test-matrix-type-full-name.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

SLANG_UNIT_TEST(matrixTypeFullName)
{
    const char* source = R"(
        struct S
        {
            row_major float3x3 rowMajor;
            column_major float2x4 columnMajor;
            float3x3 unqualified;
        }
        )";

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", source, diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    auto structType = module->getLayout()->findTypeByName("S");
    SLANG_CHECK_ABORT(structType != nullptr);
    SLANG_CHECK_ABORT(structType->getFieldCount() == 3);

    SLANG_CHECK(
        getTypeFullName(structType->getFieldByIndex(0)->getType()) ==
        "matrix<float,3,3,MatrixLayoutMode.RowMajor>");
    SLANG_CHECK(
        getTypeFullName(structType->getFieldByIndex(1)->getType()) ==
        "matrix<float,2,4,MatrixLayoutMode.ColumnMajor>");
    SLANG_CHECK(getTypeFullName(structType->getFieldByIndex(2)->getType()) == "matrix<float,3,3>");
}
