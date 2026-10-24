//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cpp -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Copying a whole value out of a constant buffer into a structured buffer of
// the same struct, when the two buffers lower the struct to different storage
// types (here because of the `float3x3`). Only SPIR-V can copy between two
// storage types directly; the other targets convert the value to its logical
// type and store it again.

struct S
{
    float x;
    float3x3 m;
};

//TEST_INPUT:set cb = cbuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0])
ConstantBuffer<S> cb;

//TEST_INPUT:set outS = ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=40)
RWStructuredBuffer<S> outS;

//TEST_INPUT:set output = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    outS[0] = cb;
    output[0] = outS[0].x;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            output[1 + r * 3 + c] = outS[0].m[r][c];
}

// CHECK: computeMain

// BUF: 1.0
// BUF-NEXT: 2.0
// BUF-NEXT: 3.0
// BUF-NEXT: 4.0
// BUF-NEXT: 5.0
// BUF-NEXT: 6.0
// BUF-NEXT: 7.0
// BUF-NEXT: 8.0
// BUF-NEXT: 9.0
// BUF-NEXT: 10.0
