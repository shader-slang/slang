//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=3616dae83508b4ffad03f364c704966c745acf49fe621c5856bbbe38e68899eb
//META: purpose=DebugSource records a source file's path, its embedded text, and a flag saying whether the file was included.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the three operands of the source record under `-g`. The CHECK pins
// the file name string and the trailing include flag (`false` for the primary
// translation unit) with the embedded source text matched loosely in between,
// since that middle operand is the whole file and would otherwise make the
// pattern brittle.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = DebugSource("{{.*}}debug-source-records-path-and-include-flag.slang", "{{.*}}", false)
