//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Regression test for shader-slang/slang#11616: two sibling [ForceInline] calls in one caller.
//
// Each call must be inlined directly at computeMain — a two-operand, top-level DebugInlinedAt
// (Line + Scope, no Inlined operand) — and never nested under the other. The caller-restore
// DebugScope emitted after the first inlined region bounds the inliner's backward DebugInlinedAt
// scan; without that bound the second call would inherit the first call's inline site and appear
// nested under it (a three-operand DebugInlinedAt).

[ForceInline]
int firstInline(int a, int b)
{
    return a * 2 + b;
}

[ForceInline]
int secondInline(int a, int b)
{
    return a + b * 3;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    int r0 = firstInline(16, 10);
    int r1 = secondInline(7, 5);
    outputBuffer[i] = r0 + r1;
}

// Two top-level DebugInlinedAt records (Line + Scope only), one per inlined call.
// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}{{$}}
// No call is nested under the other: no three-operand (Line + Scope + Inlined) DebugInlinedAt.
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
