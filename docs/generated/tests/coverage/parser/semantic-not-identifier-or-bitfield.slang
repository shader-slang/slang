//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies that a token after `:` which is neither a semantic name nor a bit-field width is reported as E20001 expecting an identifier.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `ParseSemantic` handles the `:` slot on a member with a
// chain of look-aheads -- `register`, `packoffset`, the ray-payload
// read/write forms, a bare identifier for an ordinary semantic, and an
// integer literal for a bit-field width. When none of those match it falls
// through to a branch whose only job is to produce a diagnostic: it calls
// `ReadToken(Identifier)` purely for the error it raises, then returns null so
// no modifier is attached.
//
// A floating-point literal is the simplest token that reaches that branch --
// close enough to the bit-field form to get past the earlier look-aheads'
// intent, but not an integer literal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage vertex

struct VOut
{
    float4 pos : 1.5;
//CHECK:         ^^^ unexpected token
//CHECK:         ^^^ unexpected floating-point literal, expected identifier
};

VOut main()
{
    VOut o;
    o.pos = float4(1, 2, 3, 4);
    return o;
}
