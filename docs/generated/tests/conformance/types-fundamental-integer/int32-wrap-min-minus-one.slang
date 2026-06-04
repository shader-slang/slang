//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=Signed 32-bit underflow wraps: `INT_MIN - 1 == INT_MAX`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's "wrap on overflow" rule applies to underflow too
// (overflow is the signed-arithmetic abstraction that includes both
// directions). Verify at MIN edge: `INT_MIN - 1` wraps to `INT_MAX`.

//TEST:INTERPRET(filecheck=CHECK):

int subOne(int x) { return x - 1; }

void main()
{
    int v = subOne(-2147483648);
    //CHECK: wrap=2147483647
    printf("wrap=%d\n", v);
}
