// Regression test for shader-slang/slang#11659.
//
// A graphics-stage entry point (vertex/fragment/...) compiled to a CPU
// host-C++ kernel target (cpp/hpp/c/object-code/host-callable/shared-library/
// shader-llvm) used to SIGSEGV the compiler: those targets lower entry-point
// varying parameters with the CPU varying-parameter legalizer, which only
// supports compute system values, and it dereferenced a null parameter while
// legalizing the entry point's result (e.g. an `SV_Position`). The front-end
// now rejects the unsupported stage/target combination with the existing
// capability diagnostic (E36107); and even when that check is bypassed with
// `-ignore-capabilities`, the legalizer diagnoses the unsupported varying
// instead of crashing.
//
// All of the affected targets share the one front-end predicate
// (`isCPUTarget && isKernelTarget`), so the four source `-target` values below
// (cpp/hpp/c/object-code) plus the indirectly-routed `host-callable` are
// representative of the whole family (`shared-library` and `shader-llvm` hit
// the same predicate; see the note by the `host-callable` expected block).
// Each SIMPLE directive asserts on the diagnostic text; the unfixed compiler
// crashed here, so a passing filecheck match also proves the process ran to
// completion (a crash would emit no matching output).

// Default: a clean capability error, not a crash and not silent no-output.
//TEST:SIMPLE(filecheck=VS_CPP): -target cpp -entry vertexMain
//TEST:SIMPLE(filecheck=VS_HPP): -target hpp -entry vertexMain
//TEST:SIMPLE(filecheck=VS_C): -target c -entry vertexMain
//TEST:SIMPLE(filecheck=VS_OBJ): -target object-code -entry vertexMain
//TEST:SIMPLE(filecheck=VS_HC): -target host-callable -entry vertexMain
//TEST:SIMPLE(filecheck=FS_CPP): -target cpp -entry fragmentMain
// With the capability check suppressed, the legalizer's null-guard must turn
// the unsupported result-position system value into a diagnostic, not a crash.
//TEST:SIMPLE(filecheck=IGNORE): -target cpp -entry vertexMain -ignore-capabilities
// Two entry points in one invocation under `-ignore-capabilities`: the compute
// entry point sets the legalizer's per-parameter scratch state (`m_param`) and
// then frees it, and the subsequent vertex entry point's result legalization
// must NOT read that freed pointer. This exercises the per-entry-point reset of
// `m_param` in `processEntryPoint` (without it, the result diagnostic would
// read use-after-free memory). The match proves it diagnosed rather than crashed.
//TEST:SIMPLE(filecheck=TWO): -target cpp -ignore-capabilities -entry computeMain -stage compute -entry vertexMain -stage vertex

// VS_CPP: error[E36107]
// VS_CPP: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// VS_HPP: error[E36107]
// VS_HPP: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// VS_C: error[E36107]
// VS_C: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'c' compilation target.

// `object-code` has no distinct capability atom, so it surfaces its compile
// target as `cpp` in the diagnostic.
// VS_OBJ: error[E36107]
// VS_OBJ: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage for 'cpp' compilation target.

// `host-callable` reaches the same CPU legalizer indirectly (via `CPPSource`);
// match only the stable stage prefix, not the rendered compile target. The
// remaining family members (`shared-library` via `-target dll`/`so`, and
// `shader-llvm`) hit the identical `isCPUTarget && isKernelTarget` front-end
// predicate, so they are covered transitively and are not asserted separately
// (the `dll`/`so` spelling is platform-specific and not worth pinning here).
// VS_HC: error[E36107]
// VS_HC: entrypoint 'vertexMain' uses features that are not available in 'vertex' stage

// FS_CPP: error[E36107]
// FS_CPP: entrypoint 'fragmentMain' uses features that are not available in 'fragment' stage for 'cpp' compilation target.

// E99999 is the legalizer's "unimplemented" fallback for the bypassed
// (`-ignore-capabilities`) path: the contract this test pins is "do not crash;
// emit *a* diagnostic", so the stable message text below is the load-bearing
// assertion (the `-NOT` alone would also pass on a silent crash).
// IGNORE-NOT: error[E36107]
// IGNORE: internal error[E99999]
// IGNORE: this target doesn't support this system-defined varying parameter

// TWO-NOT: error[E36107]
// TWO: this target doesn't support this system-defined varying parameter

[shader("vertex")]
float4 vertexMain() : SV_Position
{
    return float4(0, 0, 0, 1);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}

// A compute entry point with a system-value parameter: legalizing it sets and
// then frees the legalizer's `m_param`, setting up the cross-entry-point reset
// case exercised by the TWO directive above.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
}
