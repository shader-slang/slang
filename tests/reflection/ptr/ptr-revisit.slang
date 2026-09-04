//TEST(64-bit):REFLECTION:-stage compute -no-codegen -target host-callable -entry computeMain

struct Inner  { int x; }
struct Middle { Inner a; Inner b; }   // two fields of the SAME struct type
struct Holder { Middle* p; }

RWStructuredBuffer<Holder> inputBuffer;

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = int(dispatchThreadID.x);
}