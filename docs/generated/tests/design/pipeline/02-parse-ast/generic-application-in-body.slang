//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=In body position the parser attempts a generic-application parse for `<...>` after an identifier and commits if it parses cleanly, so `pickMax<int>(3, 7)` is treated as a generic call.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickMax<int>` is the positive case from `## Generics ambiguity`:
// the parser tries the generic-argument-list parse, finds it commits
// cleanly (the `>` is followed by a generic-follower `(`), and emits
// the corresponding Expr. A successful run prints the picked value.

//TEST:INTERPRET(filecheck=CHECK):

T pickMax<T : IComparable>(T a, T b)
{
    if (a > b) return a;
    return b;
}

void main()
{
    int m = pickMax<int>(3, 7);
    // CHECK: m=7
    printf("m=%d\n", m);
}
