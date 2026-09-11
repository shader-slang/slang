//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#adjacent-targets
//META: doc_section_digest=670fb73013f0eb3dbefe3964c40824a553973f0ec76bbe12d828725e0730b00f
//META: purpose=A hand-written __dispatch_kernel in an ordinary compute entry point survives removeTorchKernels and emits verbatim as fn<<<grid, block>>>, with the surface's dispatchSize argument landing in CUDA's block slot and threadGroupSize in the grid slot.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDA emitter's own kernel-launch spelling, and the
// operand inversion between the Slang surface and CUDA's syntax.
// The surface is __dispatch_kernel(fn, dispatchSize, threadGroupSize);
// CUDA's <<<...>>> takes grid-then-block. Writing dispatchSize as
// uint3(32,1,1) and threadGroupSize as uint3(1,1,1), the CHECK
// asserts the emitted launch reads <<<make_uint3(1U,1U,1U),
// make_uint3(32U,1U,1U)>>> — i.e. the two arguments swap slots. The
// callee is an ordinary function, not a [TorchEntryPoint], so
// removeTorchKernels leaves it alone and the launch reaches text.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

void myKernel()
{
    outputBuffer[0] = 7;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    __dispatch_kernel(myKernel, uint3(32, 1, 1), uint3(1, 1, 1));
}

// The callee survives as a plain __device__ function...
// CHECK: __device__ void myKernel_{{[0-9]+}}()

// ...and the launch is emitted with grid = threadGroupSize and
// block = dispatchSize, inverting the surface argument order.
// CHECK: myKernel_{{[0-9]+}}<<<make_uint3 (1U, 1U, 1U), make_uint3 (32U, 1U, 1U)>>>()
