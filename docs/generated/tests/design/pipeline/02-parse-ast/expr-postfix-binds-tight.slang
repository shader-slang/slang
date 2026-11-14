//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
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
