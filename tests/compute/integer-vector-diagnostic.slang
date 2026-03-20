//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target wgsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

cbuffer VectorBuffer
//CHECK: disallowed vector element type
{
    vector<int8_t, 2> int8Vector;
    vector<int16_t, 2> int16Vector;
    vector<int64_t, 3> int64Vector;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = int8Vector.x + int16Vector.y + int64Vector.z;
//CHECK:              ^^^^^^^^^^ disallowed vector element type
}
