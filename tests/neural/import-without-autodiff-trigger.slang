// A module deserialized from a `.slang-module` can refer to the derivative implementations that
// live in the lazily loaded autodiff supplement. `slang.neural` does: its activation functions
// reference the builtin `exp`/`sin`/`tanh`/`max` derivatives.
//
// Nothing in THIS file is a supplement load trigger -- no differentiability modifier, no
// `fwd_diff`/`bwd_diff`, no primal substitute, and no concrete `IDifferentiable` conformance.
// Every trigger fires while checking source, and a deserialized module never passes through
// them, so before the load-on-deserialize path this failed to link with eight
// `error[E45001]: unresolved external symbol`.
//
// The activations used below are the ones whose derivatives are registered in the supplement.
// Compiling is the whole assertion: linking is what resolves the imported symbols, so a
// regression reappears here as a compile failure rather than a wrong result.

//TEST:SIMPLE:-target spirv -stage compute -entry computeMain -experimental-feature -o import-without-autodiff-trigger.spv

import slang.neural;

typealias Vec3 = InlineVector<float, 3>;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float[3] arr = {-1.0, 0.0, 2.5};
    let input = Vec3(arr);

    let viaTanh = TanhActivation<float>().eval<Vec3>(input);
    let viaExp = ExpActivation<float>().eval<Vec3>(input);
    let viaSine = SineActivation<float>().eval<Vec3>(input);
    let viaRelu = ReLU<float>().eval<Vec3>(input);

    outBuf[tid.x] = viaTanh[0] + viaExp[1] + viaSine[2] + viaRelu[0];
}

// No `filecheck=`: compiling at all is the assertion. Linking is what resolves the imported
// symbols, so the regression this guards reappears as a nonzero exit, not as wrong output. `-o`
// is required for that: without it, slangc writes the compiled SPIR-V disassembly to stdout, and
// slang-test's default expectation (no `.expected.txt`, no `filecheck=`) is empty output, so the
// test would fail on success alone.
