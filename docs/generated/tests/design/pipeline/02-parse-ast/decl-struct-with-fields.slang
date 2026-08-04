//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
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
