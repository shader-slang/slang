// unit-test-generic-entrypoint.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test the compilation API for compiling a specialized generic entrypoint.

SLANG_UNIT_TEST(genericEntryPointCompile)
{
    const char* userSourceBody = R"(
            interface I { int getValue(); }
            struct X : I { int getValue() { return 100; } }
            float4 vertMain<T:I, int n, each U>(uniform T o) {
                return float4(o.getValue(), countof(U), n, 1);
            }
        )";
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

    // Test 1: Using findAndCheckEntryPoint to supply arguments in string form.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain<X, 7, int, float>",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 2.0, 7.0, 1.0)")) != -1);
    }

    // Test 2: Using `specialize` to supply arguments structurally with reflection types.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        ComPtr<slang::IComponentType> specializedEntryPoint;
        slang::SpecializationArg args[] = {
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("X")),
            slang::SpecializationArg::fromExpr("8"),
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("int")),
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("float"))};

        entryPoint->specialize(args, 4, specializedEntryPoint.writeRef(), nullptr);
        SLANG_CHECK_ABORT(specializedEntryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, specializedEntryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 2.0, 8.0, 1.0)")) != -1);
    }

    // Test 3: corner case: specialize variadic param with 0 types.
    {
        ComPtr<slang::IEntryPoint> entryPoint;
        module->findAndCheckEntryPoint(
            "vertMain",
            SLANG_STAGE_VERTEX,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(entryPoint != nullptr);
        ComPtr<slang::IComponentType> specializedEntryPoint;
        slang::SpecializationArg args[] = {
            slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("X")),
            slang::SpecializationArg::fromExpr("8")};

        entryPoint->specialize(args, 2, specializedEntryPoint.writeRef(), nullptr);
        SLANG_CHECK_ABORT(specializedEntryPoint != nullptr);
        slang::IComponentType* componentTypes[2] = {module, specializedEntryPoint.get()};
        ComPtr<slang::IComponentType> composedProgram;
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        ComPtr<slang::IComponentType> linkedProgram;
        composedProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

        ComPtr<slang::IBlob> code;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(
            UnownedStringSlice((char*)code->getBufferPointer())
                .indexOf(toSlice("vec4(float(X_getValue_0()), 0.0, 8.0, 1.0)")) != -1);
    }
}
