//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=Unsigned 64-bit overflow wraps: `UINT64_MAX + 1 == 0`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The wrap-on-overflow rule applied to uint64_t at the documented
// MAX edge.

//TEST:INTERPRET(filecheck=CHECK):

uint64_t addOne(uint64_t x) { return x + 1; }

void main()
{
    uint64_t v = addOne(18446744073709551615ULL);
    //CHECK: wrap=0
    printf("wrap=%llu\n", v);
}
