//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_section_digest=769b627a9edfb98d3dcd82a049d5720ec282bc1a720cf4d1da09d5420919dd11
//META: purpose=Suffix `U` forces an unsigned type per the suffix table; `1U` resolves the `uint` overload, not `int`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's suffix table maps `U`/`UL`/`LU` on a small decimal
// value to `uint`. Overload resolution distinguishes the literal's
// type at the call site: if `1U` is `uint`, the `uint` overload wins.

//TEST:INTERPRET(filecheck=CHECK):

int probe(int x) { return 1; }
int probe(uint x) { return 2; }
int probe(int64_t x) { return 3; }
int probe(uint64_t x) { return 4; }

void main()
{
    int picked = probe(1U);
    //CHECK: picked=2
    printf("picked=%d\n", picked);
}
