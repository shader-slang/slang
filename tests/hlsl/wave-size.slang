//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_CUDA): -target cuda -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// HLSL preserves WaveSize attribute in generated code
// CHECK_HLSL: [WaveSize(4)]

// CUDA doesn't emit WaveSize attribute but generates valid compute kernel
// CHECK_CUDA: RWStructuredBuffer<int> [[OUTPUTBUFFER:outputBuffer[_0-9]*]];
// CHECK_CUDA: extern "C" __global__ void computeMain()
// CHECK_CUDA: *(&([[GLOBALPARAMS:globalParams[_0-9]*]]->[[OUTPUTBUFFER]])[[[TID:tid[_0-9]*]]]) = [[TID]];
[WaveSize(4)]
[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = dispatchThreadID.x;
	outputBuffer[tid] = tid;
}
