//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Generic function with ForceInline attribute - now with multiple basic blocks
[ForceInline]
T genericInlineFunction<T : __BuiltinArithmeticType>(T value1, T value2)
{
    // Multiple basic blocks with conditional branching
    T result;
    if (value1 > value2)
    {
        result = value1 + value1 - value2;
    }
    else
    {
        result = value1 + value2 + value2;
    }
    return result;
}

// Basic block function that will be called by the second intermediate function
[ForceInline]
int basicBlockFunction(int x)
{
    // Simple operation (single basic block)
    return x + 42;
}

// Non-inline function that calls the forceinline generic function
int intermediateFunction(int a, int b)
{
    return genericInlineFunction<int>(a * 2, b + 5);
}

// Second intermediate function that calls the basic block function
int secondIntermediateFunction(int value)
{
    return basicBlockFunction(value * 3);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call both intermediate functions and combine their results
    int result1 = intermediateFunction(i + 10, i * 3);
    int result2 = secondIntermediateFunction(i + 5);
    
    outputBuffer[i] = result1 + result2;
}
