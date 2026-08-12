//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#strings-and-dynamic-types
//META: doc_section_digest=2a08b8c0c1ef1106c941f59ca91edbcc41b4b973001b8197d8b4b7cee56bcdcb
//META: purpose=A string literal in a printf call produces an operand of IR type String in the dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A string literal in a printf call produces an operand of IR type
// String in the dump. The most portable surface that produces a
// `String`-typed IR value is a string literal passed to `printf`. The
// IR dump shows the literal inst with `String` as its IR-type operand.

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
