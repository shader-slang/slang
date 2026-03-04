//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Test that unknown system-value semantics produce an error.

[shader("vertex")]
float4 main(uint foo : SV_Foo) : SV_Position
//CHECK:         ^^^ unknown system-value semantic
//CHECK:         ^^^ unknown system-value semantic 'SV_Foo'
{
    return float4(0, 0, 0, 1);
}
