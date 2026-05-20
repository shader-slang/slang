//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#string-dynamic-misc
//META: doc_section_digest=b5fab88de11ac4b9f993c16a3cfa3ad5866451f6b16870ea79e205028d2ea8ee
//META: purpose=StringType accepts long literal contents; a multi-hundred-character string survives unchanged into emitted CPP target code, including a marker token near its end.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of StringType: a long literal (160 characters of
// runs + an end marker). The literal must survive lowering intact;
// we anchor on the marker token to confirm the tail of the string
// is preserved.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    String s = "AAAAAAAAAABBBBBBBBBBCCCCCCCCCCDDDDDDDDDDEEEEEEEEEEFFFFFFFFFFGGGGGGGGGGHHHHHHHHHHIIIIIIIIIIJJJJJJJJJJKKKKKKKKKKLLLLLLLLLLMMMMMMMMMMNNNNNNNNNNOOOOOOOOOOPPPPPPPPPPmarker-end";
    printf(s);
//CHECK: marker-end
}
