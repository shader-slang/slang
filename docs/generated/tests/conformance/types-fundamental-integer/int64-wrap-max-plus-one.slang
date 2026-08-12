//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=Signed 64-bit overflow wraps: `INT64_MAX + 1 == INT64_MIN`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's wrap-on-overflow rule extends to int64_t. The
// MIN value here exercises the smallest-negative-int64 carve-out in
// the literal spec too (sibling bundle expressions-literal covers
// that interaction explicitly).

//TEST:INTERPRET(filecheck=CHECK):

int64_t addOne(int64_t x) { return x + 1; }

void main()
{
    int64_t v = addOne(9223372036854775807LL);
    //CHECK: wrap=-9223372036854775808
    printf("wrap=%lld\n", v);
}
