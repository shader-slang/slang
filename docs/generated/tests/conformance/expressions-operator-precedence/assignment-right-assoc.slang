//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-14 `=` is right-associative: `a = b = 5` groups as `a = (b = 5)` — both a and b become 5.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `=` at precedence level 14 is right-associative.
// `a = b = 5` must group as `a = (b = 5)`, leaving both variables equal to 5.
// A left-associative reading `(a = b) = 5` would also produce a=5, b=0, which
// CHECK-DAG rules out by asserting both a=5 and b=5.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 0;
    int b = 0;
    a = b = 5;
    // CHECK: assignment-right-assoc: a=5 b=5
    printf("assignment-right-assoc: a=%d b=%d\n", a, b);
}
