//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

// Test detection of uninitialized interface variable usage
// This test validates that the compiler properly detects and reports errors
// when interface variables are used before being initialized.

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

// Test 1: Direct uninitialized usage
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
void test_uninitialized()
{
    ICalculator calc;
    float result = calc.compute(2.0);  // Error: uninitialized
}

// Test 2: Conditional initialization - one path uninitialized
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
void test_conditional_init(bool condition)
{
    ICalculator calc;
    
    if (condition)
    {
        calc = Square(1.0);
    }
    // else path leaves calc uninitialized
    
    float result = calc.compute(3.0);  // Error: potentially uninitialized
}

// Test 3: Loop with break - potential uninitialized
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
void test_loop_break(uint count)
{
    ICalculator calc;
    
    for (uint i = 0; i < count; i++)
    {
        if (i == 5)
        {
            calc = Cube(2.0);
            break;
        }
    }
    // If count < 5, calc is uninitialized
    
    float result = calc.compute(2.0);  // Error: potentially uninitialized
}

// Test 4: Early return with uninitialized use
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
float test_early_return(bool earlyExit)
{
    ICalculator calc;
    
    if (earlyExit)
    {
        return 0.0;
    }
    
    // Oops, forgot to initialize calc
    return calc.compute(5.0);  // Error: uninitialized
}

// Test 5: Multiple variables - one uninitialized
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
void test_multiple_vars()
{
    ICalculator calc1 = Square(1.0);  // OK: initialized
    ICalculator calc2;                 // Uninitialized
    
    float result1 = calc1.compute(2.0);  // OK
    float result2 = calc2.compute(2.0);  // Error: uninitialized
}

// Test 6: Switch statement with missing case
// CHECK: error 50101: Cannot dynamically dispatch on potentially uninitialized interface object
void test_switch_incomplete(uint selector)
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
        // Missing default case
    }
    
    float result = calc.compute(3.0);  // Error: potentially uninitialized if selector >= 2
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // These functions will produce errors at compile time
    test_uninitialized();
    test_conditional_init(true);
    test_loop_break(10);
    test_multiple_vars();
    test_switch_incomplete(0);
}
