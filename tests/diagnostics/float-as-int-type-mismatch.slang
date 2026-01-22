//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test that __floatAsInt requires a floating-point type argument

float4 main() : SV_Target
{
    // Error: integer literal is not a floating-point type
    // CHECK: ([[# @LINE+1]]): error 30084:
    int a = __floatAsInt(42);

    // Error: integer type is not a floating-point type
    // CHECK: ([[# @LINE+1]]): error 30084:
    int b = __floatAsInt(int(42));

    // Error: bool is not a floating-point type
    // CHECK: ([[# @LINE+1]]): error 30084:
    int c = __floatAsInt(true);

    return float4(a, b, c, 0);
}
