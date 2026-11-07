// Chromium's WGSL resource table holds only sampled textures and samplers. Under
// `wgsl_chromium_experimental_resource_table`, a handle to any other kind is a compile-time error
// reported through the call that looks the handle up, not WGSL that Tint rejects. See
// shader-slang/slang#13641.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DSTORAGE_TEXTURE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DSTRUCTURED_BUFFER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DRW_STRUCTURED_BUFFER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DBYTE_ADDRESS_BUFFER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DCONSTANT_BUFFER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DCOMBINED

// The error points at the lookup in `read()`, then at the call to `read()` in `main`.
// CHECK: error[E41400]: static assertion failed
// CHECK: the WGSL resource table (chromium_experimental_resource_table) only holds sampled textures and samplers
// CHECK: note: see call to '__wgslGetResourceFromTable'
// CHECK-NEXT: wgsl-resource-table-unsupported.slang:{{[0-9]+}}:
// CHECK-NEXT: |
// CHECK-NEXT: | float4 read() { return res
// CHECK: note: see call to 'read'
// CHECK-NEXT: wgsl-resource-table-unsupported.slang:{{[0-9]+}}:
// CHECK-NEXT: |
// CHECK-NEXT: | outputBuffer[tid.x] = read();

// A constant buffer is dereferenced, and a combined texture-sampler sampled, inside the core
// module, so the lookup note has no location in this file; the call to `read()` is reported.
// CALL: error[E41400]: static assertion failed
// CALL: the WGSL resource table (chromium_experimental_resource_table) only holds sampled textures and samplers
// CALL: note: see call to 'read'
// CALL-NEXT: wgsl-resource-table-unsupported.slang:{{[0-9]+}}:
// CALL-NEXT: |
// CALL-NEXT: | outputBuffer[tid.x] = read();

RWStructuredBuffer<float4> outputBuffer;

#if defined(STORAGE_TEXTURE)
uniform RWTexture2D<float4>.Handle res;
float4 read() { return res[uint2(0)]; }
#elif defined(STRUCTURED_BUFFER)
uniform StructuredBuffer<float4>.Handle res;
float4 read() { return res[0]; }
#elif defined(RW_STRUCTURED_BUFFER)
uniform RWStructuredBuffer<float4>.Handle res;
float4 read() { return res[0]; }
#elif defined(BYTE_ADDRESS_BUFFER)
uniform ByteAddressBuffer.Handle res;
float4 read() { return res.Load<float4>(0); }
#elif defined(CONSTANT_BUFFER)
uniform ConstantBuffer<float4>.Handle res;
float4 read() { return *res; }
#elif defined(COMBINED)
uniform Sampler2D<float4>.Handle res;
float4 read() { return res.SampleLevel(float2(0), 0); }
#endif

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = read();
}
