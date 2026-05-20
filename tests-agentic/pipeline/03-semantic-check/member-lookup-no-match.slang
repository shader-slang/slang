//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
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
}
