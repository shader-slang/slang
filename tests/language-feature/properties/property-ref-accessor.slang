// property-ref-accessor.slang
//
// A write through a property `ref` accessor stores into the referenced field,
// and a read-modify-write (`++`, `+=`) through it observes and updates that same
// field. This exercises the accessor as a genuine reference to storage.
// https://github.com/shader-slang/slang/issues/12487, #9636

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Cell
{
    int _v;
    property int value
    {
        get { return _v; }
        [mutating] ref { return _v; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Cell c;
    c._v = 0;

    c.value = 13;
    // CHECK: 13
    outputBuffer[0] = c.value;

    c.value++;
    // CHECK-NEXT: 14
    outputBuffer[1] = c.value;

    c.value += 20;
    // CHECK-NEXT: 34
    outputBuffer[2] = c.value;

    // A plain field read sees the writes the ref accessor made.
    // CHECK-NEXT: 34
    outputBuffer[3] = c._v;
}
