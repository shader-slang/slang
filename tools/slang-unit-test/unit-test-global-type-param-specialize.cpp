// unit-test-global-type-param-specialize.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// Positive companion to the E38207 diagnostic from #11316: a module-scope
// `type_param T : IFoo` that *is* given a concrete binding via
// `IComponentType::specialize` must compile cleanly (no E38207) and produce
// code. This exercises the success path of `specializeGlobalGenericParameters`
// — the loop that the diagnostic change touches — which is otherwise only
// covered by GPU/disabled compute tests.
SLANG_UNIT_TEST(globalTypeParamSpecialize)
{
    const char* userSourceBody = R"(
        interface IFoo { int get(); }
        struct FooImpl : IFoo { int get() { return 42; } }

        // Module-scope generic parameter (global `type_param`).
        type_param T : IFoo;

        RWStructuredBuffer<int> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            T t;
            outputBuffer[0] = t.get();
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
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entryPoint.writeRef())));

    // Compose the module + entry point so the program carries the module-scope
    // `type_param` as a specialization parameter.
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(session->createCompositeComponentType(components, 2, program.writeRef())));

    // The global `type_param T` shows up as a specialization parameter.
    SLANG_CHECK(program->getSpecializationParamCount() == 1);

    // Bind it to a conforming type and confirm specialization succeeds.
    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromType(program->getLayout()->findTypeByName("FooImpl"))};
    ComPtr<slang::IComponentType> specialized;
    diagnosticBlob = nullptr;
    auto result =
        program->specialize(specArgs, 1, specialized.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK_ABORT(specialized != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(specialized->link(linkedProgram.writeRef())));

    // Code generation must succeed and emit no E38207 diagnostic.
    ComPtr<slang::IBlob> code;
    diagnosticBlob = nullptr;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(code != nullptr && code->getBufferSize() != 0);
    if (diagnosticBlob && diagnosticBlob->getBufferPointer())
    {
        const char* diagnostics = (const char*)diagnosticBlob->getBufferPointer();
        SLANG_CHECK(strstr(diagnostics, "38207") == nullptr);
    }
}
