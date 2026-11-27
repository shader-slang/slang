// Regression test for https://github.com/shader-slang/slang/issues/13200 — LLVM backend side.
//
// The #13200 fix classifies `intptr_t` as signed in the shared, backend-agnostic `isSignedType`
// (slang-ir-util.cpp), aligning it with the canonical `getIntTypeSigned`. Because that classifier
// also drives the LLVM backend, a signed `intptr_t` comparison must lower to the signed `icmp slt`;
// before the fix `isSignedType(intptr_t)` was false, so it wrongly emitted the unsigned `icmp ult`.

//TEST:SIMPLE(filecheck=CHECK): -target llvm-shader-ir -entry computeMain -stage compute

RWStructuredBuffer<int> outp;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Runtime-derived so the comparison is not constant-folded; `a` may be negative.
    intptr_t a = intptr_t(tid.x) - 5;
    intptr_t b = intptr_t(tid.y);
    outp[0] = (a < b) ? 1 : 0;
}

// CHECK: icmp slt i64
