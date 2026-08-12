// subscript-ref-accessor-write.slang
//
// A write through a `__subscript` `ref` accessor stores into the element the
// index selects, and a read-modify-write through it updates that same element.
// The subscript form carries an index argument (the `BoundStorage` path),
// exercising a code path distinct from the property form in
// property-ref-accessor.slang.
// https://github.com/shader-slang/slang/issues/12487, #9636

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Buf
{
    int data[4];

    __subscript(int index) -> int
    {
        [mutating] ref { return data[index]; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Buf b;
    for (int i = 0; i < 4; ++i)
        b.data[i] = 0;

    b[0] = 7;
    b[2] = 9;
    // CHECK: 7
    outputBuffer[0] = b.data[0];
    // CHECK-NEXT: 9
    outputBuffer[1] = b.data[2];

    b[0] += 3;
    // CHECK-NEXT: 10
    outputBuffer[2] = b.data[0];
    // A neighbouring element the writes never touched is unchanged.
    // CHECK-NEXT: 0
    outputBuffer[3] = b.data[1];
}
