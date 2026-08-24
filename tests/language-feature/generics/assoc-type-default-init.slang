// Default-initializing an interface-constrained associated type that resolves to a
// non-empty struct produces that struct's field-wise default value.
// Regression for shader-slang/slang#12708.
//
// The crash was in the type-flow specialization pass, which only runs under dynamic
// dispatch, so the `[anyValueSize]`, `createDynamicObject`, and `type_conformance`
// scaffolding is load-bearing to the repro — it forces `MaterialInstance<T>.sample()`
// (which does `T.WeightType weights = {}`) to be specialized. Do not simplify it into a
// static-dispatch test; that no longer exercises the fixed path.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IWeights
{
    float get_value();
}

struct Weights : IWeights
{
    float value;
    float get_value() { return value; }
}

interface IMaterialData
{
    associatedtype WeightType : IWeights;
}

struct MaterialData : IMaterialData
{
    typealias WeightType = Weights;
}

struct MaterialInstance<T : IMaterialData>
{
    float sample()
    {
        T.WeightType weights = {};
        return weights.get_value();
    }
}

[anyValueSize(16)]
interface IMaterial
{
    float sample();
}

//TEST_INPUT: type_conformance Material:IMaterial = 0
struct Material : IMaterial
{
    float payload;
    float sample() { return MaterialInstance<MaterialData>().sample(); }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 thread_id: SV_DispatchThreadID)
{
    IMaterial material = createDynamicObject<IMaterial>(thread_id.x, 0.0);
    outputBuffer[0] = material.sample();
    // CHECK: 0.0
}
