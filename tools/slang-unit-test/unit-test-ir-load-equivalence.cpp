// unit-test-ir-load-equivalence.cpp

#include "core/slang-io.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "scoped-env-var.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using SlangUnitTest::ScopedEnvVar;
using SlangUnitTest::writeEnvironmentVariable;

namespace
{

struct RunResult
{
    int32_t exitCode = -1;
    String out;
    String err;
};

/// Runs `slangc` with `args`, with on-demand IR loading forced to `onDemand`.
///
/// Runs a child process rather than compiling in-process because the load mode is read
/// once per process: a test that tried to switch it in-process would measure whichever
/// mode happened to be read first.
SlangResult _runSlangc(
    UnitTestContext* unitTestContext,
    const List<String>& args,
    bool onDemand,
    RunResult& out)
{
    const UnownedStringSlice varName("SLANG_ONDEMAND_IR");
    const char* const value = onDemand ? "1" : "0";
    if (writeEnvironmentVariable("SLANG_ONDEMAND_IR", value) != 0)
        return SLANG_FAIL;

    // Read it back before spawning. A setter that reported success without taking
    // effect would leave both children in the same mode, and this test would compare a
    // run against itself and pass -- green while checking nothing. The child inherits
    // the environment at creation, so confirming it here is enough.
    StringBuilder readBack;
    SLANG_RETURN_ON_FAIL(PlatformUtil::getEnvironmentVariable(varName, readBack));
    if (readBack.produceString() != value)
        return SLANG_FAIL;

    CommandLine cmdLine;
    cmdLine.setExecutableLocation(
        ExecutableLocation(unitTestContext->executableDirectory, "slangc"));
    for (const auto& arg : args)
        cmdLine.addArg(arg);

    ExecuteResult exeRes;
    SLANG_RETURN_ON_FAIL(ProcessUtil::execute(cmdLine, exeRes));
    out.exitCode = exeRes.resultCode;
    out.out = exeRes.standardOutput;
    out.err = exeRes.standardError;
    return SLANG_OK;
}

/// Compiles `sourcePath` to HLSL, optionally dumping the linked IR.
///
/// `includeDir`, when non-empty, is where `import` looks for a precompiled module.
///
/// Note what is *not* passed when `dumpIR` is false: no `-o`. The generated code has to
/// reach stdout for the comparison to mean anything -- directing it to a file leaves two
/// empty strings, which compare equal.
SlangResult _compileWith(
    UnitTestContext* unitTestContext,
    const String& sourcePath,
    const String& irSinkPath,
    bool onDemand,
    bool dumpIR,
    RunResult& out,
    const String& includeDir = String())
{
    List<String> args;
    args.add(sourcePath);
    if (includeDir.getLength())
    {
        args.add("-I");
        args.add(includeDir);
    }
    args.add("-target");
    args.add("hlsl");
    args.add("-entry");
    args.add("computeMain");
    args.add("-stage");
    args.add("compute");
    if (dumpIR)
    {
        // Dumps the linked IR -- where a divergence shows up even when it never reaches
        // codegen.
        //
        // `-o` must name a real file rather than a null device, because there is no
        // spelling of one that works on every platform this runs on. A failing `-o` would
        // fail both runs identically and leave the comparison matching two error
        // messages. The file itself is never read; only the IR dump on stderr is.
        args.add("-dump-ir");
        args.add("-o");
        args.add(irSinkPath);
    }
    return _runSlangc(unitTestContext, args, onDemand, out);
}

/// Precompiles `sourcePath` into a `.slang-module` at `modulePath`.
///
/// Built once rather than per mode. The variable under test is the *load* path; a module
/// whose serialized bytes depended on the writer's load mode would be a different bug,
/// and building it twice would fold that question into this comparison.
SlangResult _buildLibraryModule(
    UnitTestContext* unitTestContext,
    const String& sourcePath,
    const String& modulePath,
    RunResult& out)
{
    List<String> args;
    args.add(sourcePath);
    args.add("-target");
    args.add("hlsl");
    args.add("-o");
    args.add(modulePath);
    args.add("-emit-ir");
    return _runSlangc(unitTestContext, args, true, out);
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
    // Saves the caller's setting and puts it back when this scope ends, so the test does
    // not leak a mode into the rest of the process -- the environment is shared with every
    // other test running here. `_runSlangc` overwrites the value per child; this only has
    // to own the restore.
    ScopedEnvVar modeGuard("SLANG_ONDEMAND_IR", "1");

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

    const String workDir = Path::getParentDirectory(unitTestContext->executableDirectory);
    const String irSinkPath = Path::combine(workDir, "ir-load-equivalence-dump-sink.hlsl");

    // A precompiled *user* module, which takes the same deferred path as the builtin
    // ones -- `Linkage::loadSerializedModuleContents` passes a retained blob too. The
    // shapes here are the ones the deferral invariant is least obviously true for:
    // interfaces with generic methods, two conformances, witness tables, and a generic
    // value parameter, so the library carries globals nested inside generic bodies.
    //
    // The file name has to match the module name for `import` to find it.
    const String libSourcePath = Path::combine(workDir, "irLoadEquivalenceLib.slang");
    const String libModulePath = Path::combine(workDir, "irLoadEquivalenceLib.slang-module");
    const String userSourcePath = Path::combine(workDir, "irLoadEquivalenceUser.slang");
    {
        const char* libSource = R"(
module irLoadEquivalenceLib;
public interface IShape { float area(); float scaledBy<let N : int>(float k); }
public struct Circle : IShape
{
    public float r;
    public float area() { return 3.14159f * r * r; }
    public float scaledBy<let N : int>(float k) { return area() * k * float(N); }
}
public struct Box : IShape
{
    public float w; public float h;
    public float area() { return w * h; }
    public float scaledBy<let N : int>(float k) { return area() * k * float(N); }
}
public float totalArea<T : IShape>(T s, float k) { return s.scaledBy<3>(k); }
)";
        const char* userSource = R"(
import irLoadEquivalenceLib;
RWStructuredBuffer<float> gOut;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Circle c; c.r = 2.0f;
    Box b; b.w = 3.0f; b.h = 4.0f;
    gOut[tid.x] = totalArea(c, 1.5f) + totalArea(b, 2.0f);
}
)";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(libSourcePath, libSource)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(userSourcePath, userSource)));
    }

    RunResult onDemand;
    RunResult eager;
    RunResult onDemandIR;
    RunResult eagerIR;
    RunResult libBuild;
    RunResult modOnDemand;
    RunResult modEager;
    RunResult modOnDemandIR;
    RunResult modEagerIR;
    const bool ranBoth =
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, true, false, onDemand)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, false, false, eager)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, true, true, onDemandIR)) &&
        SLANG_SUCCEEDED(
            _compileWith(unitTestContext, sourcePath, irSinkPath, false, true, eagerIR)) &&
        SLANG_SUCCEEDED(
            _buildLibraryModule(unitTestContext, libSourcePath, libModulePath, libBuild)) &&
        SLANG_SUCCEEDED(_compileWith(
            unitTestContext,
            userSourcePath,
            irSinkPath,
            true,
            false,
            modOnDemand,
            workDir)) &&
        SLANG_SUCCEEDED(_compileWith(
            unitTestContext,
            userSourcePath,
            irSinkPath,
            false,
            false,
            modEager,
            workDir)) &&
        SLANG_SUCCEEDED(_compileWith(
            unitTestContext,
            userSourcePath,
            irSinkPath,
            true,
            true,
            modOnDemandIR,
            workDir)) &&
        SLANG_SUCCEEDED(_compileWith(
            unitTestContext,
            userSourcePath,
            irSinkPath,
            false,
            true,
            modEagerIR,
            workDir));

    // `modeGuard` restores the caller's setting when this scope ends, which is after the
    // assertions below -- a failing assertion does not skip it.
    File::remove(sourcePath);
    File::remove(irSinkPath);
    File::remove(libSourcePath);
    File::remove(libModulePath);
    File::remove(userSourcePath);

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

    // The same comparison for a precompiled *user* module. Deferral is not builtin-only:
    // `loadSerializedModuleContents` hands the reader a retained blob, so a
    // `.slang-module` defers on the same terms. The invariant deferral rests on -- that
    // nothing below module scope references another global's body -- was measured over
    // the builtin modules, so a user module built by the real front end is the case that
    // measurement did not cover. A violation aborts the compile on a release assert in
    // `readInstRef`, which is why the exit codes are checked and not only the output.
    //
    // Limit worth knowing: this compares modes, it cannot confirm the user module was
    // deferred. If deferral ever declined for `.slang-module` files, both runs would be
    // eager and agree trivially. The loader is not observable from a child process;
    // `irDeferralDeclinesWhenTheBlobDoesNotBackTheSpans` covers that decision directly,
    // on a module round-tripped in-process.
    SLANG_CHECK(libBuild.exitCode == 0);
    SLANG_CHECK(modOnDemand.exitCode == 0);
    SLANG_CHECK(modOnDemand.exitCode == modEager.exitCode);
    SLANG_CHECK(modOnDemand.out.getLength() > 0);
    SLANG_CHECK(modOnDemand.out == modEager.out);
    SLANG_CHECK(modOnDemand.err == modEager.err);

    SLANG_CHECK(modOnDemandIR.exitCode == 0);
    SLANG_CHECK(modOnDemandIR.exitCode == modEagerIR.exitCode);
    SLANG_CHECK(modOnDemandIR.err.getLength() > 0);
    SLANG_CHECK(modOnDemandIR.err == modEagerIR.err);
}
