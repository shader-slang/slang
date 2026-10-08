//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=d3d4c02ac4dafd74393b443c556304f894229442554014fb013287b9cae88960
//META: purpose=Inside a raw string literal `R"d(...)d"`, backslashes are taken literally — no escape processing is performed by the lexer.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a raw string preserves a literal backslash-n sequence
// rather than converting it to a newline. We compile to CPP text and
// FileCheck for the C++ rendition of the four-byte sequence
// {a, '\\', 'n', 'b'} — in C++ source that prints as `"a\\nb"`. If
// the lexer were performing escape processing on the raw string, the
// emitted C++ would be `"a\nb"` (three bytes including a real
// newline) and the CHECK would not match.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", R"x(a\nb)x");
    //CHECK: "a\\nb"
}
