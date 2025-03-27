// CHECK: ; Function secondIntermediateFunction
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
[ForceInline]
int secondIntermediateFunction(int value)
{
    return basicBlockFunction(value * 3);
}

[ForceInline]
int thirdIntermediateFunction(int value)
{
    int r = secondIntermediateFunction(value);
    return basicBlockFunction(r * 3);
}

int fourthIntermediateFunction(int value)
{
    return thirdIntermediateFunction(value);
}



[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call both intermediate functions and combine their results
    int result1 = intermediateFunction(i + 10, i * 3);
    int result2 = secondIntermediateFunction(i + 5);
    int result3 = fourthIntermediateFunction(i+10);
    
    outputBuffer[i] = result1 + result2 + result3;
}

// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-6: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope