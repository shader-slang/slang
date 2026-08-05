//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=fede71bd73c83957c94e27eab0113790d3ae8864e230db8dde234dca52b8d824
//META: purpose=Expansion: Vec composes with Double at the standard upper element count to form Vec(Double, 4 : Int) on a global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with Double at the standard upper element count to form
// Vec(Double, 4 : Int) on a global_param. `double4` exercises the
// standard upper element count with the 64-bit element, on a freshly
// named user `global_param`. (An existing `vec-double4-largest-
// element.slang` boundary uses the same anchor; this is the
// global_param-shape parallel that matches the half/double family-
// expansion pattern of this expansion pass.)

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 q;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = q.x + q.w;
}

// IR: let {{.*}}%q{{.*}}: Vec(Double, 4 : Int){{.*}}= global_param
