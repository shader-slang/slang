//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute 
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -profile cs_6_0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[3 10 2 -1  4 53 4 6  1 2 3 4  7 5 3 1], stride=4):name inputBuffer
RWStructuredBuffer<int> inputBuffer;

groupshared int sharedMem[32];

int prefixSum(WaveMask mask, int index, int waveLaneId, int originalValue, int elementCount)
{
    WaveMask localMask = WaveMaskBallot(mask, waveLaneId < elementCount);
    
    if(waveLaneId < elementCount)
	{
		int temp = 0;
		int val = originalValue;		
        
		for(int  i = 1; i < elementCount; i += i)
		{
			int temp = WaveMaskShuffle(localMask, val, waveLaneId - i);			
            if(waveLaneId >= i)
				val += temp;
		}		
        
        // Make it an exclusive prefix sum
        val -= originalValue;
        
        // Write to shared memory
		sharedMem[index] = val;
        
        // Syncronizes on the mask, and ensures memory fence for shared data write
		WaveMaskSync(localMask); 
		return val;
	}	
    
    return 0;	
}

[numthreads(32, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    const int waveLaneId = WaveGetLaneIndex();
    
    const int value = inputBuffer[index];
    const int elementCount = 9;
    
    prefixSum(WaveGetActiveMask(), index, waveLaneId, value, elementCount);
    
    // It returns the result, but we are going to read from shared memory
    
    int prefixValue = sharedMem[index];
    
    outputBuffer[index] = value;
}