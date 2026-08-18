//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#stripfrontendonlyinstructions
//META: doc_section_digest=6d9c3e171509715bc06e848e926d607a7687b1fec0c44f072b66721b314e4dc1
//META: purpose=Phase D's stripFrontEndOnlyInstructions (D10) removes IRNameHintDecoration only when the obfuscation option sets shouldStripNameHints, and never clears source locations.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the conditional half of the stripping list against its default.
// Both runs compile the same module; the default run must still show the
// `nameHint` decorations that carry user-chosen identifiers into the IR,
// and the `-obfuscate` run must show none of them between the snapshot
// header and the lowered function, which still appears.

//TEST:SIMPLE(filecheck=KEEP):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=STRIP):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -obfuscate

uniform int userNamedGlobal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = userNamedGlobal + 1;
}

// KEEP-LABEL: ### LOWER-TO-IR:
// KEEP: nameHint("userNamedGlobal")

// STRIP-LABEL: ### LOWER-TO-IR:
// STRIP-NOT: nameHint(
// STRIP: {{^}}func %
