//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=533753bf58b03097d708a72394179634eea26760dccf152ebe2bcd53f0cfb0c0
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
