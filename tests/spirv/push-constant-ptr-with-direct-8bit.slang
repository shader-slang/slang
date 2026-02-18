// Test that StoragePushConstant8 IS emitted when a push constant directly
// contains an 8-bit type, even if it also contains pointers to 8-bit types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// CHECK-DAG: OpCapability StoragePushConstant8
// CHECK-DAG: OpCapability PhysicalStorageBufferAddresses
// CHECK-DAG: OpCapability Int8

struct CSInput
{
    uint8_t *output;
    int8_t directValue;
};

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint DTid: SV_DispatchThreadID, uniform CSInput consts)
{
    consts.output[DTid] = consts.directValue;
}
