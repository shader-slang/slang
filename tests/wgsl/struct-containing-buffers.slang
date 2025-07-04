//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl

// Test that struct types containing buffer fields are properly filtered
// from function parameters and converted to globals

struct BufferContainer
{
    StructuredBuffer<float> dataBuffer;
    RWStructuredBuffer<int> indexBuffer;
}

struct ComplexData
{
    float4 values[4];  // Should emit as array<vec4<f32>, 4>
    int count;
}

// This should be converted to global variables
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read> buffers_dataBuffer_0 : array<f32>;
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> buffers_indexBuffer_0 : array<i32>;
BufferContainer buffers;

// Function taking struct with buffers should have parameter filtered
// WGSL: fn processData_0( _S1 : u32) -> f32
float processData(BufferContainer container, uint index)
{
    float data = container.dataBuffer[index];
    int idx = container.indexBuffer[index];
    return data * float(idx);
}

// Function with non-buffer struct should keep parameters
float processComplex(ComplexData data, float scale)
{
    return data.values[0].x * scale;
}

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    
    // Call with buffer-containing struct should filter buffer argument
    // WGSL: processData_0({{.*}})
    float result1 = processData(buffers, index);
    
    // Call with regular struct should keep all arguments  
    ComplexData complex;
    complex.count = 42;
    complex.values[0] = float4(1, 2, 3, 4);
    // WGSL: processComplex_0({{.*}}, 2.0f)
    float result2 = processComplex(complex, 2.0);
    
    // Use the results to prevent optimization
    if (result1 > result2) {
        buffers.indexBuffer[index] = int(result1);
    }
}