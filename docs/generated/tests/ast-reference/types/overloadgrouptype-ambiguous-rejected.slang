//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#general-purpose-types
//META: doc_section_digest=d1ff0a0974c79b15cca66d02e10faaa455cbee26ce299de54d386fa9d4a40656
//META: purpose=OverloadGroupType is the pseudo-type of an unresolved overload set; when no target type lets the checker collapse the set, the reference is diagnosed as ambiguous.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `OverloadGroupType` as "the pseudo-type of an
// unresolved overload set; collapsed by checking". The visible
// surface: a bare name referring to multiple overloads, used in a
// context where no target type forces a pick, is rejected as
// "ambiguous reference".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int convert(int x) { return x * 2; }
float convert(float x) { return x + 1.0; }

void main()
{
    int x = convert;
//CHECK:    ^^^^^^^ ambiguous reference to 'convert'
//CHECK: E39999
}
