// Test that a push constant containing a pointer to uint8_t does NOT
// emit OpCapability StoragePushConstant8. The capability is for storing
// 8-bit values directly in push constants, not for pointers to 8-bit values.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// CHECK-NOT: OpCapability StoragePushConstant8
// CHECK: OpCapability PhysicalStorageBufferAddresses
// CHECK: OpCapability Int8

struct CSInput
{
    uint8_t *output;
};

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint DTid: SV_DispatchThreadID, uniform CSInput consts)
{
    consts.output[DTid] = 1;
}
