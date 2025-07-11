//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

// Test that int16_t and uint16_t types emit diagnostics only once per type to prevent duplicates

//CHECK: error 56103
//CHECK: 16-bit integer type 'uint16_t' is not supported by the WGSL backend

//CHECK: error 56103
//CHECK: 16-bit integer type 'int16_t' is not supported by the WGSL backend

RWStructuredBuffer<int> buffer;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test uint16_t - should produce error (first occurrence)
    uint16_t test_u16 = uint16_t(dispatchThreadID.x);
    buffer[0] = test_u16;
    
    // Test int16_t - should produce error (first occurrence of int16_t)
    int16_t test_i16 = int16_t(dispatchThreadID.y);
    buffer[1] = test_i16;
    
    // Use uint16_t again - should not produce duplicate error
    uint16_t another_u16 = uint16_t(42);
    buffer[2] = another_u16;
    
    // Use int16_t again - should not produce duplicate error
    int16_t another_i16 = int16_t(123);
    buffer[3] = another_i16;
}