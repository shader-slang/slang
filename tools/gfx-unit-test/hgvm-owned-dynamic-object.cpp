#include "core/slang-io.h"
#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
struct DynamicObjectOutput : HGVM::IHGVMOutputSink
{
    String text;

    void writeLine(std::string_view line) override
    {
        text.append(line.data(), Index(line.size()));
        text.append("\n");
    }
};
} // namespace

SLANG_UNIT_TEST(hgvmOwnedDynamicObject)
{
    // Compile the source regression through the real component API: the public IDs are chosen
    // explicitly, so execution never depends on the compiler's implicit conformance ordering.
    const char* sourcePath = "tests/compute/hgvm/owned-dynamic-object.slang";
    String source;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::readAllText(sourcePath, source)));

    slang::TargetDesc target = {};
    target.format = SLANG_HGBC;
    slang::CompilerOptionEntry option = {};
    option.name = slang::CompilerOptionName::ExperimentalFeature;
    option.value.kind = slang::CompilerOptionValueKind::Int;
    option.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &target;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = &option;
    sessionDesc.compilerOptionEntryCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "owned-dynamic-object",
        sourcePath,
        source.getBuffer(),
        diagnostics.writeRef());
    if (diagnostics)
        fprintf(stderr, "%s", static_cast<const char*>(diagnostics->getBufferPointer()));
    SLANG_CHECK_ABORT(module);
    auto layout = module->getLayout();
    SLANG_CHECK_ABORT(layout);
    auto interfaceType = layout->findTypeByName("IValue");
    auto firstType = layout->findTypeByName("First");
    auto secondType = layout->findTypeByName("Second");
    SLANG_CHECK_ABORT(interfaceType && firstType && secondType);
    ComPtr<slang::ITypeConformance> first;
    ComPtr<slang::ITypeConformance> second;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createTypeConformanceComponentType(
        firstType,
        interfaceType,
        first.writeRef(),
        0,
        diagnostics.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createTypeConformanceComponentType(
        secondType,
        interfaceType,
        second.writeRef(),
        7,
        diagnostics.writeRef())));

    struct TestCase
    {
        const char* entryPoint;
        const char* expectedOutput;
        const char* expectedError;
    };
    TestCase cases[] = {
        {"main", "41\n52\n", nullptr},
        {"invalidID", "", "HGVM tagged union construction uses an invalid case"},
        {"wrongType", "", "HGVM boxed payload has a different concrete type"},
    };
    for (const auto& test : cases)
    {
        ComPtr<slang::IEntryPoint> entry;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(module->findEntryPointByName(test.entryPoint, entry.writeRef())));
        slang::IComponentType* components[] = {module, entry, first, second};
        ComPtr<slang::IComponentType> composite;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            composite.writeRef(),
            diagnostics.writeRef())));
        ComPtr<slang::IComponentType> linked;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnostics.writeRef())));
        ComPtr<slang::IBlob> code;
        auto compileResult = linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
        if (diagnostics)
            fprintf(stderr, "%s", static_cast<const char*>(diagnostics->getBufferPointer()));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));

        RefPtr<HGVM::Program> program;
        String error;
        auto loadResult = HGVM::Program::load(
            code->getBufferPointer(),
            code->getBufferSize(),
            sourcePath,
            program,
            error);
        if (SLANG_FAILED(loadResult))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadResult));
        RefPtr<DynamicObjectOutput> output = new DynamicObjectOutput;
        HGVM::Desc desc;
        desc.outputSink = output;
        HGVM::Interpreter interpreter(desc);
        auto result = interpreter.call(program, test.entryPoint, error);
        if (test.expectedError)
        {
            SLANG_CHECK(SLANG_FAILED(result));
            if (!error.contains(test.expectedError))
                fprintf(stderr, "%s\n", error.getBuffer());
            SLANG_CHECK(error.contains(test.expectedError));
        }
        else
        {
            if (SLANG_FAILED(result))
                fprintf(stderr, "%s\n", error.getBuffer());
            SLANG_CHECK(SLANG_SUCCEEDED(result));
        }
        SLANG_CHECK(output->text == test.expectedOutput);
    }
}
