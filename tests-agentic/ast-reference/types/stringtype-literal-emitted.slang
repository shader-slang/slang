//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#string-dynamic-misc
//META: doc_section_digest=b5fab88de11ac4b9f993c16a3cfa3ad5866451f6b16870ea79e205028d2ea8ee
//META: purpose=StringType is the Slang `String` type; a string-typed value's literal text survives lowering to emitted target code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `StringType` is the StringTypeBase leaf for the Slang `String`
// type. The user-facing claim: a String-typed literal preserves its
// text through to emitted target code. We FileCheck the CPP emit
// for the literal text because `slangi` printf does not support `%s`.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    String s = "stringtype-marker-token";
    printf(s);
//CHECK: stringtype-marker-token
}
