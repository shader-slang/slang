//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#angle-bracket-annotations
//META: doc_section_digest=7ce4d4ac217c6c67d9cc75b8b918d498cbe8d315518ddcbd63412016bbc998ce
//META: purpose=The declarator-level annotation skip is opt-in, so without `-enable-effect-annotations` the same `< ... >` clause is read as a generic argument list and its `;` is an unexpected token.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the declarator-level skip really is gated rather than always
// on. This is the identical declaration the opt-in test compiles cleanly, run
// without the flag: the `<` then has only one available reading — a generic
// argument list — and the parser stops at the first `;`, reporting it as an
// unexpected token where a `>` was expected. That the complaint is about the
// separator rather than about the annotation syntax as a whole is what shows
// the clause was never recognised as an annotation at all.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

Texture2D tex <string uiName = "Tex"; int uiOrder = 3;>;
//CHECK:                            ^ error E20001
//CHECK:                            ^ unexpected ';', expected '>'

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = 0.0;
}
