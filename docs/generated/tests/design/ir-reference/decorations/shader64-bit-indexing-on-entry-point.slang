//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#shader64bitindexing
//META: doc_section_digest=15a476c52c9de445ea6e2a505dab18d19a9a1c21a561638e852d1127d3079f22
//META: purpose=A [Shader64BitIndexing] attribute written on the entry point itself produces a nullary shader64BitIndexing decoration on that entry-point function.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the first of the three spellings the doc says are handled
// uniformly, and the decoration's nullary shape. The CHECK pins the bare
// `[shader64BitIndexing]` line followed by `func %main`, which is the
// entry-point-scoped placement the SPIR-V execution mode requires.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[Shader64BitIndexing]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [shader64BitIndexing]
// CHECK: [nameHint("main")]
// CHECK: func %main
