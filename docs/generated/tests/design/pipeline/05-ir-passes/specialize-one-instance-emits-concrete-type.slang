//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=2f10610b6a8497f9b2cec62d411162a4585b32e9698a95e5288845967ffba16d
//META: purpose=Boundary: a generic with exactly one call-site instantiation produces exactly one concrete specialization — the `### AFTER specializeModule` dump shows a `Func(Float, Float, Float)` signature.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickFirst<T>` is called exactly once with `T = float`. After the
// specializeModule pass the function carries a single concrete
// `Func(Float, Float, Float)` signature.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after specializeModule -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickFirst(a, a + 1.0f);
}

// CHECK-LABEL: ### AFTER specializeModule:
// CHECK: func %pickFirst{{[^:]*}}: Func(Float, Float, Float)
