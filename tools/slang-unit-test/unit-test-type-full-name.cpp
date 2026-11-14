// unit-test-type-full-name.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(type->getFullName(blob.writeRef())));
    return String((const char*)blob->getBufferPointer());
}

// `TypeReflection::getFullName` can run on a thread that has not set the current AST builder.
// Printing `vector<float,3>` creates decl-refs for its element type, so
// `spReflectionType_GetFullName` must set the AST builder of the type itself. Sanitizer builds
// report a null `ASTBuilder` here.
SLANG_UNIT_TEST(typeFullName)
{
    const char* source = R"(
        struct S
        {
            float3 v;
            float2x2 m;
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
    SLANG_CHECK_ABORT(structType->getFieldCount() == 2);

    SLANG_CHECK(getTypeFullName(structType) == "S");
    SLANG_CHECK(getTypeFullName(structType->getFieldByIndex(0)->getType()) == "vector<float,3>");
    SLANG_CHECK(getTypeFullName(structType->getFieldByIndex(1)->getType()) == "matrix<float,2,2>");
}
