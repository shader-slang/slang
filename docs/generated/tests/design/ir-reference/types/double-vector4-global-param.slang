//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Expansion: Vec composes with Double at the standard upper element count to form Vec(Double, 4 : Int) on a global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc admits any scalar in `Vec(elementType, elementCount)`.
// `double4` exercises the standard upper element count with the
// 64-bit element, on a freshly named user `global_param`. (An
// existing `vec-double4-largest-element.slang` boundary uses the
// same anchor; this is the global_param-shape parallel that
// matches the half/double family-expansion pattern of this
// expansion pass.)

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 q;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = q.x + q.w;
}

// IR: let {{.*}}%q{{.*}}: Vec(Double, 4 : Int){{.*}}= global_param
