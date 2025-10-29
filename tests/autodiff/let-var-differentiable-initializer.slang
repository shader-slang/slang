// Test that let and var declarations without explicit types preserve differentiable
// context when checking initializer expressions. This ensures that calls in
// initializers (like dot) get properly marked as differentiable and can propagate
// derivatives correctly.
//
// Without the fix, let/var declarations without explicit types would fail to compile
// with errors like "derivative cannot be propagated through call to non-backward-differentiable function `dot`".
//
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float test_let_without_type(float3 q)
{
    let x = dot(q, q);
    if (x < 0) {
        // x is immutable, so we just return a default value if needed
        return 0.0;
    }
    return x;
}

[Differentiable]
float test_var_without_type(float3 q)
{
    var x = dot(q, q);
    if (x < 0) {
        x = 0.0;
    }
    return x;
}

[Differentiable]
float test_let_with_type(float3 q)
{
    float x = dot(q, q);
    if (x < 0) {
        // x is immutable, so we can't modify it
    }
    return x;
}

[Differentiable]
float test_var_with_type(float3 q)
{
    const float x = dot(q, q);
    if (x < 0) {
        // x is immutable, so we can't modify it
    }
    return x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float3 q = float3(1.0, 2.0, 3.0);
    
    // Test that all variants compile and can be differentiated
    // Expected: q·q = 1² + 2² + 3² = 1 + 4 + 9 = 14
    outputBuffer[0] = test_let_without_type(q);      // Expect: 14.0
    outputBuffer[1] = test_var_without_type(q);      // Expect: 14.0
    outputBuffer[2] = test_let_with_type(q);         // Expect: 14.0
    outputBuffer[3] = test_var_with_type(q);         // Expect: 14.0
}
