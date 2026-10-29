// unit-test-large-matrix-row-stride-reflection.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// A matrix with more than 4 columns is laid out as an array of rows, each row an array. The row
// layout that reflection hands out for such a matrix reports the array's element stride (16 bytes
// under std140), not the scalar size.
SLANG_UNIT_TEST(largeMatrixRowStrideReflection)
{
    const char* userSourceBody = R"(
        cbuffer constants
        {
            matrix<float, 3, 6> wide;
            matrix<float, 6, 3> tall;
            float4x4 native;
        }
        struct Element
        {
            matrix<float, 3, 6> wide;
        };
        StructuredBuffer<Element> buffer;
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
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    auto reflection = module->getLayout();
    SLANG_CHECK_ABORT(reflection != nullptr);
    SLANG_CHECK_ABORT(reflection->getParameterCount() == 2);

    auto rowStride = [](slang::TypeLayoutReflection* matrixLayout)
    {
        auto rowLayout = matrixLayout->getElementTypeLayout();
        return rowLayout ? rowLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM) : 0;
    };

    // std140 constant buffer.
    auto constantsLayout =
        reflection->getParameterByIndex(0)->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(constantsLayout != nullptr);
    SLANG_CHECK_ABORT(constantsLayout->getFieldCount() == 3);

    auto wideLayout = constantsLayout->getFieldByIndex(0)->getTypeLayout();
    SLANG_CHECK(wideLayout->getKind() == slang::TypeReflection::Kind::Matrix);
    // 3 rows of float[6], every element on a 16-byte boundary.
    SLANG_CHECK(wideLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == 3 * 6 * 16);
    SLANG_CHECK(
        wideLayout->getElementTypeLayout()->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == 96);
    SLANG_CHECK(rowStride(wideLayout) == 16);

    // A row of 3 is a native vector: its elements are packed.
    auto tallLayout = constantsLayout->getFieldByIndex(1)->getTypeLayout();
    SLANG_CHECK(rowStride(tallLayout) == 4);

    auto nativeLayout = constantsLayout->getFieldByIndex(2)->getTypeLayout();
    SLANG_CHECK(rowStride(nativeLayout) == 4);

    // std430 structured buffer: no padding inside a row.
    auto elementLayout =
        reflection->getParameterByIndex(1)->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(elementLayout != nullptr);
    SLANG_CHECK_ABORT(elementLayout->getFieldCount() == 1);
    auto bufferWideLayout = elementLayout->getFieldByIndex(0)->getTypeLayout();
    SLANG_CHECK(bufferWideLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == 3 * 6 * 4);
    SLANG_CHECK(rowStride(bufferWideLayout) == 4);
}
