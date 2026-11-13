// unit-test-matrix-layout-type-name.cpp

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

// Test that the reflected full name of a matrix type includes its layout when the layout is
// specified, and keeps the `matrix<T,R,C>` spelling when it is not (shader-slang/slang#13383).

SLANG_UNIT_TEST(matrixLayoutTypeName)
{
    const char* userSourceBody = R"(
        struct S
        {
            row_major float2x3 rowMajor;
            float2x3 unspecified;
            column_major float2x3 columnMajor;
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
    SLANG_CHECK_ABORT(module != nullptr);

    auto type = module->getLayout()->findTypeByName("S");
    SLANG_CHECK_ABORT(type != nullptr);
    SLANG_CHECK_ABORT(type->getFieldCount() == 3);
    SLANG_CHECK(
        getTypeFullName(type->getFieldByIndex(0)->getType()) ==
        "matrix<float,2,3,MatrixLayoutMode.RowMajor>");
    SLANG_CHECK(getTypeFullName(type->getFieldByIndex(1)->getType()) == "matrix<float,2,3>");
    SLANG_CHECK(
        getTypeFullName(type->getFieldByIndex(2)->getType()) ==
        "matrix<float,2,3,MatrixLayoutMode.ColumnMajor>");
}
