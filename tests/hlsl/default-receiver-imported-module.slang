//TEST:COMPILE: tests/hlsl/default-receiver-module-lib.hlsl -o tests/hlsl/default-receiver-module-lib.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A default HLSL method or interface requirement accepts a copy of an immutable receiver (issue
// #13964). Both come from a precompiled `.slang-module` here, so the calls are accepted only if
// serialization preserves the checked receiver information that permits them.

import default_receiver_module_lib;

//TEST_INPUT: set gHelpers = ubuffer(data=[7], stride=4)
StructuredBuffer<Helper> gHelpers;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

int getThroughRequirement<T : IGetter>(T getter)
{
    let immutableGetter = getter;
    return immutableGetter.get();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 7
    outputBuffer[0] = gHelpers[0].get();
    // CHECK: 7
    outputBuffer[1] = getThroughRequirement(gHelpers[0]);
}
