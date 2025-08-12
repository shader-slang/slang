//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -DBUFFER_TYPE=StructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -DBUFFER_TYPE=RWStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -DBUFFER_TYPE=AppendStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -DBUFFER_TYPE=ConsumeStructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -DBUFFER_TYPE=RasterizerOrderedStructuredBuffer

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer
BUFFER_TYPE<int> buffer;

//CHECK: error 36107:
//CHECK: buffer.GetDimensions

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
