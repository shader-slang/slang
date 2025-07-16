// sign-wgsl.slang

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute

// Test that sign function returns correct type for WGSL
// WGSL's sign returns float but Slang's sign should return int

struct OutputBuffer
{
    int scalar_result;
    int2 vector_result;
};

[[vk::binding(0, 0)]]
RWStructuredBuffer<OutputBuffer> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    // Test scalar sign function
    int scalar_result = sign(0.5);
    
    // Test vector sign function  
    int2 vector_result = sign(float2(1.5, -0.5));
    
    outputBuffer[0].scalar_result = scalar_result;
    outputBuffer[0].vector_result = vector_result;
    
    // WGSL-DAG: i32(sign(
    // WGSL-DAG: vec2<i32>(sign((vec2<f32>
}