// Regression test for shader-slang/slang#13126: buffer-load-arg specialization must key on the
// preserved alignment operand.
//
// Two `LoadAligned` calls that differ only in their alignment promise must be specialized into
// separate clones; otherwise they collapse into one clone and a call inherits whichever alignment
// was specialized first (a source-order-dependent miscompile of the later wide-vs-scalarized
// decision). `struct S { float4 values[2]; }` is deferred-loaded, and a runtime offset routes the
// alignment promise into that decision, so the collapse is observable. With correct keying each
// call keeps its own alignment operand: after this pass there are four `byteAddressBufferLoad`
// insts -- the two original call-site loads plus one in each of the two specialized clones -- two
// carrying alignment 16 and two carrying alignment 4. A collapse merges the clones, leaving three.

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
    // Both alignments must survive on distinct loads (two carrying 16, two carrying 4); a collapse
    // loses one of the alignment values.
    // CHECK-DAG: byteAddressBufferLoad(%input, {{.*}}, 16 : UInt)
    // CHECK-DAG: byteAddressBufferLoad(%input, {{.*}}, 16 : UInt)
    // CHECK-DAG: byteAddressBufferLoad(%input, {{.*}}, 4 : UInt)
    // CHECK-DAG: byteAddressBufferLoad(%input, {{.*}}, 4 : UInt)
}
