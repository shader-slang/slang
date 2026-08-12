//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T13:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-fundamental.md#integer
//META: doc_section_digest=663f749e8cb7b2eb7369790ba71f10f28717032d7716ffc16b13b2f783be258c
//META: purpose=Signed 32-bit overflow wraps per the doc's "All arithmetic operations on signed and unsigned integers wrap on overflow": `INT_MAX + 1 == INT_MIN`.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The reference's #integer-types section states: "All arithmetic
// operations on signed and unsigned integers wrap on overflow."
// Verify at the documented MAX edge: `INT_MAX + 1` wraps to `INT_MIN`.
// The operand is passed through a function to defeat the constant
// folder (see _common.md § Defeat the optimizer).

//TEST:INTERPRET(filecheck=CHECK):

int addOne(int x) { return x + 1; }

void main()
{
    int v = addOne(2147483647);
    //CHECK: wrap=-2147483648
    printf("wrap=%d\n", v);
}
