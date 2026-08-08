//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#shader64bitindexing
//META: doc_section_digest=15a476c52c9de445ea6e2a505dab18d19a9a1c21a561638e852d1127d3079f22
//META: purpose=A [Shader64BitIndexing] attribute written on a callee is lifted onto the calling entry point, which is where the shader64BitIndexing decoration appears.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lifting behaviour: the attribute is written on `helper`, yet
// the decoration must end up on `main`, because the matching execution mode
// is entry-point scoped and would be invalid on a callee. The CHECK asserts
// the decoration precedes `func %main`; if it had stayed on the callee the
// `func %main` line would be reached without ever matching it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[Shader64BitIndexing]
void helper(uint3 tid)
{
    buf[0] = int(tid.x);
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    helper(tid);
}

// CHECK: [shader64BitIndexing]
// CHECK: [nameHint("main")]
// CHECK: func %main
