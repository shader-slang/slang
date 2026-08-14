//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#runtime-type-information
//META: doc_section_digest=10ae9cc79d86ded1f6703277629437a1410db2756cb171fadd23944d8b2ba02f
//META: purpose=A non-default binding index reaches GetDynamicResourceHeap as its literal operand rather than being normalised to zero.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on the binding-index operand, whose default is zero. Requesting
// binding 2 must survive into the inst's operand list; the CHECK pins the
// literal `2 : UInt`, so a dropped or defaulted index would fail.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    let heap = __getDynamicResourceHeap<Texture2D<float4> >(2);
    buf[0] = int(heap[inVal].Load(int3(0, 0, 0)).x);
}

// CHECK: GetDynamicResourceHeap(2 : UInt)
