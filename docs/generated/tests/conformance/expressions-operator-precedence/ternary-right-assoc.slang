//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-13 `?:` is right-associative: `true ? 1 : false ? 2 : 3` groups as `true ? 1 : (false ? 2 : 3)` = 1.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc example shows `!!a` = `!(!a)` for right-associative unary.
// For `?:`, the doc says right associativity (level 13).
// `true ? 1 : false ? 2 : 3`:
//   right-assoc: `true ? 1 : (false ? 2 : 3)` = 1 (outer cond true)
//   left-assoc:  `(true ? 1 : false) ? 2 : 3` = `1 ? 2 : 3` = 2 (different!)

//TEST:INTERPRET(filecheck=CHECK):

// Use int parameters (1/0 for true/false) to avoid slangi bool-parameter quirk.
int ternary_chain(int a, int b, int x, int y, int z)
{
    return a ? x : b ? y : z;
}

void main()
{
    // true ? 1 : (false ? 2 : 3) = 1  (outer true path)
    int r1 = ternary_chain(1, 0, 1, 2, 3);
    // false ? 1 : (true ? 2 : 3) = 2  (inner true path)
    int r2 = ternary_chain(0, 1, 1, 2, 3);
    // false ? 1 : (false ? 2 : 3) = 3  (both false — else of inner)
    int r3 = ternary_chain(0, 0, 1, 2, 3);
    // CHECK-DAG: ternary-right-assoc r1=1
    // CHECK-DAG: ternary-right-assoc r2=2
    // CHECK-DAG: ternary-right-assoc r3=3
    printf("ternary-right-assoc r1=%d\n", r1);
    printf("ternary-right-assoc r2=%d\n", r2);
    printf("ternary-right-assoc r3=%d\n", r3);
}
