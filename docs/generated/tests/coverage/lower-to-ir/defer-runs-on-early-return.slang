//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a `defer` block fires before an early `return` on every path that leaves the function scope.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins that the defer block registered at function scope runs on both the
// early-return path (n < 0) and the fall-through return. `visitDeferStmt`
// wires the defer block into the scope-end branch, so the cleanup print
// appears after the path-specific print on each call. Two calls exercise
// both exit edges. INTERPRET; no short-circuit operators (avoids the known
// slangi VM `&&` bug).
//TEST:INTERPRET(filecheck=CHECK):
void doWork(int n)
{
    defer { printf("cleanup\n"); }
    if (n < 0)
    {
        printf("negative\n");
        return;
    }
    printf("positive\n");
}
void main()
{
    doWork(-1);
    printf("---\n");
    doWork(5);
    //CHECK: negative
    //CHECK-NEXT: cleanup
    //CHECK-NEXT: ---
    //CHECK-NEXT: positive
    //CHECK-NEXT: cleanup
}
