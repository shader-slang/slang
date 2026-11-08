//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute -preserve-params

// With -preserve-params, GLSL declares each unused byte-address buffer as the `uint`
// structured-buffer block that a used one gets, at its original binding (#14039).

ByteAddressBuffer unusedBuffer;
globallycoherent RWByteAddressBuffer unusedCoherentBuffer;
RWStructuredBuffer<uint> outputBuffer;

// CHECK-DAG: layout(std430, binding = 0) readonly buffer StructuredBuffer_uint_t_{{[0-9]+}} {
// CHECK-DAG: } unusedBuffer_0;
// CHECK-DAG: coherent layout(std430, binding = 1) buffer StructuredBuffer_uint_t_{{[0-9]+}} {
// CHECK-DAG: } unusedCoherentBuffer_0;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1;
}
