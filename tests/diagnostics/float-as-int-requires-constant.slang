//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
    int a = __floatAsInt(getFloat());
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    // Error: uniform variable is not a compile-time constant
    int b = __floatAsInt(uniformFloat);
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    // Error: local variable is not a compile-time constant
    int c = __floatAsInt(localFloat);
//CHECK:    ^^^^^^^^^^^^ bit cast requires constant
//CHECK:    ^^^^^^^^^^^^ '__floatAsInt' requires a compile-time constant floating-point expression

    return float4(a, b, c, 0);
}
