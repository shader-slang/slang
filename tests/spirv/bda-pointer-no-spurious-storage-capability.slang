// Regression for #11096: a `uint16_t*` kernel argument (BDA pointer) bundled
// into the entry-point push-constant block used to spuriously require
// `StoragePushConstant16`, because `checkTypeNeedsStorageCapability`
// recursed through pointer types into their pointee.  The pointee lives
// in PhysicalStorageBuffer, not the outer container's storage class.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

[shader("compute")]
[numthreads(512, 1, 1)]
void main(
    uint* param1,
    uint16_t* param2,
    uniform int iteration,
    uint3 groupThreadID : SV_GroupThreadID,
    uint3 groupId : SV_GroupID)
{
    param1[groupId.x] = (uint)param2[groupThreadID.x] + iteration;
}

// The push-constant block contains BDA addresses and a plain int32 only --
// no inline 16-bit scalars -- so none of the *16 storage capabilities
// should be required.
// CHECK-NOT: OpCapability StoragePushConstant16
// CHECK-NOT: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK: OpCapability PhysicalStorageBufferAddresses
// The pointee is a uint16_t, so Int16 is still needed.
// CHECK: OpCapability Int16
