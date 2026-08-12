//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T14:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-operators.md#conditional-expression
//META: doc_section_digest=7fc06f74bccd80f5ad67ec31cc96986ffd178b15f5edb80257c79bb75b196d01
//META: purpose=The conditional operator `?:` does NOT short-circuit per the explicit doc note "Slang currently follows the precedent of HLSL where `?:` does not short-circuit." Both branches are evaluated.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's note is unambiguous: Slang's `?:` evaluates both
// branches even though only one value is yielded. Verify by placing
// a side-effecting bump on each branch and asserting the counter
// reflects BOTH bumps, not just the selected one.
//
// If the compiler accidentally implements C-family short-circuit
// `?:`, this test catches it.

//TEST:INTERPRET(filecheck=CHECK):

int bump(inout int counter) { counter++; return counter; }

void main()
{
    int counter = 0;
    bool cond = true;
    int picked = cond ? bump(counter) : bump(counter);
    //CHECK: counter=2
    printf("counter=%d\n", counter);
}
