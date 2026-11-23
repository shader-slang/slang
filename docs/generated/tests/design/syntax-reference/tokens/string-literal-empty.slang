//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=d772d9a407ac188fcd34c5908294cbc14fdde88567d1c3fba6970a29a34502af
//META: purpose=Boundary - the empty StringLiteral (`""`) is a valid token whose content is zero characters.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: zero-length string literal must lex cleanly and
// survive through to emitted C++ source as `""`.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", "");
    //CHECK: printf({{.*}}"%s{{.*}}", "")
}
