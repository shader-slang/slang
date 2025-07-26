//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Comprehensive test combining all WGSL generation fixes:
// 1. Array size syntax (no i32() cast)
// 2. Buffer parameter filtering 
// 3. Global storage buffer generation
// 4. Function call argument filtering
// 5. Switch case syntax (no type casts)

struct DataStruct
{
    float values[8];     // Should emit as array<f32, 8>, not array<f32, i32(8)>
    int indices[4];      // Should emit as array<i32, 4>, not array<i32, i32(4)>
    uint typeId;
}

// Buffer should become global with binding
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> dataBuffer_0 : array<DataStruct_std430_0>;
RWStructuredBuffer<DataStruct> dataBuffer;

// Function with buffer parameter should have it filtered
// WGSL: fn processData_0( _S{{[0-9]+}} : u32, _S{{[0-9]+}} : f32) -> f32
float processData(RWStructuredBuffer<DataStruct> buffer, uint index, float multiplier)
{
    DataStruct data = buffer[index];
    
    // Switch statement should emit case values without type casts
    float result;
    switch (data.typeId)
    {
        // WGSL: case 0:
        case 0:
            result = data.values[0] * multiplier;
            break;
        // WGSL: case 1:
        case 1:
            result = data.values[1] * multiplier * 2.0;
            break;
        // WGSL: case 2, 3:
        case 2:
        case 3:
            result = data.values[2] + data.values[3];
            break;
        default:
            result = 0.0;
            break;
    }
    
    return result;
}

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    
    // Function call should only pass non-buffer arguments
    // WGSL: processData_0({{.*}}, 2.5f)
    float result = processData(dataBuffer, index, 2.5);
    
    // Direct struct access should work with proper array syntax
    DataStruct local;
    local.values[0] = result;        // array<f32, 8>
    local.indices[0] = int(result);  // array<i32, 4>
    local.typeId = index % 4;
    
    dataBuffer[index % 16] = local;
}