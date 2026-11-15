//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=f5ca0be190b77cca517eee93b8fc03af868bd9b8d69abe8e20fadc934d5e4ab2
//META: purpose=Torch glue emission only binds functions marked [TorchEntryPoint], so `-target torch` on an ordinary compute entry point has nothing to bind and produces an empty pybind module rather than an error.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Torch caveat in the emit section: the glue emitter binds only
// `[TorchEntryPoint]` / `[AutoPyBindCUDA]` functions. An ordinary compute
// entry point with no parameters therefore yields a module with an empty
// body, and is not bound under its own name.

//TEST:SIMPLE(filecheck=TORCH):-target torch -line-directive-mode none

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gOut[0] = 1;
}

// TORCH: PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
// TORCH-NEXT: }
// TORCH-NOT: computeMain
