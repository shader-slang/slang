//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// Test that int16_t and uint16_t types are supported in WGSL by promotion to 32-bit

//WGSL-DAG: var<storage, read_write> buffer_0 : array<i32>;
//WGSL-DAG: @compute
//WGSL-DAG: @workgroup_size(1, 1, 1)
//WGSL-DAG: i32({{.*}})
//WGSL-DAG: u32({{.*}})

RWStructuredBuffer<int> buffer;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test uint16_t
    uint16_t test_u16 = uint16_t(dispatchThreadID.x);
    buffer[0] = test_u16;
    
    // Test int16_t
    int16_t test_i16 = int16_t(dispatchThreadID.y);
    buffer[1] = test_i16;
    
    // Test arithmetic operations
    uint16_t sum_u16 = uint16_t(test_u16 + test_i16);
    buffer[2] = sum_u16;
    
    // Test constants
    int16_t const_i16 = 42;
    uint16_t const_u16 = 65535;
    buffer[3] = const_i16;
    buffer[4] = const_u16;
}