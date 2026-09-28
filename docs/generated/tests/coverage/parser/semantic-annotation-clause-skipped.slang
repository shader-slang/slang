//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an angle-bracketed annotation clause written after a semantic is skipped wholesale, leaving the semantic itself intact.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `_parseOptSemantics` parses `: SEMANTIC` and then, if the
// next token is `<`, discards every token up to the matching `>` without
// interpreting any of it. This is the legacy D3D effect-annotation slot, and
// unlike the declarator-level annotation skip it needs no command-line flag
// and does no disambiguation at all -- a `<` after a semantic is always an
// annotation.
//
// The annotation below contains a `;` and a string literal, neither of which
// is valid in any other position here, which is what makes the skip visible.
// CHECK confirms `SV_Position` still reached the emitted HLSL and that nothing
// from inside the angle brackets did.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage vertex

struct VOut
{
    float4 pos : SV_Position < int hint = 3; string s = "x"; >;
};

//CHECK: struct VOut_0
//CHECK: float4 pos_0 : SV_Position;

VOut main()
{
    VOut o;
    o.pos = float4(1, 2, 3, 4);
    return o;
}
