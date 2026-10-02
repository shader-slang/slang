//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):name=storage
RWStructuredBuffer<uint> storage;

// TensorView layout: data pointer, five byte strides, five sizes, rank, padding.
// Bind a contiguous 1D view and a strided 4D view using the existing 32-bit metadata.
//TEST_INPUT:ubuffer(data=[storage 4 0 0 0 0 1073741826 0 0 0 0 1 0], stride=56):name=linear
uniform TensorView<uint>* linear;
//TEST_INPUT:ubuffer(data=[storage 3221225472 1073741824 8 4 0 2 2 2 2 0 4 0], stride=56):name=strided
uniform TensorView<uint>* strided;

//TEST_INPUT:ubuffer(count=7, stride=8):out,name=outputBuffer
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
}

// CHECK: 4
// CHECK: 4294967292
// CHECK: 4294967296
// CHECK: 4294967300
// CHECK: 4294967296
// CHECK: 4294967304
// CHECK: 4294967308
