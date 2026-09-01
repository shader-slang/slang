//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=A `struct` declaration parses into a Decl node whose members are themselves Decls; field accesses through `.` resolve to the parsed member.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare a two-field struct and read both fields back. The values
// only match if both `int a` and `int b` member-decls were parsed and
// attached to the struct's ContainerDecl child list.

//TEST:INTERPRET(filecheck=CHECK):

struct V { int a; int b; };

void main()
{
    V v;
    v.a = 3;
    v.b = 4;
    // CHECK: sum=7
    printf("sum=%d\n", v.a + v.b);
}
