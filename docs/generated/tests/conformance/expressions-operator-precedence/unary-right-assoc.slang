//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-2 prefix/unary operators are right-associative: `!!a` groups as `!(!a)` as shown in the doc example.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that prefix/unary operators at level 2 are right-associative.
// `!!a` groups as `!(!a)`: double-negation of true is true, of false is false.
// CHECK-DAG asserts r1=1 for `!!true` and r2=0 for `!!false`, confirming that
// each `!` applies to the result of the inner `!`, not to a partially-evaluated left side.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // Use inline bool variables to avoid slangi bool-parameter passing quirk
    bool t = true;
    bool f = false;
    int r1 = !!t ? 1 : 0;
    int r2 = !!f ? 1 : 0;
    // CHECK-DAG: unary-right-assoc: r1=1
    // CHECK-DAG: unary-right-assoc: r2=0
    printf("unary-right-assoc: r1=%d\n", r1);
    printf("unary-right-assoc: r2=%d\n", r2);
}
