//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/types.md#strings-and-dynamic-types
//META: doc_section_digest=6a9d8c3140290ee10be2242b0279924f5334c2ed95e44fc5ad5aace3a95de920
//META: purpose=A string literal in a printf call produces an operand of IR type String in the dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Strings and dynamic types" table lists `String`
// (C++ wrapper `StringType`) as "Slang-language string". The most
// portable surface that produces a `String`-typed IR value is a
// string literal passed to `printf`. The IR dump shows the literal
// inst with `String` as its IR-type operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    printf("hi");
    buf[0] = 1;
}

// IR-LABEL: func %main
// IR: String
