//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=Unsigned 32-bit underflow wraps: `0u - 1u == UINT_MAX`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's wrap-on-overflow rule covers underflow in the unsigned
// direction: subtracting from zero wraps to UINT_MAX.

//TEST:INTERPRET(filecheck=CHECK):

uint subOne(uint x) { return x - 1; }

void main()
{
    uint v = subOne(0u);
    //CHECK: wrap=4294967295
    printf("wrap=%u\n", v);
}
