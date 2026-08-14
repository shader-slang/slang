// Verify the integer `dot` fallback emits straight-line multiply-adds — no runtime `for` loop. The
// `default:` arm of the integer `dot` overload (`hlsl.meta.slang`, `T : __BuiltinIntegerType`) is
// `[ForceUnroll]`'d. GLSL, Metal, and the C-like family (CUDA/CPU) reach it (they have no native
// integer `dot`), whereas HLSL/WGSL/SPIR-V take native arms. GLSL, Metal, and CUDA are checked here
// as representative fallback targets — this is not exhaustive fallback coverage (CPU/LLVM also reach
// the arm, but their host output always contains unrelated compute-grid loops, so a whole-output
// `-NOT: for(` cannot be used for them).
//
// Each prefix asserts `-NOT: for(` both before AND after its entry-point anchor so the checked region
// spans the whole output: the `dot` helper carrying the loop is emitted ahead of the entry point
// today, but a trailing `-NOT` keeps the test honest if that helper is ever inlined into the entry
// point (the loop would then move below the anchor). Operands are derived from `SV_DispatchThreadID`
// so the `dot` calls are not constant-folded before reaching the fallback. Three widths (2, 3, 4)
// are exercised so the loop appears three times when unrolling regresses, independent of inlining.
//
// GLSL renames the compute entry point to `main`; Metal and CUDA keep `computeMain`.

//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

// CHECK_GLSL-NOT: for(
// CHECK_GLSL: void main
// CHECK_GLSL-NOT: for(
// CHECK_METAL-NOT: for(
// CHECK_METAL: computeMain
// CHECK_METAL-NOT: for(
// CHECK_CUDA-NOT: for(
// CHECK_CUDA: computeMain
// CHECK_CUDA-NOT: for(

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int2 a2 = int2(int(tid.x), int(tid.y));
    int2 b2 = int2(int(tid.y), int(tid.x));
    int3 a3 = int3(int(tid.x), int(tid.y), int(tid.z));
    int3 b3 = int3(int(tid.z), int(tid.x), int(tid.y));
    int4 a4 = int4(int(tid.x), int(tid.y), int(tid.z), int(tid.x));
    int4 b4 = int4(int(tid.z), int(tid.y), int(tid.x), int(tid.z));
    outputBuffer[0] = dot(a2, b2) + dot(a3, b3) + dot(a4, b4);
}
