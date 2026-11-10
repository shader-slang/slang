//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#astbuilder
//META: doc_section_digest=9487463b588fd4f4aea24e0b0545152b863344c40f968f8aaab8d2c6b857d237
//META: purpose=The ASTBuilder hash-conses types, so two structurally identical spellings denote one type and cannot be used to declare two distinct overloads.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies type interning through the one place it becomes user-visible:
// overload identity. `float3` and `vector<float, 3>` are written differently
// and are built at different points in the file, so if each spelling produced
// its own type node the two `probe` declarations would be distinct overloads
// and both would be accepted. Instead the second is reported as a redeclaration
// of the first, which is only possible if the builder handed back the same
// interned type for both spellings.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int probe(float3 v) { return 1; }

int probe(vector<float, 3> v) { return 2; }
/*CHECK:
    ^^^^^ error E30201
    ^^^^^ redeclared here
*/

void main()
{
    printf("v=%d\n", probe(float3(1, 2, 3)));
}
