// unit-test-ir-load-equivalence.cpp

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

struct RunResult
{
    int32_t exitCode = -1;
    String out;
    String err;
};

/// Compiles `source` with `slangc`, with on-demand IR loading forced to `onDemand`.
///
/// Runs a child process rather than compiling in-process because the load mode is read
/// once per process: a test that tried to switch it in-process would measure whichever
/// mode happened to be read first.
SlangResult _compileWith(
    UnitTestContext* unitTestContext,
    const String& sourcePath,
    const String& irSinkPath,
    bool onDemand,
    bool dumpIR,
    RunResult& out)
{
    const UnownedStringSlice varName("SLANG_ONDEMAND_IR");
    const UnownedStringSlice value(onDemand ? "1" : "0");
    SLANG_RETURN_ON_FAIL(PlatformUtil::setEnvironmentVariable(varName, &value));

    // Read it back before spawning. A setter that reported success without taking
    // effect would leave both children in the same mode, and this test would compare a
    // run against itself and pass -- green while checking nothing. The child inherits
    // the environment at creation, so confirming it here is enough.
    StringBuilder readBack;
    SLANG_RETURN_ON_FAIL(PlatformUtil::getEnvironmentVariable(varName, readBack));
    if (readBack.produceString().getUnownedSlice() != value)
        return SLANG_FAIL;

    CommandLine cmdLine;
    cmdLine.setExecutableLocation(
        ExecutableLocation(unitTestContext->executableDirectory, "slangc"));
    cmdLine.addArg(sourcePath);
    cmdLine.addArg("-target");
    cmdLine.addArg("hlsl");
    cmdLine.addArg("-entry");
    cmdLine.addArg("computeMain");
    cmdLine.addArg("-stage");
    cmdLine.addArg("compute");
    if (dumpIR)
    {
        // Dumps the linked IR, builtin modules included -- which is where deferral
        // acts, and where a divergence shows up even when it never reaches codegen.
        //
        // `-o` goes to a real file rather than a null device. `/dev/null` was hardcoded
        // here, which is not a path on Windows: `-o` would fail, both runs would fail
        // identically, and the comparison below would pass having compared two error
        // messages. A temp file sidesteps the platform question entirely -- the file is
        // never read, only the IR dump on stdout is.
        cmdLine.addArg("-dump-ir");
        cmdLine.addArg("-o");
        cmdLine.addArg(irSinkPath);
    }

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));
    out.exitCode = exeRes.resultCode;
    out.out = exeRes.standardOutput;
    out.err = exeRes.standardError;
    return SLANG_OK;
}

} // namespace

// Checks that on-demand and eager IR loading produce the same result.
//
// This is the premise the whole change rests on -- the two paths decode the same bytes
// and must agree -- and it is the one property no other test covers, because the mode is
// process-global and every suite run picks a single mode. A divergence here is silent by
// nature: the decoration-subtree bug found during review produced no diagnostic, just a
// global value that had lost its children.
//
// Deliberately narrow: one shader, output and exit status compared. Breadth across the
// whole test corpus needs a bulk sweep -- two compiles per shader over thousands of
// shaders -- which is a development aid rather than something to run on every build, and
// is proposed separately in shader-slang/slang#12704.
SLANG_UNIT_TEST(irLoadEquivalence)
{
    // Remember the caller's setting, so this test does not leak a mode into the rest of
    // the process -- the environment is shared with every other test running here.
    StringBuilder previous;
    const bool hadPrevious = SLANG_SUCCEEDED(
        PlatformUtil::getEnvironmentVariable(UnownedStringSlice("SLANG_ONDEMAND_IR"), previous));

    String sourcePath;
    {
        // A shader that reaches a reasonable slice of the core module: generics, matrix
        // math, a resource, and a call through an interface constraint.
        const char* source = R"(
interface IScale { float apply(float v); }
struct Doubler : IScale { float apply(float v) { return v * 2.0f; } }
float scaleAll<T : IScale>(T s, float v) { return s.apply(v); }

RWStructuredBuffer<float> gOut;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = float4x4(1.0f);
    float3 v = normalize(float3(1.0f, 2.0f, 3.0f));
    Doubler d;
    gOut[tid.x] = scaleAll(d, dot(v, mul(m, float4(v, 1.0f)).xyz));
}
)";
        sourcePath = Path::combine(
            Path::getParentDirectory(unitTestContext->executableDirectory),
            "ir-load-equivalence-test.slang");
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(sourcePath, source)));
    }

    const String irSinkPath = Path::combine(
        Path::getParentDirectory(unitTestContext->executableDirectory),
        "ir-load-equivalence-dump-sink.hlsl");

    RunResult onDemand;
    RunResult eager;
    RunResult onDemandIR;
    RunResult eagerIR;
    const bool ranBoth =
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, true, false, onDemand)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, false, false, eager)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, true, true, onDemandIR)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, false, true, eagerIR));

    // Restore before asserting, so a failure does not also corrupt later tests.
    if (hadPrevious)
    {
        const String previousText = previous.produceString();
        const UnownedStringSlice previousSlice = previousText.getUnownedSlice();
        PlatformUtil::setEnvironmentVariable(
            UnownedStringSlice("SLANG_ONDEMAND_IR"),
            &previousSlice);
    }
    else
    {
        PlatformUtil::setEnvironmentVariable(UnownedStringSlice("SLANG_ONDEMAND_IR"), nullptr);
    }
    File::remove(sourcePath);
    File::remove(irSinkPath);

    SLANG_CHECK_ABORT(ranBoth);

    // The compile is expected to succeed; a shared failure would make the comparison
    // below pass while proving nothing.
    SLANG_CHECK(onDemand.exitCode == 0);
    SLANG_CHECK(onDemand.exitCode == eager.exitCode);
    // Non-empty as well as equal: a compile that succeeded while emitting nothing would
    // otherwise compare two empty strings, which is how the IR arm below sat vacuous.
    SLANG_CHECK(onDemand.out.getLength() > 0);
    SLANG_CHECK(onDemand.out == eager.out);
    SLANG_CHECK(onDemand.err == eager.err);

    // Compares the IR too, because a decode divergence need not reach codegen: a global
    // value that lost children can emit identical target code. `-dump-ir` writes to
    // *stderr*, which is what is compared -- this arm previously compared stdout and so
    // matched two empty strings, which is why the length assertion is here.
    //
    // Known limit: this cannot catch the decoration-subtree bug, since no decoration in
    // the builtin modules has children. `irDeferredBodyKeepsDecorationChildren` covers
    // that by building the shape; breadth would come from a corpus-wide sweep
    // (shader-slang/slang#12704), which is too slow to run per build.
    SLANG_CHECK(onDemandIR.exitCode == 0);
    SLANG_CHECK(onDemandIR.exitCode == eagerIR.exitCode);
    SLANG_CHECK(onDemandIR.err.getLength() > 0);
    SLANG_CHECK(onDemandIR.err == eagerIR.err);
}
