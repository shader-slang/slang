//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test that __floatAsInt requires a compile-time constant expression

float getFloat()
{
    return 1.0f;
}

cbuffer U
{
    float uniformFloat;
}

float4 main() : SV_Target
{
    float localFloat = 1.0f;
    
    // Error: function call result is not a compile-time constant
    // CHECK: ([[# @LINE+1]]): error 30085:
    int a = __floatAsInt(getFloat());

    // Error: uniform variable is not a compile-time constant
    // CHECK: ([[# @LINE+1]]): error 30085:
    int b = __floatAsInt(uniformFloat);

    // Error: local variable is not a compile-time constant
    // CHECK: ([[# @LINE+1]]): error 30085:
    int c = __floatAsInt(localFloat);

    return float4(a, b, c, 0);
}
