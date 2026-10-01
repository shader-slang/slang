// Regression for https://github.com/shader-slang/slang/issues/13257.
//
// A Debug build aborted at module load in `prelinkIR` with "duplicate global instruction".
// Importing `shading` -- an interface `IBrdf` plus a conforming `Lambert` whose method calls the
// core intrinsics `max`/`dot` -- into a module that both declares a `Lambert` local and calls
// `normalize` made prelink queue several imported interfaces whose transitive closures overlap on
// one `[__unsafeForceInlineEarly]` helper (`core.float.dadd`). An [Import]-marked clone of that
// helper was added to the module but not registered in prelink's symbol table, so the next
// overlapping closure cloned it a second time under the same mangled name, which `checkIRDuplicate`
// rejects (a `_DEBUG`-only check; Release silently tolerated the duplicate). This test must load
// and run.
//
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import shading;

typealias BRDF = Lambert;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    BRDF brdf = {};
    float3 n = normalize(float3(3.0f, 0.0f, 4.0f)); // (0.6, 0, 0.8)
    float3 r = brdf.shade(n);                       // dot(n,n) == 1, so r == n == (0.6, 0, 0.8)
    // Scale by 10 and round to nearest (`* 10 + 0.5` then truncating `uint(...)`) so the float
    // components map to the small integers the CHECK lines match: 6, 0, 8.
    outputBuffer[0] = uint(r.x * 10.0f + 0.5f);
    // CHECK: 6
    outputBuffer[1] = uint(r.y * 10.0f + 0.5f);
    // CHECK: 0
    outputBuffer[2] = uint(r.z * 10.0f + 0.5f);
    // CHECK: 8
}
