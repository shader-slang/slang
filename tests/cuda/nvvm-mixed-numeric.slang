// Exercises the bounded integer-width, signedness, conversion, float32, and int2 support selected
// by Slice 68 through an ordinary file-backed direct-NVVM compilation.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0

[CUDAKernel]
void computeMain(
    uniform Ptr<int8_t, Access::ReadWrite, AddressSpace::Device> output8,
    uniform Ptr<uint16_t, Access::ReadWrite, AddressSpace::Device> output16,
    uniform Ptr<int64_t, Access::ReadWrite, AddressSpace::Device> output64,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> output32,
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> outputFloat,
    uniform Ptr<int2, Access::ReadWrite, AddressSpace::Device> outputVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> leftVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> rightVector,
    uniform int8_t narrowSigned,
    uniform uint8_t narrowUnsigned,
    uniform int16_t mediumSigned,
    uniform uint16_t mediumUnsigned,
    uniform int64_t wideSigned,
    uniform uint64_t wideUnsigned,
    uniform float scalarFloat)
{
    int index = int(cudaThreadIdx().x);
    output8[index] = ~(narrowSigned + int8_t(narrowUnsigned));
    output16[index] = (uint16_t(mediumSigned) + mediumUnsigned) ^ uint16_t(0x55aa);
    output64[index] = (int64_t(wideUnsigned) + wideSigned) * int64_t(3);
    output32[index] = int(scalarFloat) + int(narrowUnsigned);
    outputFloat[index] = float(mediumSigned) + scalarFloat;
    outputVector[index] = leftVector[index] + rightVector[index];
}

// CHECK: .visible .entry computeMain
// CHECK: mov.u32 {{.*}}%tid.x
// CHECK: st.global.u8
// CHECK: st.global.u16
// CHECK: st.global.u64
// CHECK: st.global.u32
// CHECK: st.global.f32
