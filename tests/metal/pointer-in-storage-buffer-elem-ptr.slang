//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Exercise RWStructuredBufferGetElementPtr on a pointer-element buffer by
// passing an element by inout reference. This forces the compiler to take
// the address of the buffer element rather than using Load/Store directly.
//
// See: https://github.com/shader-slang/slang/issues/10841

RWStructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

void swapWithNext(inout int* slot, int* next)
{
    int* tmp = slot;
    slot = next;
    output[0] = *tmp;
}

// CHECK: ulong device*
// CHECK-LABEL: computeMain
// CHECK: int device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* nextPtr = ptrs[1];
    swapWithNext(ptrs[0], nextPtr);
}
