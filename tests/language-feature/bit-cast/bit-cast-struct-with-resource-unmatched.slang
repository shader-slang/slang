// Test that a bit_cast between structs holding resources reports E41205 when the
// structs do not match position-for-position (issue #13480): different handle types,
// different field counts, and arrays of different lengths.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute

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

struct ShortView
{
    StructuredBuffer<uint> data;
}

uniform UintView gView;
uniform UintView gViews[2];
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    FloatView a = bit_cast<FloatView, UintView>(gView);
//CHECK:                                       ^ E41205
//CHECK:                                       ^ cannot bit_cast 'UintView' to 'FloatView'
//CHECK:                                       ^ has no bit representation here
    outputBuffer[0] = a.data[a.offset];

    ShortView b = bit_cast<ShortView, UintView>(gView);
//CHECK:                                       ^ E41205
//CHECK:                                       ^ cannot bit_cast 'UintView' to 'ShortView'
//CHECK:                                       ^ has no bit representation here
    outputBuffer[1] = float(b.data[0]);

    UintView c[3] = bit_cast<UintView[3], UintView[2]>(gViews);
//CHECK:                                              ^ E41205
//CHECK:                                              ^ cannot bit_cast
//CHECK:                                              ^ has no bit representation here
    outputBuffer[2] = float(c[2].data[c[2].offset]);
}
