//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Issue #11545: `ByteAddressBuffer`/`RWByteAddressBuffer` `LoadAligned`/`Store`
// take a `constexpr` alignment, so a runtime (non-compile-time-constant)
// alignment is rejected at compile time. This replaces the former custom
// diagnostic with the language's built-in `constexpr` enforcement, and is a
// front-end check, so it fires independent of the chosen target.

RWByteAddressBuffer buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint runtimeAlignment = tid.x;
//CHECK:                        ^ expected a compile-time constant
    float v = buf.LoadAligned<float>(0, runtimeAlignment);
    buf.Store<float>(16, v, runtimeAlignment);
}
