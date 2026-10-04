//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the hard-coded `__GPU_FOREACH(device, gridDims, LAMBDA(uint3 id) { call; });` statement form parses and lowers to a call to the kernel's `_wrapper` entry.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: characterization-unverified=true
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `ParseGpuForeachStmt` is a hand-written parse of one exact
// token sequence -- the `LAMBDA` keyword, the parameter list, the braces and
// even the trailing `;` inside the braces are all read literally, with no
// general lambda grammar involved. The declared id parameter
// (`dispatchThreadID` here) is pushed into a fresh scope so the kernel call
// can name it.
//
// Emission turns the whole statement into a call to `<kernel>_wrapper` with
// the device and grid-dimension arguments spliced in front of the kernel's own
// arguments.
//
// characterization-unverified: the emitted `kernelCall_0_wrapper` symbol is
// never defined by the compiler in the generated code, so the output does not
// stand on its own -- it names a wrapper that a host-side driver is expected
// to supply. This test pins that as observed behaviour without asserting that
// it is the intended contract.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

void kernelCall(int a)
{
}

//CHECK: void main()
//CHECK: kernelCall_0_wrapper(int(0), uint3(1U, 1U, 1U), int(1));

[numthreads(1, 1, 1)]
void main()
{
    int renderer = 0;
    uint3 gridDims = uint3(1, 1, 1);
    __GPU_FOREACH(renderer, gridDims, LAMBDA(uint3 dispatchThreadID) {
        kernelCall(1); });
}
