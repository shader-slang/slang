//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
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
