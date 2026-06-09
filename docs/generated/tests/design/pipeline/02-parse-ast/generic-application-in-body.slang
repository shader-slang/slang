//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=6cf4f0017f8ed0ca4ddf4b4f37de1831b62091dde8e6a8351a061ad3fe939266
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
