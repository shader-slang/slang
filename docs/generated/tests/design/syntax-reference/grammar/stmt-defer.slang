//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`DeferStmt` parses as `'defer' Stmt` and its statement runs after the rest of the enclosing scope.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `defer` production takes a plain Stmt (here a block)
// with no parentheses and no trailing clause. The log digit-appends
// on each step, so the final value records the execution order:
// `12` means the deferred block ran after the statement written below
// it, while `21` would mean it ran where it was written.

//TEST:INTERPRET(filecheck=CHECK):

void record(inout int log)
{
    defer { log = log * 10 + 2; }
    log = log * 10 + 1;
}

void main()
{
    int log = 0;
    record(log);
    // CHECK: log=12
    printf("log=%d\n", log);
}
