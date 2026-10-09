//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):name=storage
RWStructuredBuffer<uint> storage;

// Four axes with byte strides 4, 8, 16, 32 and sizes 2^32 + 2, plus a singleton fifth axis.
// Layout: pointer, five uint64 strides, five uint64 sizes, uint rank, padding.
//TEST_INPUT:ubuffer(data=[storage 4 0 8 0 16 0 32 0 64 0 2 1 2 1 2 1 2 1 1 0 5 0], stride=96):name=tensor
uniform TensorView<uint, uint64_t>* tensor;

//TEST_INPUT:ubuffer(count=10, stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Check addresses without dereferencing them so the test needs only a small allocation.
    uint64_t base = uint64_t(__getAddress(storage[0]));
    uint64_t index = (uint64_t(1) << 32) + 1;
    outputBuffer[0] = tensor.size(0);
    outputBuffer[1] = uint64_t(tensor.data_ptr_at(index)) - base;
    outputBuffer[2] = uint64_t(tensor.data_ptr_at(vector<uint64_t, 2>(index))) - base;
    outputBuffer[3] = uint64_t(tensor.data_ptr_at(vector<uint64_t, 3>(index))) - base;
    outputBuffer[4] = uint64_t(tensor.data_ptr_at(vector<uint64_t, 4>(index))) - base;

    // Reference subscripts exercise load's scalar overloads without reading the addressed data.
    outputBuffer[5] = uint64_t(&(*tensor)[index]) - base;
    outputBuffer[6] = uint64_t(&(*tensor)[index, index]) - base;
    outputBuffer[7] = uint64_t(&(*tensor)[index, index, index]) - base;
    outputBuffer[8] = uint64_t(&(*tensor)[index, index, index, index]) - base;
    outputBuffer[9] = uint64_t(&(*tensor)[index, index, index, index, 0]) - base;
}

// CHECK: 4294967298
// CHECK: 17179869188
// CHECK: 51539607564
// CHECK: 120259084316
// CHECK: 257698037820
// CHECK: 17179869188
// CHECK: 51539607564
// CHECK: 120259084316
// CHECK: 257698037820
// CHECK: 257698037820
