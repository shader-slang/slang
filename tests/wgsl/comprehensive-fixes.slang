//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Comprehensive test covering all the WGSL generation fixes:
// 1. Array size syntax (no i32() cast)
// 2. Buffer parameter filtering 
// 3. Global storage buffer generation
// 4. Function call argument filtering

struct DataArray
{
    float values[16];    // Should emit as array<f32, 16>, not array<f32, i32(16)>
    int indices[8];      // Should emit as array<i32, 8>, not array<i32, i32(8)>
};

struct ProcessorState
{
    // These buffers should be filtered from function parameters
    RWStructuredBuffer<DataArray> inputData;
    RWStructuredBuffer<float> outputData;
    StructuredBuffer<int> indexMapping;
}

// Global buffers should be generated with proper bindings
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> {{.*}} : array<DataArray{{.*}}>;
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> {{.*}} : array<f32>;
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read> {{.*}} : array<i32>;
ProcessorState processor;

// Additional standalone buffer
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read> constantData_0 : array<vec4<f32>>;
StructuredBuffer<float4> constantData;

// Functions with buffer parameters should have them filtered
// WGSL: fn processArray_0( _S{{[0-9]+}} : u32, _S{{[0-9]+}} : f32) -> f32
float processArray(ProcessorState state, uint index, float scale)
{
    DataArray data = state.inputData[index];
    int mappedIndex = state.indexMapping[index % 8];
    float result = data.values[mappedIndex] * scale;
    state.outputData[index] = result;
    return result;
}

// Function with mixed parameters (some buffers, some regular)
// WGSL: fn computeAverage_0( _S{{[0-9]+}} : u32, _S{{[0-9]+}} : f32) -> f32
float computeAverage(StructuredBuffer<float4> constants, uint count, float multiplier)
{
    float sum = 0.0;
    for (uint i = 0; i < count; i++)
    {
        sum += constants[i].x;
    }
    return (sum / float(count)) * multiplier;
}

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    
    // Function calls should only pass non-buffer arguments
    // WGSL: processArray_0({{.*}}, 2.5f)
    float result1 = processArray(processor, index, 2.5);
    
    // WGSL: computeAverage_0(u32(16), 1.5f)
    float result2 = computeAverage(constantData, 16, 1.5);
    
    // Direct array access should work with fixed sizes
    DataArray local;
    local.values[0] = result1;    // array<f32, 16>
    local.indices[0] = int(result2);  // array<i32, 8>
}