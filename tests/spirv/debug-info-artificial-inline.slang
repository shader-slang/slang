//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O1 -emit-spirv-directly

// Check that an inlined constructor retains its artificial flag and inline scope.
// A runtime initializer keeps the constructor's inline variable metadata live.
//CHECK: [[NAME:%[0-9]+]] = OpString "Generated.$init"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK: [[CTOR:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugFunction [[NAME]] {{.*}} [[NAME]] [[ARTIFICIAL]]
//CHECK: [[SITE:%[0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugInlinedAt
//CHECK: DebugScope [[CTOR]] [[SITE]]

RWStructuredBuffer<float> data;
struct Generated
{
    float value = data[0] + 1.0;
};
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Generated value = {};
    data[1] = value.value;
}
