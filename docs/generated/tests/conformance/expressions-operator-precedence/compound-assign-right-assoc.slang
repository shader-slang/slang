//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-14 compound assignments `+=`, `-=`, etc. are right-associative like `=`: the RHS of `a += b = 3` is fully evaluated (b=3) before `a` is updated.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that compound-assignment operators (`+=` etc.) at precedence level 14
// are right-associative, just like plain `=`. In `a += b = 3`, right-to-left
// grouping gives `a += (b = 3)`: b is set to 3 first, then a increases by 3.
// CHECK asserts a=13 and b=3, which only holds under right-assoc grouping.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 10;
    int b = 0;
    a += b = 3;
    // CHECK: compound-assign-right-assoc: a=13 b=3
    printf("compound-assign-right-assoc: a=%d b=%d\n", a, b);
}
