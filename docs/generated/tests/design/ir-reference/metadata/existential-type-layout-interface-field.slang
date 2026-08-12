//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=8133b77a147a20aa12fcc20641167e9fc2f42707469a60ef2b36207749f54a95
//META: purpose=An interface-typed field of a laid-out struct produces an existentialTypeLayout record.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the existential branch of type-layout lowering is reachable from
// ordinary Slang source: an interface-typed member of a constant-buffer struct
// is the value whose layout is not known statically. The CHECK pins the
// `existentialTypeLayout` opcode together with the enclosing struct layout, so
// the test fails if the interface member silently degrades to the generic
// `typeLayout` fallback.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

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
RWStructuredBuffer<float> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb.color.x;
}

// CHECK-DAG: = structTypeLayout(
// CHECK-DAG: = existentialTypeLayout(
