//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

import "transitive-attribute-extensions-facade";
#ifdef DIRECT_IMPORT
import "transitive-attribute-extensions-lib";
#endif

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float evaluate(Container container, float x)
{
    return container.operations.square(x);
}

[ForwardDifferentiable]
float evaluateCustom(Container container, float x)
{
    return container.operations.custom(x);
}

[Differentiable]
float evaluateGeneric(Container container, float x)
{
    return container.operations.genericSquare<float>(x);
}

[Differentiable]
float evaluateOf(Container container, float x)
{
    return container.operations.decoratedElsewhere(x);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Container container = {};
    // Both automatic call differentiation and explicit derivative lookup must
    // discover the attributes on a member reached through the facade's field.
    outputBuffer[0] = fwd_diff(evaluate)(container, diffPair(3.0, 1.0)).d;
    var x = diffPair(3.0, 0.0);
    bwd_diff(evaluate)(container, x, 1.0);
    outputBuffer[1] = x.d;
    outputBuffer[2] = fwd_diff(ExposedOperations.square)(container.operations, diffPair(3.0, 1.0)).d;
    outputBuffer[3] = fwd_diff(evaluateGeneric)(container, diffPair(3.0, 1.0)).d;
    outputBuffer[4] = fwd_diff(evaluateCustom)(container, diffPair(3.0, 1.0)).d;
    bwd_diff(ExposedOperations.customReverse)(container.operations, x, 1.0);
    outputBuffer[5] = x.d;
    let second = fwd_diff(fwd_diff(evaluate))(
        container, diffPair(diffPair(3.0, 1.0), diffPair(1.0, 0.0)));
    outputBuffer[6] = second.d.d;
    outputBuffer[7] = fwd_diff(evaluateOf)(container, diffPair(3.0, 1.0)).d;
    x = diffPair(3.0, 0.0);
    bwd_diff(evaluateOf)(container, x, 1.0);
    outputBuffer[8] = x.d;
#ifdef DIRECT_IMPORT
    // A direct import still makes handwritten extensions available, without
    // duplicating the attribute-generated candidates from the same module.
    outputBuffer[9] = container.operations.ordinaryExtension(4.0);
#else
    outputBuffer[9] = 0.0;
#endif
    // CHECK: 6.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 9.0
    // CHECK-NEXT: 15.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 21.0
    // CHECK-NEXT: 33.0
}
