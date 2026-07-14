//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=c19c97466348045802dd2ac3f50daa2f7b25e6a69bb90b23bd259cb7cc63fb72
//META: purpose=A DeclRefType carries generic substitutions; two instantiations of the same generic struct with different type arguments behave as distinct types with substituted field types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the DeclRefType claim that a decl-ref carries "whatever
// generic substitutions apply": Box<int> and Box<float> resolve to
// the same StructDecl but with different substituted element types.
// Each stored value reads back at its substituted type; CHECK asserts
// both, proving the substitution is carried per-instantiation.

//TEST:INTERPRET(filecheck=CHECK):

struct Box<T> { T value; }

void main()
{
    Box<int> bi;
    bi.value = 7;
    Box<float> bf;
    bf.value = 2.5f;
    // CHECK: bi=7
    printf("bi=%d\n", bi.value);
    // CHECK: bf=2
    printf("bf=%d\n", int(bf.value));
}
