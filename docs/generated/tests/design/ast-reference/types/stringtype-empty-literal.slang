//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#string-dynamic-misc
//META: doc_section_digest=b5fab88de11ac4b9f993c16a3cfa3ad5866451f6b16870ea79e205028d2ea8ee
//META: purpose=StringType accepts the empty string literal ""; the empty literal survives to emitted CPP target code as an empty quoted form.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small-edge of StringType: the empty literal
// `""` is still a valid String value. The CPP backend emits it as
// an empty quoted string in the source.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    String s = "";
    printf(s);
//CHECK: ""
}
