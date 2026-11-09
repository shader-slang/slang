#include "slang-hgvm/slang-hgvm-compiler.h"
#include "slang-hgvm/slang-hgvm.h"

#include <cstdio>

using namespace Slang;
using namespace Slang::HGVM;

static int reportFailure(const char* operation, const String& error)
{
    std::fprintf(stderr, "%s: %s\n", operation, error.getBuffer());
    return 1;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::fprintf(stderr, "Usage: %s <hello-world.slang>\n", argv[0]);
        return 1;
    }
    const char* sourcePath = argv[1];

    // Keep the compiler session alive when compiling additional modules with this compiler.
    ComPtr<slang::IGlobalSession> globalSession;
    if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())))
        return reportFailure("Create session", "Cannot create the Slang compiler session");

    HGBCCompiler compiler(globalSession);
    ComPtr<slang::IBlob> code;
    String diagnostics;
    SlangResult compileResult = compiler.compile(
        sourcePath,
        "main", // Pass nullptr instead to compile all concrete [host] entry points.
        nullptr,
        0, // No additional import search paths; the source directory is included automatically.
        SLANG_DEBUG_INFO_LEVEL_NONE,
        SLANG_DIAGNOSTIC_COLOR_NEVER,
        code,
        diagnostics);
    // Compilation can succeed with warnings, so report diagnostics on either path.
    if (diagnostics.getLength())
        std::fprintf(stderr, "%s\n", diagnostics.getBuffer());
    if (SLANG_FAILED(compileResult))
        return reportFailure("Compile", "Could not compile the [host] main entry point");

    // Program owns the validated bytecode. It can outlive the compiler and the input blob.
    String error;
    RefPtr<Program> program;
    if (SLANG_FAILED(Program::load(
            code->getBufferPointer(),
            code->getBufferSize(),
            sourcePath,
            program,
            error)))
        return reportFailure("Load HGBC", error);

    // A default interpreter needs no GPU device for this program. Slang print() uses stdout.
    Interpreter vm;
    const auto* mainFunction = program->findFunctionExport("main");
    if (!mainFunction)
        return reportFailure("Find entry point", "The program does not export main");

    // Binding stages the program and resolves its signature once. Reuse the binding for more calls.
    RefPtr<BoundFunction> main;
    if (SLANG_FAILED(vm.bindFunction(program, mainFunction->id, main, error)))
        return reportFailure("Bind main", error);

    // This is the same invocation API used by Python and slangi. main takes no arguments and
    // returns void; functions with parameters use Argument, and non-void results live in Value.
    Value result;
    if (SLANG_FAILED(vm.invoke(main, {}, result, error)))
        return reportFailure("Invoke main", error);

    // Destruction also closes the VM. Explicit close reports shutdown errors and invalidates
    // surviving function/value handles immediately.
    if (SLANG_FAILED(vm.close(error)))
        return reportFailure("Close VM", error);
    return 0;
}
