//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Member lookup that finds no match is rejected with diagnostic 30027 naming the missing member and the host type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The semantic checker performs member lookup on `e.name` and
// rejects when no member of that name exists in the type. Verify
// the rendered diagnostic and that it identifies both the missing
// name and the host type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Foo { int x; }

void main()
{
    Foo f;
    int y = f.nope;
//CHECK:      ^^^^ member not found
//CHECK:      ^^^^ 'nope' is not a member of 'Foo'.
// CHECK: E30027
}
