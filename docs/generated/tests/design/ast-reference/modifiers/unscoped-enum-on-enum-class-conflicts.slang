//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-11T00:00:00+00:00
//META: source_commit=48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=ceb5d5225723810627591493eadff0fc89fc5232324a3c22bb1a0df55b36803b
//META: purpose=An explicit [UnscopedEnum] on an `enum class` conflicts with the implicit `class` modifier and is reported as E31202, not resolved by a precedence rule.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `getModifierConflictGroupKind` maps both `EnumClassModifier` and
// `UnscopedEnumAttribute` to the same conflict group, so the second one to
// land on a declaration is rejected rather than silently winning or losing.
// The plain `enum class` below carries no annotation, and exhaustive mode is
// what turns that silence into an assertion: if the conflict group ever
// started firing on a lone `enum class`, the unannotated diagnostic would
// fail the test rather than passing unnoticed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

enum class Scoped { A, B }

[UnscopedEnum] enum class Conflicting { A, B }
/*CHECK:
 ^^^^^^^^^^^^ error E31202
 ^^^^^^^^^^^^ modifier 'UnscopedEnum' is redundant or conflicting with existing modifier 'class'
*/

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    gOut[0] = (int)Scoped.A;
}
