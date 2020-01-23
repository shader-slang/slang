//TEST(compute):COMPARE_COMPUTE:-cpu -compute 
//TEST(compute):COMPARE_COMPUTE:-cuda -compute 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer : register(u0);

//TEST_INPUT: Texture2D(size=4, content=one):name texture
Texture2D<float> texture;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = int(dispatchThreadID.x);

    int3 point = int3(tid, 3 - tid, 0);
    
    outputBuffer[tid] = texture.Load(point);
}
