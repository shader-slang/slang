//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/overview.md#ir-passes
//META: doc_section_digest=8eb46aace39b08225cdeb2b72e28b3195136184200d25f39b56909696873a5e9
//META: purpose=The IR pass list itself branches on the target: the PyTorch binding generation runs only for the torch target, so one source with a [TorchEntryPoint] function gains a pybind module binding on `-target torch` and nothing of the kind on the C++ target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The overview says "target-sensitive" means whole passes are guarded by
// target predicates inside `linkAndOptimizeIR`, naming the PyTorch binding
// generation as an example (differences that are only spelling are left to
// the emitter, which is why thread-group spelling is NOT used here). The
// same module is compiled for torch and for C++. Only the torch run runs the
// binding pass and so only it contains a `PYBIND11_MODULE` that registers
// `addOne`; the C++ run emits the ordinary entry point and no binding.

//TEST:SIMPLE(filecheck=TORCH):-target torch -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none

[TorchEntryPoint]
int addOne(int x)
{
    return x + 1;
}

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gOut[0] = addOne(1);
}

// TORCH: PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
// TORCH: m.def("addOne", &addOne, "addOne");

// CPP: computeMain
// CPP-NOT: PYBIND11_MODULE
// CPP-NOT: m.def(
