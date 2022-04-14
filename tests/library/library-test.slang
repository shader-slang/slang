// library-test.slang

// A test to try out the basics of using libraries that can be linked (by downstream tools).
// (This is in contrast to Slang modules and Slangs own linkage mechanisms)

// This didn't work for lib_6_2 when compiling via DXC (!). Even though it's stated elsewhere the feature is available from 6.1

//TEST:COMPILE: tests/library/library.slang -profile lib_6_3 -target dxil -o tests/library/library.dxil
//DISABLE_TEST:COMPILE: tests/library/library-test.slang -profile cs_6_3 tests/library/library.dxil -o tests/library/library-test.dxil

// It seems that I can't just compile the source containing the entry point/stage with the library/s. 
// Instead I have to compile all the parts with lib profile, and then link together at the end. 

//DISABLE_TEST:COMPARE_COMPUTE_EX:-slang -compute -xslang -r -xslang tests/serialization/serialized-module.slang-module -shaderobj

extern int foo(int a);

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = (int)dispatchThreadID.x;
       
    outputBuffer[index] = foo(index);
}
