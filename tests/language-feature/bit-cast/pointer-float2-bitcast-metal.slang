// float2 and a 64-bit pointer are both 8 bytes, so this is a well-typed
// same-size bit_cast (unlike uint3). On Metal it should compile via the
// vector<->pointer ulong hop, not E41202.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

uniform float2 bits;
uniform RWStructuredBuffer<int> outputBuffer;

//CHECK-NOT: E41202
//CHECK: (int device*)(ulong(

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    int* p = bit_cast<int*>(bits);
    outputBuffer[0] = p[0];
}
