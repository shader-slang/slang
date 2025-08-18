//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=0 -DBUFFER_TYPE=StructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=0 -DBUFFER_TYPE=RWStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=0 -DBUFFER_TYPE=AppendStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=0 -DBUFFER_TYPE=ConsumeStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=0 -DBUFFER_TYPE=RasterizerOrderedStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=1 -DBUFFER_TYPE=ByteAddressBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=1 -DBUFFER_TYPE=RWByteAddressBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry cmain -DTEST=1 -DBUFFER_TYPE=RasterizerOrderedByteAddressBuffer

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer
#if TEST == 0
BUFFER_TYPE<int> buffer;
#elif TEST==1
BUFFER_TYPE buffer;
#endif

//CHECK: error 36107:
//CHECK: buffer.GetDimensions

[shader("compute")]
[numthreads(1, 1, 1)]
void cmain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
#if TEST == 0
    buffer.GetDimensions(count, stride);
#elif TEST == 1
    buffer.GetDimensions(count);
#endif
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
