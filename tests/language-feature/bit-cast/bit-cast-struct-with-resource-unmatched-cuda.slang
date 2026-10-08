// Test that on CUDA a bit_cast between structs holding different resource types, which
// cannot be rebuilt field-for-field and whose destination holds a resource, reports
// E41205 instead of an internal error (issue #13480).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

struct UintView
{
    StructuredBuffer<uint> data;
    uint offset;
}

struct FloatView
{
    StructuredBuffer<float> data;
    uint offset;
}

uniform UintView gView;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    FloatView a = bit_cast<FloatView, UintView>(gView);
//CHECK:                                       ^ E41205
//CHECK:                                       ^ cannot bit_cast 'UintView' to 'FloatView'
//CHECK:                                       ^ has no bit representation here
    outputBuffer[0] = a.data[a.offset];
}
