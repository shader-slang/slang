//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#lowerexpandtype
//META: doc_section_digest=2af9384f52b762a61f92e97f07d8418049eb9e1a371668e59b67a4bec604335f
//META: purpose=Phase B's lowerExpandType (B5) rewrites IRExpandType so the pattern type becomes a child yielded from inside IRExpand rather than a same-level sibling.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shape change B5 performs on variadic-generic types. A
// generic taking `expand Box<each T>` produces an expand-type at lowering
// time; after B5 the snapshot shows an `Expand` inst whose body block
// yields the specialised pattern, which is exactly "the pattern nested
// inside the expand". The CHECK-NOT asserts no free-standing `ExpandType`
// spelling is left, so specialisation only ever has to handle `Expand`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct Box<T> { T v; }

int count<each T>(expand Box<each T> boxes) { return 7; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Box<int> x;
    Box<float> y;
    buf[0] = count(x, y);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-NOT: ExpandType
// CHECK: Expand %{{[A-Za-z0-9_]+}} : Type
// CHECK: yield(specialize(
