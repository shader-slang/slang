// Regression test for https://github.com/shader-slang/slang/issues/10069
// Zero-size array in nested struct causes ICE 99999 on all targets.
// Run: slangc zero-size-array-nested-struct.slang -target spirv -entry computeMain -stage compute

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//CHECK: error 99999

struct Nested1 { struct Inner { float x; }; Inner data[0]; };
struct Nested2 { Nested1 a; Nested1 b; };
RWStructuredBuffer<Nested2> buf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Nested2 v = buf[tid.x];
    buf[tid.x] = v;
}
