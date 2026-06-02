//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct conforming to an interface with a method emits to GLSL with the concrete method directly called; interface specialization produces GLSL with the concrete type's method name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the GLSL emission for an interface-typed parameter that is
// specialized for a concrete type at the call site.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

interface IProcessor
{
    int process(int x);
}

struct Tripler : IProcessor
{
    int process(int x) { return x * 3; }
}

int applyProc(IProcessor p, int x)
{
    return p.process(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<int> buf, uint3 id : SV_DispatchThreadID)
{
    Tripler t;
    buf[id.x] = applyProc(t, int(id.x));
}

// CHECK: Tripler_process
// CHECK: applyProc
