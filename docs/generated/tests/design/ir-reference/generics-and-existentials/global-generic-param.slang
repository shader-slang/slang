//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:18+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=1b35acbd21812d559bc561b706440cf4fe3c630c3cac3ebd9b614ae7f2aa48b6
//META: purpose=A module-scope type_param T declaration lowers to let %T : Type = global_generic_param.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that a generic parameter declared at module scope, rather than on a
// declaration, becomes its own module-scope IR value. The parameter is left
// unused in shader code: a used, unbound `type_param` is rejected with E38207
// (see global-generic-param-used-without-binding-diagnostic.slang), and the
// CHECK pins the `global_generic_param` line with its `Type` result in the
// lowered IR dump.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

type_param T;

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 1;
}

// CHECK: let  %{{[0-9]+}} : Type = global_generic_param
