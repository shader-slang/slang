//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute -preserve-params

// Indexing an array of byte-address buffers, or an array of arrays, produces one structured-buffer
// array declaration at the array's binding, with no leftover byte-address buffer array, even
// when -preserve-params keeps every parameter alive (#14039).

ByteAddressBuffer usedArray[2];
ByteAddressBuffer usedNestedArray[2][3];
RWStructuredBuffer<uint> outputBuffer;

// The equivalent structured-buffer array is created before the original, so it takes the `_0`
// suffix. A leftover byte-address buffer array would be a second block that also ends in
// `} usedArray_N[2];`, with a higher suffix. The checks therefore allow only the `_0` block of
// each array, anywhere in the output.

// CHECK-NOT: } usedArray_{{[1-9][0-9]*}}[2];
// CHECK-NOT: } usedNestedArray_{{[1-9][0-9]*}}[2][3];
// CHECK: } usedArray_0[2];
// CHECK-NOT: } usedArray_{{[0-9]+}}[2];
// CHECK-NOT: } usedNestedArray_{{[1-9][0-9]*}}[2][3];
// CHECK: } usedNestedArray_0[2][3];
// CHECK-NOT: } usedArray_{{[0-9]+}}[2];
// CHECK-NOT: } usedNestedArray_{{[0-9]+}}[2][3];

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[0] = usedArray[tid.x].Load(0) + usedNestedArray[1][tid.x].Load(4);
}
