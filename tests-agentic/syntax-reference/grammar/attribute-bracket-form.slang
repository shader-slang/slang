//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#attributes-and-decorations
//META: doc_section_digest=4060d5c47c1516c06a54f1ded73028e3fc2a792c74817f782fe37321c52bdcef
//META: purpose=`Attribute` parses as `'[' AttributeName ('(' ArgList? ')')? ']'` and attaches to the following decl.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bracket-form attribute production. `[numthreads(1,1,1)]`
// is an `AttributeName` followed by an `ArgList` inside parens, inside
// square brackets, attached to the entry-point function.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: numthreads
}
