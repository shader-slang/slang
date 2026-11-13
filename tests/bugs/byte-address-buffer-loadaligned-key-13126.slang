// Regression test for shader-slang/slang#13126: buffer-load-arg specialization must key on the
// preserved alignment operand.
//
// Two `LoadAligned` calls that differ only in their alignment promise must be specialized into
// separate clones. If they shared one clone, the second call would get the first call's alignment.
// `struct S { float4 values[2]; }` is deferred-loaded, so each call gets a clone of `S.get` that
// takes the runtime offset as a parameter. The checks require a clone loading with alignment 16 and
// a clone loading with alignment 4.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -dump-ir-after specializeFuncsForBufferLoadArgs -o -

struct S
{
    float4 values[2];
    float get() { return values[0].x + values[1].w; }
}

ByteAddressBuffer input;
RWStructuredBuffer<float> output;
uniform uint dynOffset;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input.LoadAligned<S>(dynOffset, 16).get();
    output[1] = input.LoadAligned<S>(dynOffset, 4).get();
    // The clones of `S.get` follow the entry point in the dump, so matching after the first clone
    // excludes the original call-site loads. Each clone holds exactly one load, so finding both a
    // 16-aligned and a 4-aligned load there requires two clones with different alignments.
    // CHECK: func %computeMain
    // CHECK: func %{{.*}}get{{.*}}Func(Float, UInt)
    // CHECK-DAG: byteAddressBufferLoad(%input, {{[^,]*}}, 16 : UInt)
    // CHECK-DAG: byteAddressBufferLoad(%input, {{[^,]*}}, 4 : UInt)
}
