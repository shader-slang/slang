//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#option-set-toggles
//META: doc_section_digest=7c951a88b67c0d94a6d4504f2d7616fdfe324b58f96f2245c67178d693892a88
//META: purpose=The DebugInfoIncludeSource toggle widens A4 so IRDebugSource carries the source text at Minimal debug-info level, without promoting the debug-info level itself.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-debug-info-include-source` is a widener for A4 rather
// than a level promotion. Both compilations run at `-g1` (Minimal), where
// the `DebugSource` inst is emitted either way; the difference is its
// second operand. Plain `-g1` leaves that operand an empty string, and the
// regex CHECKs distinguish an empty second operand from a non-empty one.
// The MIN run additionally asserts the compilation-unit inst that only the
// Standard level would add is still absent, so the flag really did not
// raise the level; the INC run cannot repeat that assertion because the
// embedded source text itself now contains the inst's name.

//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1
//TEST:SIMPLE(filecheck=INC):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1 -debug-info-include-source

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// Minimal level alone: the text operand is the empty string.
// MIN-LABEL: ### LOWER-TO-IR:
// MIN: DebugSource("{{.*}}", "", {{.*}})
// MIN-NOT: DebugCompilationUnit(

// Minimal level plus the include-source toggle: the text operand carries
// the shader source, so the entry-point declaration is quoted inside it.
// INC-LABEL: ### LOWER-TO-IR:
// INC: DebugSource("{{.*}}", "{{.+}}numthreads(1, 1, 1){{.+}}", {{.*}})
