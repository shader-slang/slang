// Regression for #11289: `lowerReinterpret` can see a vector whose length is
// still a generic value parameter. The compiler must treat that layout size as
// unresolved instead of dereferencing the non-literal count as an integer.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

__generic<let N : int>
vector<float, N> roundTrip(vector<float, N> value)
{
    return reinterpret<vector<float, N>>(value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = roundTrip<2>(float2(1.0, 2.0)).x;
}

// CHECK: RWStructuredBuffer<float > outputBuffer
