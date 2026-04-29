//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that __floatAsInt requires a floating-point type argument

float4 main() : SV_Target
{
    // Error: integer literal is not a floating-point type
    int a = __floatAsInt(42);
//CHECK:    ^^^^^^^^^^^^ bit cast type mismatch
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a half, float, double, or BFloat16 argument

    // Error: integer type is not a floating-point type
    int b = __floatAsInt(int(42));
//CHECK:    ^^^^^^^^^^^^ bit cast type mismatch
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a half, float, double, or BFloat16 argument

    // Error: bool is not a floating-point type
    int c = __floatAsInt(true);
//CHECK:    ^^^^^^^^^^^^ bit cast type mismatch
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a half, float, double, or BFloat16 argument

    return float4(a, b, c, 0);
}
