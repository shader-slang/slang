//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=`int` and `int32_t` are spec-level aliases — values are interchangeably assignable and equal.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's #integer-types table lists `int` and `int32_t`
// together in the same row, marking them as spec-level aliases for
// the 32-bit signed integer. Verify a value declared as one is
// directly assignable to and equal to the same value typed as the
// other, without an explicit conversion.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 12345;
    int32_t b = a;
    int c = b;
    bool eq = (a == b) && (b == c) && (a == c);
    //CHECK: a=12345 b=12345 c=12345 eq=1
    printf("a=%d b=%d c=%d eq=%d\n", a, b, c, eq ? 1 : 0);
}
