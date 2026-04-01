//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Failure to pass compile-time-constant argument
// where it is expected.
//
// Test both callee and nested callee that require constexpr parameters.
// Test both function defined in the same module and external function.

Texture2D<int> input;
RWTexture2D<int> output;

int func_with_constexpr_params(constexpr int const_a)
{
    return const_a + const_a;
}

int func_internal(int a)
{
    // Error: arg passed through isn't compile-time constant
    return func_with_constexpr_params(a);
//CHECK:                             ^ argument is not a compile-time constant
//CHECK:                             ^ arg 1 in 'func_with_constexpr_params' is not a compile-time constant
}

int func_external(int a)
{
    int2 offset = int2(a, a);
//CHECK:              ^ expected a compile-time constant
    int3 location = int3(0, 0, 0);
    // Load requires constexpr offset
    return input.Load(location, offset);
}

[shader("compute")]
void Test(uint3 thread_index : SV_DispatchThreadID)
{
    int result = 0;

    // Okay, immediate constant
    result += func_with_constexpr_params(1);


    int ii = 0;
    if (thread_index.x > 0)
    {
        ii = 1;
    }
    // Pass non-compile-time-constant argument to function that requires constexpr parameter
    result += func_with_constexpr_params(ii);
//CHECK:                                ^ argument is not a compile-time constant
//CHECK:                                ^ arg 1 in 'func_with_constexpr_params' is not a compile-time constant

    // The nested function calls should issue compile errors
    result += func_internal(ii);
    result += func_external(ii);
}
