// Dynamic dispatch across module boundaries. The interface and one
// implementation live in a helper module; a second implementation
// and the dispatch site are in this file.

//TEST:COMPILE: tests/language-feature/dynamic-dispatch/dispatch-cross-module-helper.slang -o tests/language-feature/dynamic-dispatch/dispatch-cross-module-helper.slang-module
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

import "dispatch-cross-module-helper";

// Uses synthesized init (no explicit __init), unlike HelperImpl.
struct LocalImpl : ICrossModule
{
    int w;
    int getValue() { return w * 2; }
}

int doDispatch(ICrossModule obj)
{
    return obj.getValue();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ICrossModule a = HelperImpl(5);
    ICrossModule b = LocalImpl(3);
    outputBuffer[0] = doDispatch(a); // CHECK: 5
    outputBuffer[1] = doDispatch(b); // CHECK: 6
}
