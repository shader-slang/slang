//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-09-11T00:00:00+00:00
//META: source_commit=48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=cf9986ac73585feb8d7138003d7be69083848e024e0c79dad6db42132ddad3fe
//META: purpose=globallycoherent on a function parameter is the concrete pair that reaches E31201, while the same modifier on a struct field is accepted without -allow-glsl.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The non-GLSL arm for `GloballyCoherentModifier` requires `as<VarDecl>(decl)`,
// and a parameter is a `ParamDecl` -- a sibling of `VarDecl` under
// `VarDeclBase`, not a subclass -- so the predicate returns false and the
// modifier is rejected. The struct field below takes the *same* arm and is
// accepted, which is the half that shows `-allow-glsl` is not what admits it;
// exhaustive mode asserts the field draws no diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct G { globallycoherent int a; }

void takesCoherent(globallycoherent int x) { }
/*CHECK:
                   ^^^^^^^^^^^^^^^^ error E31201
                   ^^^^^^^^^^^^^^^^ modifier 'globallycoherent' is not allowed here.
*/

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    takesCoherent(1);
    gOut[0] = 1;
}
