//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=An integer literal suffix splits into one width part and one unsigned part, so repeating either part draws the invalid-integer-literal-suffix error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the suffix split is a split into two single slots rather than a
// free-form character bag. One file repeats the unsigned part (`uu`), the width
// part in its two-character spelling (`llll`), and the pointer-width part
// (`zz`); each draws the same rejection, which is what shows the rule is about
// the slot being filled twice and not about a particular letter. Exhaustive
// mode means no other diagnostic is emitted, so the literals really are being
// rejected at suffix-split time rather than later on their magnitude.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    let a = 1uu;
//CHECK:    ^^^ error E39999
//CHECK:    ^^^ invalid suffix 'uu' on integer literal

    let b = 1llll;
//CHECK:    ^^^^^ error E39999
//CHECK:    ^^^^^ invalid suffix 'llll' on integer literal

    let c = 1zz;
//CHECK:    ^^^ error E39999
//CHECK:    ^^^ invalid suffix 'zz' on integer literal

    printf("v=%d\n", int(a) + int(b) + int(c));
}
