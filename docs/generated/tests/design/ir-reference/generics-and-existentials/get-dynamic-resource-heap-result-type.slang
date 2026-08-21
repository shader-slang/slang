//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#runtime-type-information
//META: doc_section_digest=10ae9cc79d86ded1f6703277629437a1410db2756cb171fadd23944d8b2ba02f
//META: purpose=The dynamic-resource-heap intrinsic lowers to GetDynamicResourceHeap(bindingIndex) whose result type is an unsized array of the descriptor type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the descriptor-heap accessor's operand and result shape. The heap is
// indexed by a runtime value and the sampled texel is written to a buffer so
// nothing is dead-stripped before the dump. The CHECK asserts the inst takes
// the binding index as a literal operand and produces an array of the
// requested descriptor type rather than a single descriptor.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    let heap = __getDynamicResourceHeap<Texture2D<float4> >(0);
    buf[0] = int(heap[inVal].Load(int3(0, 0, 0)).x);
}

// CHECK: let  %{{[0-9]+}} : UnsizedArray(TextureType({{.*}})) = GetDynamicResourceHeap(0 : UInt)
