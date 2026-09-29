// ir-type-alignment-attr-existential.slang

// Regression test for the attribute-ordering invariant of `IRTypeAlignmentAttr`.
//
// `IRTypeLayout::getSizeAttrs()` uses `findAttrs<IRTypeSizeAttr>()`, which relies
// on all size attributes forming one contiguous run of operands (it stops at the
// first operand of a different type). The type-layout builder must therefore emit
// every `IRTypeSizeAttr` before any `IRTypeAlignmentAttr`; interleaving an
// alignment attribute between two size attributes would silently truncate the
// size-attribute enumeration — and, for a layout mixing byte size with a resource
// kind, drop the resource entries and miscompile its binding.
//
// This bites specifically when a layout has byte size *with* a byte alignment > 1
// (so an alignment attribute is emitted) plus a higher-numbered resource kind.
// `struct S` here has `Uniform` size 48 aligned to 16 (from the `float4`) plus two
// existential-parameter sizes, so its `structTypeLayout` carries three size attrs
// and one alignment attr. The check below pins the operand order: the three size
// operands must come first, contiguously, and the alignment operand must follow
// them — the exact ordering the interleaving bug violated.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry main -dump-ir -o -

interface ILight
{
    float getIntensity();
}

struct S
{
    float4 color;
    ILight light;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.color.x;
}

// Capture the first four operands of `S`'s `structTypeLayout` (three size attrs
// followed by the alignment attr), then confirm each operand's kind by its
// definition. Under the interleaving bug the alignment attr would be the second
// operand, so `[[SIZE1]] ... = size` on the second captured operand would fail.
//CHECK: structTypeLayout([[SIZE0:%[0-9]+]], [[SIZE1:%[0-9]+]], [[SIZE2:%[0-9]+]], [[ALIGN:%[0-9]+]]
//CHECK-DAG: [[SIZE0]]{{.*}}= size(
//CHECK-DAG: [[SIZE1]]{{.*}}= size(
//CHECK-DAG: [[SIZE2]]{{.*}}= size(
//CHECK-DAG: [[ALIGN]]{{.*}}= TypeAlignment(16 : Int)
