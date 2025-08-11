// unit-test-unit-test-conditional-vertex-input.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test the compilation API for compiling an entrypoint that uses `Conditional<T>` to
// represent conditional vertex attribute input that can be specialized away.

SLANG_UNIT_TEST(conditionalVertexInput)
{
    const char* userSourceBody = R"(
            struct Vertex<bool hasColor> {
                float3 pos : POSITION;
                Conditional<float3, hasColor> color : COLOR;
                float3 normal : NORMAL;
            }

            extern static const bool vertexHasColor = true;

            [shader("vertex")]
            float4 vertMain(Vertex<vertexHasColor> o) {
                if (let color = o.color.get())
                {
                    // If `vertexHasColor` is true, we can use `color`.
                    return float4(o.pos + color + o.normal, 1);
                }
                return float4(o.pos + o.normal, 1);
            }
        )";
    const char* userSourceBodyNoColor = R"(export static const bool vertexHasColor = false;)";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_GLSL;
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

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "vertMain",
        SLANG_STAGE_VERTEX,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());

    // Check the program with `vertexHasColor = true`.
    {
        slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        auto paramLayout = linkedProgram->getLayout()
                               ->getEntryPointByIndex(0)
                               ->getParameterByIndex(0)
                               ->getTypeLayout();

        // Total number of varying inputs should be 3. (pos, color and normal)
        SLANG_CHECK(paramLayout->getSize(slang::ParameterCategory::VaryingInput) == 3);

        // Offset of `normal` should be 2.
        SLANG_CHECK(
            paramLayout
                ->getFieldByIndex(2) // `o.normal`
                ->getOffset(slang::ParameterCategory::VaryingInput) == 2);
        ComPtr<slang::IBlob> code;
        SLANG_CHECK(
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef()) ==
            SLANG_OK);
        auto codeStr = Slang::UnownedStringSlice((const char*)code->getBufferPointer());
        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 0)")) != -1);
        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 1)")) != -1);
        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 2)")) != -1);
    }

    // Check the program with `vertexHashColor = false`.
    {
        auto configModule = session->loadModuleFromSourceString(
            "config",
            "config.slang",
            userSourceBodyNoColor,
            diagnosticBlob.writeRef());
        SLANG_CHECK(module != nullptr);

        slang::IComponentType* componentTypes[3] = {module, entryPoint.get(), configModule};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            3,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        auto paramLayout = linkedProgram->getLayout()
                               ->getEntryPointByIndex(0)
                               ->getParameterByIndex(0)
                               ->getTypeLayout();

        // Total number of varying inputs should be 2. (pos and normal)
        SLANG_CHECK(paramLayout->getSize(slang::ParameterCategory::VaryingInput) == 2);

        // Offset of `normal` should be 1.
        SLANG_CHECK(
            paramLayout
                ->getFieldByIndex(2) // `o.normal`
                ->getOffset(slang::ParameterCategory::VaryingInput) == 1);
        ComPtr<slang::IBlob> code;
        SLANG_CHECK(
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef()) ==
            SLANG_OK);

        auto codeStr = Slang::UnownedStringSlice((const char*)code->getBufferPointer());

        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 0)")) != -1);
        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 1)")) != -1);
        // Resulting code should not contain `layout(location = 1)` since `color` is not used.
        SLANG_CHECK(codeStr.indexOf(toSlice("layout(location = 2)")) == -1);
    }
}
