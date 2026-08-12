//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=A raw string literal performs no escape processing at all, so a backslash sequence inside it survives into the string's bytes verbatim.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the raw-string scanner hands its body through without
// running it past the escape decoder. `\n` inside the raw literal must
// stay two characters (backslash, `n`) rather than collapsing to a
// newline. The emitted C++ re-escapes a literal backslash as `\\`, so
// seeing `a\\nb` in the generated source proves the backslash was
// preserved; a decoded escape would emit `a\nb` instead. slangi printf
// does not support `%s`, hence the emission observation.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", R"(a\nb)");
    //CHECK: "a\\nb"
}
