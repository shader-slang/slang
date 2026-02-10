//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Test that properly initialized interface variables work correctly
// This is the positive test showing correct patterns that should NOT produce errors

interface ICalculator
{
    float compute(float x);
}

struct Square : ICalculator
{
    float factor;
    float compute(float x) { return x * x * factor; }
};

struct Cube : ICalculator
{
    float factor;
    float compute(float x) { return x * x * x * factor; }
};

// Test 1: Initialize at declaration
float test_init_at_declaration()
{
    ICalculator calc = Square(2.0);  // OK: initialized at declaration
    return calc.compute(3.0);
}

// Test 2: Conditional initialization - ALL paths initialized
float test_conditional_all_paths(bool condition)
{
    ICalculator calc;
    
    if (condition)
    {
        calc = Square(1.0);
    }
    else
    {
        calc = Cube(2.0);  // This path also initializes
    }
    
    return calc.compute(3.0);  // OK: all paths initialize
}

// Test 3: Switch with default case covering all paths
float test_switch_complete(uint selector)
{
    ICalculator calc;
    
    switch(selector)
    {
        case 0:
            calc = Square(1.0);
            break;
        case 1:
            calc = Cube(2.0);
            break;
        default:
            calc = Square(3.0);  // Default ensures all paths covered
            break;
    }
    
    return calc.compute(2.0);  // OK: all cases covered
}

// Test 4: Loop with guaranteed initialization
float test_loop_guaranteed()
{
    ICalculator calc = Square(1.0);  // Initialize before loop
    
    for (uint i = 0; i < 10; i++)
    {
        if (i == 5)
        {
            calc = Cube(2.0);  // Reassignment is OK
            break;
        }
    }
    
    return calc.compute(2.0);  // OK: initialized before loop
}

// Test 5: Initialize immediately after declaration
float test_init_after_declaration()
{
    ICalculator calc;
    calc = Square(5.0);  // Initialize before use
    return calc.compute(2.0);  // OK
}

// Test 6: Multiple variables all initialized
float test_multiple_initialized()
{
    ICalculator calc1 = Square(1.0);
    ICalculator calc2 = Cube(2.0);
    
    return calc1.compute(3.0) + calc2.compute(3.0);  // OK: both initialized
}

// Test 7: Initialize in helper function
ICalculator createCalculator(uint id)
{
    if (id == 0)
        return Square(1.0);
    else
        return Cube(2.0);
}

float test_factory_initialization(uint id)
{
    ICalculator calc = createCalculator(id);  // OK: factory returns initialized value
    return calc.compute(4.0);
}

// Test 8: Reassignment after initialization
float test_reassignment()
{
    ICalculator calc = Square(1.0);  // Initialize
    float result1 = calc.compute(2.0);  // OK: Use
    
    calc = Cube(3.0);  // OK: Reassign
    float result2 = calc.compute(2.0);  // OK: Use after reassignment
    
    return result1 + result2;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = test_init_at_declaration();        // CHECK: 18.0
    outputBuffer[1] = test_conditional_all_paths(true);  // CHECK: 9.0
    outputBuffer[2] = test_conditional_all_paths(false); // CHECK: 54.0
    outputBuffer[3] = test_switch_complete(0);           // CHECK: 4.0
    outputBuffer[4] = test_switch_complete(1);           // CHECK: 16.0
    outputBuffer[5] = test_switch_complete(99);          // CHECK: 12.0
    outputBuffer[6] = test_loop_guaranteed();            // CHECK: 16.0
    outputBuffer[7] = test_init_after_declaration();     // CHECK: 20.0
    outputBuffer[8] = test_multiple_initialized();       // CHECK: 63.0
    outputBuffer[9] = test_factory_initialization(1);    // CHECK: 128.0
}
