// unit-test-invalid-specialization-arg.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that we can report proper error when passing an invalid specialization arg to
// `IComponentType::specialize`.

SLANG_UNIT_TEST(invalidSpecializationArg)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        interface IFoo {}
        struct FooImpl : IFoo {}
        struct BarImpl {}
        float4 fragMain<int x, T:IFoo>(float4 pos:SV_Position) : SV_Target
        {
            return pos;
        }
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

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> specializedEntryPoint;
    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromExpr("1"),
        slang::SpecializationArg::fromExpr("NonExistentType")};

    // Try to specialize with an invalid specialization argument.
    auto result = entryPoint->specialize(
        specArgs,
        2,
        specializedEntryPoint.writeRef(),
        diagnosticBlob.writeRef());
    // We should return an error code without crashing.
    SLANG_CHECK(SLANG_FAILED(result));
    // Check that diagnostic blob contains some message.
    SLANG_CHECK(diagnosticBlob != nullptr && diagnosticBlob->getBufferPointer() != nullptr);

    diagnosticBlob = nullptr;

    // Specialize with not sufficient specialization arguments should result in an error.
    slang::SpecializationArg specArgs1[] = {slang::SpecializationArg::fromExpr("1")};
    result = entryPoint->specialize(
        specArgs1,
        1,
        specializedEntryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(diagnosticBlob != nullptr && diagnosticBlob->getBufferPointer() != nullptr);

    diagnosticBlob = nullptr;

    // Specialize with a valid type, but doesn't conform to the constraint.
    slang::SpecializationArg specArgs2[] = {
        slang::SpecializationArg::fromExpr("1"),
        slang::SpecializationArg::fromExpr("BarImpl")};
    result = entryPoint->specialize(
        specArgs2,
        2,
        specializedEntryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(diagnosticBlob != nullptr && diagnosticBlob->getBufferPointer() != nullptr);

    // Finally, specialize with a valid type that conforms to the constraint.
    diagnosticBlob = nullptr;
    slang::SpecializationArg specArgs3[] = {
        slang::SpecializationArg::fromExpr("1"),
        slang::SpecializationArg::fromExpr("FooImpl")};
    result = entryPoint->specialize(
        specArgs3,
        2,
        specializedEntryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(specializedEntryPoint != nullptr);
    SLANG_CHECK(diagnosticBlob == nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(specializedEntryPoint->link(linkedProgram.writeRef())));

    // Check the specialized entrypoint can compile.
    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);
}
