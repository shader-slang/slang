//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Test that Atomic<T>::load() is non-mutating and can be called in a property getter.
// This is a regression test for GitHub issue where load() was incorrectly marked as [__ref]
// instead of [constref], causing it to be considered mutating.

groupshared Atomic<uint> sharedNumber;

struct AtomicCounter
{
    // This property getter should compile since load() is non-mutating
    property uint value { get { return sharedNumber.load(); } }
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    AtomicCounter counter;
    
    // Access the value through the property getter
    // This should compile without errors
    uint val = counter.value;
    
    outputBuffer[0] = val;
}

// CHECK: OpAtomicLoad
