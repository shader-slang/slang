//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=ExtensionDecl owns a scope; the extended type's member-lookup finds the extension method when called via the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ExtensionDecl is listed as scope-bearing. The extension's members
// are not in the enclosing file scope, but member lookup on the
// extended type reaches them.

//TEST:INTERPRET(filecheck=CHECK):

struct Point { int x; int y; }

extension Point {
    int manhattan() { return x + y; }
}

void main()
{
    Point p = {3, 5};
    int m = p.manhattan();
    // CHECK: m=8
    printf("m=%d\n", m);
}
