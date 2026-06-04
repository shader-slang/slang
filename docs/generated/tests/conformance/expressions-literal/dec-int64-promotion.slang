//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Decimal literal in `[2147483648, 9223372036854775807]` without an explicit suffix promotes to `int64_t` per the documented suffix-table row `(none)/L | dec | [2147483648, …] | int64_t`.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's suffix-table row for `(none)/L`, decimal base,
// value range `[2147483648, 9223372036854775807]`, gives `int64_t`.
// The unsuffixed literal `2147483648` should therefore land on the
// `int64_t` overload, not the `int` (overflow) or `uint` (would
// require explicit unsigned) overloads.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int x) { return 1; }
int probe(uint x) { return 2; }
int probe(int64_t x) { return 3; }
int probe(uint64_t x) { return 4; }

void main()
{
    int picked = probe(2147483648);
    //CHECK: picked=3
    printf("picked=%d\n", picked);
}
