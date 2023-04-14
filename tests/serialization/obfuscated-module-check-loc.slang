//TEST:COMPILE: tests/serialization/obfuscated-loc-module.slang -o tests/serialization/obfuscated-loc-module.zip -g -obfuscate -source-map
//TEST:SIMPLE:-target hlsl -stage compute -entry computeMain -obfuscate -r tests/serialization/obfuscated-loc-module.zip -g -source-map -line-directive-mode none

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import obfuscated_loc_module;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = int(dispatchThreadID.x);
    
    int v = silly(x);
    
    outputBuffer[x] = v;
}

