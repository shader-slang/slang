// Regression test for issue #13795.
//
// Splitting debug info out of a SPIR-V module is done by Slang's own word filter
// (`stripDbgSpirvFromArtifact`), so `-separate-debug-info` must not need the downstream
// optimizer from `slang-glslang`. `-spirv-opt-path` points at a directory that does not exist,
// so any required load of that optimizer reports E00100.
//
// At `-O0` nothing needs the optimizer, so the split compile must succeed without it. At `-O1` the
// optimizer is required, so the compile must fail with E00100 and must not reach the internal
// error (E99997) that a missing debug module raises. A segfault and a clean failure both report
// result code -1 here, so these CLI checks cannot tell them apart; the in-process unit test
// `separateDebugWithMissingSpirvOptMatchesNonSplitCompile` is what catches a crash.
//
// The `BAD_PASS` case runs with the optimizer present and an invalid `-Xspirv-opt` pass, so the
// optimizer fails. With separate debug info, that error must be reported rather than lost, and
// the compile must not report the misleading E00113 ("did not produce separate debug
// information").
//
// `COMMAND_LINE_SIMPLE` runs a fresh `slangc` process, so the bogus path cannot leak into the
// global session that slang-test shares between tests. `-skip-spirv-validation` keeps CI's
// `SLANG_RUN_SPIRV_VALIDATION=1` from loading the library for validation, which would make the
// `-O0` case depend on it again.

//TEST:COMMAND_LINE_SIMPLE(filecheck=O0):-target spirv -O0 -g2 -skip-spirv-validation -separate-debug-info -separate-debug-info-output separate-debug-without-spirv-opt-O0.dbg.spv -o separate-debug-without-spirv-opt-O0.spv -spirv-opt-path separate-debug-without-spirv-opt-missing-dir
//TEST:COMMAND_LINE_SIMPLE(filecheck=O1):-target spirv -O1 -g2 -skip-spirv-validation -separate-debug-info -separate-debug-info-output separate-debug-without-spirv-opt-O1.dbg.spv -o separate-debug-without-spirv-opt-O1.spv -spirv-opt-path separate-debug-without-spirv-opt-missing-dir
//TEST:COMMAND_LINE_SIMPLE(filecheck=BAD_PASS):-target spirv -O0 -g2 -skip-spirv-validation -Xspirv-opt --separate-debug-without-spirv-opt-bogus-pass -separate-debug-info -separate-debug-info-output separate-debug-without-spirv-opt-bad-pass.dbg.spv -o separate-debug-without-spirv-opt-bad-pass.spv

// O0: result code = 0
// O0-NOT: E00100
// O0-NOT: E99997

// O1: result code = {{-?[1-9][0-9]*}}
// O1-NOT: E99997
// O1: error[E00100]
// O1-NOT: E99997

// BAD_PASS: result code = {{-?[1-9][0-9]*}}
// BAD_PASS-NOT: E00113
// BAD_PASS: spirv-opt{{.*}}error{{.*}}separate-debug-without-spirv-opt-bogus-pass
// BAD_PASS-NOT: E00113

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[tid.x] = tid.x + 1;
}
