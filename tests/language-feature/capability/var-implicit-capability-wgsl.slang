//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute -ignore-capabilities


Buffer<float> InputTyped;
RWBuffer<float> OutputTyped;

//CHECK: ([[# @LINE+3]]): error 36107
//CHECK_IGNORE_CAPS-NOT: error 36107
[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    OutputTyped[DTid.x] = InputTyped[DTid.x];
}