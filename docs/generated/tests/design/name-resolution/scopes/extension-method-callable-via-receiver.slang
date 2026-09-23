//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=c25b4949da1f02764ce1c67f1094b0f0f36902472c4e7072dce4b3577317245e
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
