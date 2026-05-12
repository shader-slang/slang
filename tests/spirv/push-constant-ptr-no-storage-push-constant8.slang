// Regression for #10055: a push-constant block containing a pointer to
// uint8_t should not require StoragePushConstant8. The push constant stores a
// physical-storage-buffer address; the pointed-to uint8_t lives in that storage
// class, not inline in the push-constant block.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

struct CSInput
{
    uint8_t* output;
};

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint DTid: SV_DispatchThreadID, uniform CSInput consts)
{
    consts.output[DTid] = 1;
}

// CHECK-NOT: OpCapability StoragePushConstant8
// CHECK: OpCapability PhysicalStorageBufferAddresses
// The pointee is a uint8_t, so Int8 is still needed.
// CHECK: OpCapability Int8
// CHECK-NOT: OpCapability StoragePushConstant8
