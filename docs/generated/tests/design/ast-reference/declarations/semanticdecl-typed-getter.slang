//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A `semantic` declaration with a typed `get : <type>` accessor introduces a semantic name usable on a struct field.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a user-declared semantic is a real declaration usable in the
// `: Name` position of a field. The field annotated with the semantic is
// written and read like any other field; the CHECK asserts the round-tripped
// value, which requires the semantic name to have resolved to the declaration.

//TEST:INTERPRET(filecheck=CHECK):

semantic MySem { get : int; }

struct S { int v : MySem; }

void main()
{
    S s;
    s.v = 8;
    //CHECK: 8
    printf("%d\n", s.v);
}
