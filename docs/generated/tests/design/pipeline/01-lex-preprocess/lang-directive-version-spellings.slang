//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-11T00:00:00+00:00
//META: source_commit=48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=b8118c99e1f7a9dcb30eebf7d13f30e2f807dfde61a88dc27f936547536b52c3
//META: purpose=The #lang version operand is resolved by name through one table in which each version carries several accepted spellings, so `202b` selects the same version as `2026`.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HandleLanguageDirective` hands the operand's *text* to
// `TypeTextUtil::findLanguageVersion` whether it arrived as an identifier or
// as an integer literal, so the letter-suffixed spellings are not a separate
// code path -- they are extra names on the same table row. This pins `202b`,
// the spelling that shares a row with `2026`; if the alias were dropped the
// directive would report E15207 rather than compiling.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#lang 202b

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    gOut[0] = 1;
}

// CHECK: void main
