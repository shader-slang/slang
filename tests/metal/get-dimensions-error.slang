//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK0): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK1): -target metal -entry computeMainRW -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK2): -target metal -entry computeMainAppend -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK3): -target metal -entry computeMainConsume -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK4): -target metal -entry computeMainRaster -stage compute

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer0
StructuredBuffer<int> buffer0;
//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer1
RWStructuredBuffer<int> buffer1;
//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer2
AppendStructuredBuffer<int> buffer2;
//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer3
ConsumeStructuredBuffer<int> buffer3;
//TEST_INPUT:ubuffer(data=[7 2 9 53], stride=4):name buffer4
RasterizerOrderedStructuredBuffer<int> buffer4;

//CHECK0: error 36107:
//CHECK0: buffer0.GetDimensions
//CHECK1: error 36107:
//CHECK1: buffer1.GetDimensions
//CHECK2: error 36107:
//CHECK2: buffer2.GetDimensions
//CHECK3: error 36107:
//CHECK3: buffer3.GetDimensions
//CHECK4: error 36107:
//CHECK4: buffer4.GetDimensions

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer0.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
[numthreads(1, 1, 1)]
void computeMainRW(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer1.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}

[numthreads(1, 1, 1)]
void computeMainAppend(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer2.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
[numthreads(1, 1, 1)]
void computeMainConsume(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer3.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
[numthreads(1, 1, 1)]
void computeMainRaster(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint count = 0, stride = 0;
    buffer4.GetDimensions(count, stride);
	outputBuffer[0] = count;
    outputBuffer[1] = stride;
}
