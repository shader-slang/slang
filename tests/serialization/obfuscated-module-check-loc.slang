//TEST:COMPILE: tests/serialization/obfuscated-loc-module.slang -o tests/serialization/obfuscated-loc-module.zip -g -obfuscate -source-map
//TEST:SIMPLE:-target hlsl -stage compute -entry computeMain -obfuscate -r tests/serialization/obfuscated-loc-module.zip 

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// This test checks obfuscated source map loc tracking through a round trip, and producing a location correctly from slang-module that has a source map

import obfuscated_loc_module;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = int(dispatchThreadID.x);
    
    // Will produce an error, because silly has an error.
    int v = silly(x);
    
    outputBuffer[x] = v;
}

