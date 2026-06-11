//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -emit-spirv-directly -O0

// Regression test for shader-slang/slang#11550.
//
// A struct initializer list synthesizes a member-wise initializer. With -g2 -O0
// that synthesized function must NOT receive source-level debug info: it has no
// user-authored body, and its source locations are inherited from the struct and
// member declarations. Previously it was given a DebugFunction (named after the
// struct, here "Pair.$init") plus a DebugScope and DebugLines on the struct/member
// declaration lines, so a debugger could step *into* the compiler-generated
// initializer and walk the struct/member declarations instead of staying on the
// user's initializer-list statement and advancing to the next statement.
//
// Checks below are written to be robust against -g2 embedding the source text as
// a DebugSource OpString: quoted names match only the real (unescaped) records,
// and DebugLine checks reference resolved @LINE numbers absent from the embedded
// directive text.

// Debug info is emitted (main gets a debug name), but the synthesized initializer
// never gets one, so it has no DebugFunction / DebugScope / DebugLine at all:
//CHECK: OpString "main"
//CHECK-NOT: OpString "Pair.$init"

struct Pair
{
    float x;
    float y;
};

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p = {1.0, 2.0};
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
    outputBuffer[tid.x] = p.x + p.y;
    //CHECK: DebugLine {{.*}} %uint_[[#@LINE-1]]
}
