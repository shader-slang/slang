//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=e736541fe370e6b141fb2ebd19d2f4b376079f81f8b0c4c945979076e75beca5
//META: purpose=Standard modules (e.g. `neural`) are loaded on demand via `import slang.<name>`; without that import, neural-namespaced names are unresolved.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states standard modules are loaded on demand via
// `import slang.<name>`. Without an `import slang.neural;`, a
// reference to a `neural.*`-qualified name is an unknown identifier.
// We do NOT depend on the neural module existing at the agentic
// runner — we only assert that a `neural`-qualified name without
// import is rejected. The exact diagnostic is undeclared-identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // `neural` is a standard-module namespace not implicitly in scope.
    float v = neural.SomeName(tid.x);
//CHECK:      ^^^^^^ undefined identifier 'neural'.
    outBuf[tid.x] = v;
}
