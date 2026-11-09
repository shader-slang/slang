#include "core/slang-memory-file-system.h"
#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-compiler.h"

using namespace Slang;
using namespace Slang::HGVM;

SLANG_UNIT_TEST(hgvmCompilerVirtualFileSystem)
{
    // Neither source exists on the OS filesystem. Import resolution and source canonicalization
    // must both use the supplied filesystem, as they do for fetched browser modules.
    ComPtr<MemoryFileSystem> files(new MemoryFileSystem);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(files->createDirectory("app")));
    const char* helper = "public int twice(int x) { return x * 2; }";
    const char* main = "import helper; [host] int main(int x) { return twice(x); }";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(files->saveFile("app/helper.slang", helper, strlen(helper))));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(files->saveFile("app/main.slang", main, strlen(main))));

    HGBCCompiler compiler(unitTestContext->slangGlobalSession, files);
    ComPtr<slang::IBlob> code;
    String error;
    SlangResult status = compiler.compile(
        "app/./main.slang",
        nullptr,
        nullptr,
        0,
        SLANG_DEBUG_INFO_LEVEL_NONE,
        SLANG_DIAGNOSTIC_COLOR_NEVER,
        code,
        error);
    if (SLANG_FAILED(status))
        fprintf(stderr, "%s\n", error.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(status));

    RefPtr<Program> program;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        Program::load(code->getBufferPointer(), code->getBufferSize(), "virtual", program, error)));
    Interpreter vm;
    uint64_t stagedId = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(vm.prepareProgram(program, stagedId, error)));
    RefPtr<BoundFunction> function;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    Argument argument = Argument::scalar(int32_t(21));
    Value result;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*function)(makeArrayView(&argument, 1), result, error)));
    int32_t number = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(result.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
    SLANG_CHECK(number == 42);
}
