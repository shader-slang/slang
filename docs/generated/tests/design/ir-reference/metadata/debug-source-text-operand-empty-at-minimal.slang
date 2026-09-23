//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugsource
//META: doc_section_digest=c3e1ce68343ea764de69074a2743917e1632704b0d47873ab2035e3408e6691d
//META: purpose=Compiling at -g1 leaves the DebugSource text operand an empty string, so the record carries the path without a copy of the file.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the absent form of the source-text operand. The record keeps three
// operands at every level, so the only way to show the text is genuinely empty
// -- rather than merely elided by the printer -- is to pin the two quotes with
// nothing between them and the include flag straight after. The companion
// observation is that this is what makes a `-g1` dump safe to pattern-match:
// nothing in the dump echoes the patterns being matched against it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// The middle operand is `""` -- the two quote characters are adjacent.
// CHECK: = DebugSource("{{.*}}debug-source-text-operand-empty-at-minimal.slang", "", false)
