//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=02a80201a5690a8a624d91ba229db896f43d7b71e5abd5c2e95980b3d576102d
//META: purpose=Boundary - a raw string with an empty delimiter (`R"(...)"`) lexes correctly, terminating only at `)"`.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the documented raw-string syntax `R"delimiter(...)delimiter"`
// permits an empty delimiter. The content `hello` here must reach
// emitted CPP intact.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", R"(hello)");
    //CHECK: "hello"
}
