//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly

// Regression test for feeding block-local witness tables synthesized by autodiff to
// `IRBuilder::getSet`, which requires global operands. Existential-table collection must keep only
// module-scope conformances.

[anyValueSize(16)]
interface ISensor<Real : IFloat> : IDifferentiable
{
    [Differentiable] Real splat(Real p);
}

struct Perspective<Real : IFloat> : ISensor<Real>, IDifferentiable
{
    Real focal;
    [Differentiable] Real splat(Real p) { return focal * p; }
}

// Keep the dispatch dynamic.
struct Ortho<Real : IFloat> : ISensor<Real>, IDifferentiable
{
    Real scale;
    [Differentiable] Real splat(Real p) { return scale + p; }
}

//TEST_INPUT: type_conformance Perspective<float>:ISensor<float> = 0
//TEST_INPUT: type_conformance Ortho<float>:ISensor<float> = 1

//TEST_INPUT: set sensors = ubuffer(data=[0 0 0 0 1077936128 0 0 0], stride=4)
StructuredBuffer<ISensor<float>> sensors;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float project(ISensor<float> sensor, float p)
{
    return sensor.splat(p);
}

[Differentiable]
float compute(int i, float p)
{
    ISensor<float> s = sensors[i];
    return project(s, p);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var dpp = diffPair(2.0f);
    bwd_diff(compute)(0, dpp, 1.0);
    // d/dp [ focal * p ] = focal = 3.0
    // CHECK: 3.0
    outputBuffer[tid.x] = dpp.d;
}
