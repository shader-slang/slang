//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage vertex -entry vertexMain

// SubpassInput requires the fragment stage; using it in compute or vertex should fail.

// CHECK: error[E36107]
// CHECK: SubpassLoad

RWStructuredBuffer<float4> outputBuffer;
[[vk::input_attachment_index(0)]] SubpassInput<float4> input0;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = input0.SubpassLoad();
}

float4 vertexMain() : SV_Position
{
    return input0.SubpassLoad();
}
