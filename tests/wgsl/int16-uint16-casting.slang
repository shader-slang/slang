//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// Test casting between 16-bit and 32-bit integers in WGSL

//WGSL-DAG: var<storage, read_write> buffer_0 : array<i32>;
//WGSL-DAG: @compute
//WGSL-DAG: @workgroup_size(1, 1, 1)

RWStructuredBuffer<int> buffer;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test casting 32-bit to 16-bit and back
    uint val32 = dispatchThreadID.x;
    uint16_t val16 = uint16_t(val32);
    uint result = uint(val16);
    
    buffer[0] = int(result);
    
    // Test signed variant
    int sval32 = int(dispatchThreadID.y);
    int16_t sval16 = int16_t(sval32);
    int sresult = int(sval16);
    
    buffer[1] = sresult;
}