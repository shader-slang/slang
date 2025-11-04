//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Test that enums can be declared inside generic structs without crashing

public struct Test<T> {
    public enum Inner {
        Variant1,
        Variant2
    };
    
    T value;
    Inner state;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Test<int> t;
    t.value = 42;
    t.state = Test<int>.Inner.Variant1;
    
    // CHECK: 42
    outputBuffer[0] = t.value;
    // CHECK: 0
    outputBuffer[1] = int(t.state);
}
