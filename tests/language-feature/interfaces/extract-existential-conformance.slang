// Test that ExtractExistentialType from interface I is a subtype of I
// Tests lines 231-247 in slang-check-conformance.cpp

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IComputable {
    int compute(int x);
}

struct ImplA : IComputable {
    int multiplier;
    int compute(int x) { return x * multiplier; }
}

struct ImplB : IComputable {
    int offset;
    int compute(int x) { return x + offset; }
}

// Test that when we extract an existential value from an interface type,
// the extracted type is still considered a subtype of the original interface
int processViaInterface(IComputable obj, int value) {
    // The existential value 'obj' when extracted should still conform to IComputable
    return obj.compute(value);
}

int testExtraction(uint id, int value) {
    IComputable obj;
    if (id == 0)
        obj = ImplA(3);
    else
        obj = ImplB(10);
    
    // This should work because ExtractExistentialType(IComputable) <: IComputable
    return processViaInterface(obj, value);
}

//TEST_INPUT: type_conformance ImplA:IComputable = 0
//TEST_INPUT: type_conformance ImplB:IComputable = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    // CHECK: 15
    // ImplA with multiplier=3: 5 * 3 = 15
    outputBuffer[0] = testExtraction(0, 5);
    
    // CHECK: 17
    // ImplB with offset=10: 7 + 10 = 17
    outputBuffer[1] = testExtraction(1, 7);
}
