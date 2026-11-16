// A scalar-buffer guard — the `i < n && fbuf[i]` idiom — must also emit as
// one short-circuit expression: the bare `__ldg` read folds into the `&&`
// operand, where it evaluates only when the bound check passes. This pins
// the scalar form of the guarded access that the vector-form
// short-circuit-and-guard test covers; it fails if the scalar load stops
// folding into the `if` line. (Companion to shader-slang/slang#14494.)

//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

StructuredBuffer<float> fbuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint count = output[0];
    //CHECK-NOT: bool _S
    if (tid.x < count && fbuf[tid.x] > 0.5)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: if({{.*}}&&{{.*}}__ldg
}
