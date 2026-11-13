//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

import "transitive-attribute-extensions-facade";

//TEST_INPUT:ubuffer(data=[2.0], stride=4):name=values
RWStructuredBuffer<float> values;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=gradients
RWStructuredBuffer<float> gradients;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float evaluate(SampleContainer container, float scale)
{
    // sample has no differentiable inputs, but its custom backward still has
    // an observable effect. Losing its association must not silently erase it.
    return container.samples.sample(0) * scale;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    SampleContainer container;
    container.samples.values = values;
    container.samples.gradients = gradients;
    var scale = diffPair(3.0, 0.0);
    bwd_diff(evaluate)(container, scale, 1.0);
    outputBuffer[0] = gradients[0];
    outputBuffer[1] = scale.d;
    // CHECK: 3.0
    // CHECK: 2.0
}
