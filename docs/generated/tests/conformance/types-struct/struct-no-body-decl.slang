//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A no-body struct declaration (forward declaration) declares a struct type's existence and allows it to be used in type expressions before the full definition.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the doc's no-body ("forward") struct declaration followed by a full definition of the
// same name.
//
// The language reference says the no-body form "specifies an existence of a structure type ...
// This enables its use in type expressions without the member declarations", which reads as a
// forward declaration that a later with-members declaration completes. The compiler does not
// merge the two: it treats `struct ForwardDeclared;` and `struct ForwardDeclared { int x; }` as
// two conflicting declarations of the same name (E30200) and then reports every use of the name
// as ambiguous (E39999). That happens in the front end, so it is identical on every target --
// no target reaches emission for this input. The CHECKs below pin that observed rejection on
// each shader target rather than an invented struct emission.
//
// The spec-vs-compiler disagreement is recorded as
// `_meta/findings/types-struct-no-body-forward-decl-conflicting-declaration.yaml` and as a
// `drift-from-source` row in this bundle's README `## Doc gaps observed`.
//
// A no-body declaration on its own does work: `struct Fwd;` followed by `Fwd f;` (with no
// second declaration) compiles. It is specifically the "... before the full definition" half of
// the claim that the compiler rejects.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

// Forward declaration (no body)
struct ForwardDeclared;

// Full definition follows
struct ForwardDeclared
{
    int x;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    ForwardDeclared fd;
    fd.x = 55;
    output[0] = fd.x;
}

// The previous CHECKs here were `ForwardDeclared` and `x`, which matched the *error text*
// rather than any emitted HLSL, so the test passed while asserting nothing. These pin the
// diagnostic explicitly.
// CHECK-DAG: error
// CHECK-DAG: E30200
// CHECK-DAG: conflicting declaration
// CHECK-DAG: E39999
// CHECK-DAG: ambiguous reference to 'ForwardDeclared'

// GLSL-DAG: error
// GLSL-DAG: E30200
// GLSL-DAG: conflicting declaration
// GLSL-DAG: E39999
// GLSL-DAG: ambiguous reference to 'ForwardDeclared'

// SPV-DAG: error
// SPV-DAG: E30200
// SPV-DAG: conflicting declaration
// SPV-DAG: E39999
// SPV-DAG: ambiguous reference to 'ForwardDeclared'

// METAL-DAG: error
// METAL-DAG: E30200
// METAL-DAG: conflicting declaration
// METAL-DAG: E39999
// METAL-DAG: ambiguous reference to 'ForwardDeclared'

// WGSL-DAG: error
// WGSL-DAG: E30200
// WGSL-DAG: conflicting declaration
// WGSL-DAG: E39999
// WGSL-DAG: ambiguous reference to 'ForwardDeclared'
