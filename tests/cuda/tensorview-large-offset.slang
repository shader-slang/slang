//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):name=storage
RWStructuredBuffer<uint> storage;

// Wide TensorView layout: data pointer, five uint64 byte strides, five uint64 sizes, rank, padding.
//TEST_INPUT:ubuffer(data=[storage 4 0 0 0 0 0 0 0 0 0 1073741826 0 0 0 0 0 0 0 0 0 1 0], stride=96):name=linear
uniform TensorView<uint, uint64_t>* linear;
//TEST_INPUT:ubuffer(data=[storage 3221225472 0 1073741824 0 8 0 4 0 0 0 2 0 2 0 2 0 2 0 0 0 4 0], stride=96):name=strided
uniform TensorView<uint, uint64_t>* strided;
// A byte stride of 4 GiB + 4 itself requires 64 bits.
//TEST_INPUT:ubuffer(data=[storage 4 1 0 0 0 0 0 0 0 0 2 0 0 0 0 0 0 0 0 0 1 0], stride=96):name=wideStride
uniform TensorView<uint, uint64_t>* wideStride;

//TEST_INPUT:ubuffer(count=9, stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Inspect addresses without dereferencing them, so CI needs only a small allocation.
    uint64_t base = uint64_t(__getAddress(storage[0]));
    outputBuffer[0] = uint64_t(linear.data_ptr_at(1)) - base;
    outputBuffer[1] = uint64_t(linear.data_ptr_at((1u << 30) - 1)) - base;
    outputBuffer[2] = uint64_t(linear.data_ptr_at(1u << 30)) - base;
    outputBuffer[3] = uint64_t(linear.data_ptr_at((1u << 30) + 1)) - base;

    // Each product fits in 32 bits, but their sum does not.
    outputBuffer[4] = uint64_t(strided.data_ptr_at(uint2(1, 1))) - base;
    outputBuffer[5] = uint64_t(strided.data_ptr_at(uint3(1, 1, 1))) - base;
    outputBuffer[6] = uint64_t(strided.data_ptr_at(uint4(1, 1, 1, 1))) - base;
    outputBuffer[7] = uint64_t(wideStride.data_ptr_at(1)) - base;
    outputBuffer[8] = wideStride.stride(0);
}

// CHECK: 4
// CHECK: 4294967292
// CHECK: 4294967296
// CHECK: 4294967300
// CHECK: 4294967296
// CHECK: 4294967304
// CHECK: 4294967308
// CHECK: 4294967300
// CHECK: 4294967300
