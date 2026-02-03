//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that unknown system-value semantics produce an error.

// CHECK: ([[# @LINE+2]]): error 49999: unknown system-value semantic 'SV_Foo'
[shader("vertex")]
float4 main(uint foo : SV_Foo) : SV_Position
{
    return float4(0, 0, 0, 1);
}
