// serialized-module.slang

// A test to try out the basics of module
// serialization.

//DISABLE_TEST:SERIALIZED_MODULE:
//TEST:COMPARE_COMPUTE_EX:-slang -compute

#ifdef PRODUCER

//int foo(int x) { return x; }

#else

//import serialized_module;

// TODO: need to get the name mangling to line up!
int foo(int x) { return x; }

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):dxbinding(0),glbinding(0),out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = (int)dispatchThreadID.x;
    outputBuffer[index] = foo(index);
}

#endif