// Size mismatch: uint3 is 12 bytes, a device pointer is 8. Must be E41202,
// including on Metal where vector<->pointer legalization runs before lowerBitCast.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -stage compute -entry computeMain

uniform uint3 bits;
uniform RWStructuredBuffer<int> outputBuffer;

//CHECK: error[E41202]: bit_cast size mismatch

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    int* p = bit_cast<int*>(bits);
    outputBuffer[0] = p[0];
}
