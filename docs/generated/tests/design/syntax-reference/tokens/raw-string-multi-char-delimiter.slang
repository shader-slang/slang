//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=e44df04d9dcfdbcd540c6b84f9f1c3b8b556eee2b84931196b66361c17faa2df
//META: purpose=Boundary - a multi-character delimiter (`long`) on a raw string opens with `R"long(` and closes only at `)long"`.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the documented raw-string rule uses an "arbitrary"
// delimiter. A four-character delimiter is exercised here. The string
// content includes a `)` and a `)w` substring that resemble a close
// for shorter delimiters but must NOT terminate this token.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", R"long(a)b)wc)long");
    //CHECK: "a)b)wc"
}
