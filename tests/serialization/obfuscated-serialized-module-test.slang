// obfuscated-serialized-module-test.slang

// A test to try out the basics of module
// serialization, obfuscation and source maps.

//TEST:COMPILE: tests/serialization/serialized-module.slang -o tests/serialization/obfuscated-serialized-module.slang-module -obfuscate -source-map
//TEST:COMPARE_COMPUTE_EX:-slang -compute -Xslang... -r tests/serialization/obfuscated-serialized-module.slang-module -obfuscate -source-map -X. -shaderobj

//import obfuscated_serialized_module;

// This is fragile - needs match the definition in obfuscated_serialized_module
struct Thing
{
    int a;
    int b;
};

// TODO: need to get the name mangling to line up!
int foo(Thing thing);

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Thing thing;

    int index = (int)dispatchThreadID.x;
        
    thing.a = index;
    thing.b = -index;

    outputBuffer[index] = foo(thing);
}
