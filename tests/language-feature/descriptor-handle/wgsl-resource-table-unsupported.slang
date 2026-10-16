// Chromium's WGSL resource table holds only sampled textures and samplers. Under
// `wgsl_chromium_experimental_resource_table`, a handle to any other kind is a compile-time error
// at the use site, not WGSL that Tint rejects. See shader-slang/slang#13641.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DSTORAGE_TEXTURE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DSTRUCTURED_BUFFER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DCONSTANT_BUFFER

// CHECK: error[E41400]: static assertion failed
// CHECK: the WGSL resource table (chromium_experimental_resource_table) only holds sampled textures and samplers
// CHECK: see call to '__wgslGetResourceFromTable'
// CHECK: wgsl-resource-table-unsupported.slang

RWStructuredBuffer<float4> outputBuffer;

#if defined(STORAGE_TEXTURE)
uniform RWTexture2D<float4>.Handle res;
float4 read() { return res[uint2(0)]; }
#elif defined(STRUCTURED_BUFFER)
uniform StructuredBuffer<float4>.Handle res;
float4 read() { return res[0]; }
#elif defined(CONSTANT_BUFFER)
uniform ConstantBuffer<float4>.Handle res;
float4 read() { return *res; }
#endif

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = read();
}
