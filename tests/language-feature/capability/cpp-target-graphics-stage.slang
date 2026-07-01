// Regression test for shader-slang/slang#11659.
//
// Compiling a graphics-stage entry point (vertex/fragment/...) to a CPU
// host-C++ kernel target (cpp/hpp/host-callable/...) used to SIGSEGV the
// compiler. Those targets lower entry-point varying parameters with the CPU
// varying-parameter legalizer, which only models the compute system values; a
// graphics varying (e.g. an `SV_Position`) is not representable there, so it
// falls to the legalizer's "unsupported varying" diagnostic. That diagnostic
// dereferenced the per-parameter pointer `m_param`, but the entry point's
// *result* is legalized before the parameter loop assigns `m_param` (it is
// still null), so the dereference crashed.
//
// The fix makes the legalizer fall back to the entry-point function's source
// location when `m_param` is null, turning the crash into a normal diagnostic.
// Each case below would SIGSEGV on the unfixed compiler, so a passing filecheck
// match also proves the process ran to completion (a crash emits no matching
// output).
//
// `c`/`object-code` are intentionally not covered: in some configurations they
// fail earlier (no C backend / no downstream object compiler) before reaching
// the legalizer, so they do not exercise this path. The diagnostic is the
// legalizer's existing "unimplemented" (E99999) fallback for an unsupported
// varying; the load-bearing assertion here is "do not crash; emit a diagnostic
// pointing at the entry point", so the stable message text and the offending
// entry-point name are checked rather than the exact error code (which a future
// diagnostic upgrade may change). Upgrading this to a cleaner user-facing
// diagnostic is a separate enhancement.

// A bare `SV_Position`/`SV_Target` result routes through the *system*-value
// path (`diagnoseUnsupportedSystemVal`).
//TEST:SIMPLE(filecheck=GFX_VS): -target cpp -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_VS): -target hpp -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_VS): -target host-callable -entry vertexMain
//TEST:SIMPLE(filecheck=GFX_FS): -target cpp -entry fragmentMain

// A struct result (the original #11659 repro) routes through the *user*-varying
// path (`diagnoseUnsupportedUserVal`) — a distinct diagnostic helper that shares
// the same null-`m_param` fallback, so it must be covered too.
//TEST:SIMPLE(filecheck=GFX_USER): -target cpp -entry structOutMain

// Two entry points in one invocation: the compute entry point sets the
// legalizer's per-parameter scratch state (`m_param`) and then frees it, and the
// subsequent vertex entry point's result legalization must NOT read that freed
// pointer. This exercises the per-entry-point reset of `m_param` in
// `processEntryPoint`. Without the reset the result diagnostic would read freed
// memory, which is undefined behavior and not reliably observable, so this
// directive is a defense-in-depth / documentary check: it confirms the reset
// keeps the diagnostic pointing at `vertexMain` rather than at stale state.
//TEST:SIMPLE(filecheck=GFX_VS): -target cpp -entry computeMain -stage compute -entry vertexMain -stage vertex

// The diagnostic must not crash, and must point at the offending entry-point
// function — that is the null-`m_param` fallback location the fix introduces,
// so checking the function name guards the fallback directly.
// GFX_VS: this target doesn't support this system-defined varying parameter
// GFX_VS: vertexMain

// GFX_FS: this target doesn't support this system-defined varying parameter
// GFX_FS: fragmentMain

// GFX_USER: this target doesn't support this user-defined varying parameter
// GFX_USER: structOutMain

[shader("vertex")]
float4 vertexMain() : SV_Position
{
    return float4(0, 0, 0, 1);
}

// `SV_Target` is a system value; a fragment entry point returning it reaches the
// same system-value legalizer path as `vertexMain`.
[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(1, 0, 0, 1);
}

// The original issue repro: a struct-returning vertex entry point. The struct
// result is lowered through the user-varying path, exercising
// `diagnoseUnsupportedUserVal` rather than the system-value helper above.
struct VertexOut
{
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOut structOutMain()
{
    VertexOut o;
    o.position = float4(0, 0, 0, 1);
    return o;
}

// A compute entry point with a system-value parameter: legalizing it sets and
// then frees the legalizer's `m_param`, setting up the cross-entry-point reset
// case exercised by the multi-entry directive above.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
}
