//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -stage compute -entry computeMain

// Test that int8_t and uint8_t types emit diagnostics only once per type to prevent duplicates

//CHECK: 8-bit integers not supported in WGSL
//CHECK: 8-bit integer type 'uint8_t' is not supported by the WGSL backend.

//CHECK: 8-bit integers not supported in WGSL
//CHECK: 8-bit integer type 'int8_t' is not supported by the WGSL backend.

RWStructuredBuffer<int> buffer;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test uint8_t - should produce error (first occurrence)
    uint8_t test_u8 = uint8_t(dispatchThreadID.x);
    buffer[0] = test_u8;

    // Test int8_t - should produce error (first occurrence of int8_t)
    int8_t test_i8 = int8_t(dispatchThreadID.y);
    buffer[1] = test_i8;

    // Use uint8_t again - should not produce duplicate error
    uint8_t another_u8 = uint8_t(42);
    buffer[2] = another_u8;

    // Use int8_t again - should not produce duplicate error
    int8_t another_i8 = int8_t(123);
    buffer[3] = another_i8;
}
