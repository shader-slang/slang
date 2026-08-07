//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
//META: purpose=Stress: five levels of nested `namespace` blocks resolved by a five-segment `::`-qualified lookup probe that the namespace decl-keyword does not collapse depth.
//META: intent=stress
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Deepens the smoke-test coverage of `namespace` by stacking five
// nested namespace blocks and then walking back through all five
// segments at the call site.

//TEST:INTERPRET(filecheck=CHECK):

namespace a {
    namespace b {
        namespace c {
            namespace d {
                namespace e {
                    int get() { return 42; }
                }
            }
        }
    }
}

void main()
{
    int v = a::b::c::d::e::get();
    // CHECK: v=42
    printf("v=%d\n", v);
}
