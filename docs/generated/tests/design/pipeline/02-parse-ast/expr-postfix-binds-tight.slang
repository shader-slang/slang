//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=Postfix `.` member access binds tightly and left-associatively, so `o.f.g + 1` parses as `((o.f).g) + 1`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two chained `.` accesses must each resolve before the `+` adds the
// literal. If `.` and `+` had the wrong relative binding the parse
// would fail or the wrong subtree would receive the `+ 1`.

//TEST:INTERPRET(filecheck=CHECK):

struct Inner { int g; };
struct Outer { Inner f; };

void main()
{
    Outer o;
    o.f.g = 5;
    // CHECK: r=6
    printf("r=%d\n", o.f.g + 1);
}
