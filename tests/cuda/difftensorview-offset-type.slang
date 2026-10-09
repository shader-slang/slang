//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[2.0 3.0 4.0], stride=4):name=values
RWStructuredBuffer<float> values;
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):name=gradients
RWStructuredBuffer<float> gradients;

// Each wide view has five uint64 strides, five uint64 sizes, rank, and padding.
//TEST_INPUT:ubuffer(data=[values 4 0 4 0 4 0 4 0 0 0 3 0 1 0 1 0 1 0 0 0 4 0], stride=96):name=primal
uniform TensorView<float, uint64_t>* primal;
//TEST_INPUT:ubuffer(data=[gradients 4 0 4 0 4 0 4 0 0 0 3 0 1 0 1 0 1 0 0 0 4 0], stride=96):name=gradient
uniform TensorView<float, uint64_t>* gradient;

//TEST_INPUT:ubuffer(count=14, stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float readMixed(DiffTensorView<float, uint64_t> view, uint64_t i)
{
    return 2 * view.load(i) + view[vector<uint64_t, 2>(i, 0)];
}

[Differentiable]
void writeScalar(DiffTensorView<float, uint64_t> view, uint64_t i, float value)
{
    view.store(i, 3 * value);
}

[Differentiable]
float readOnce(DiffTensorView<float, uint64_t> view, uint64_t i)
{
    return 5 * view.loadOnce(vector<uint64_t, 3>(i, 0, 0));
}

[Differentiable]
void writeOnce(DiffTensorView<float, uint64_t> view, uint64_t i, float value)
{
    view.storeOnce(vector<uint64_t, 4>(i, 0, 0, 0), 7 * value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    AtomicAdd<uint64_t> wrapper;
    wrapper.diff = *gradient;
    let view = DiffTensorView<float, uint64_t>(*primal, wrapper);
    uint64_t i = 1;

    gradients[1] = 4;
    let readPair = fwd_diff(readMixed)(view, i);
    outputBuffer[0] = readPair.p;
    outputBuffer[1] = readPair.d;
    bwd_diff(readMixed)(view, i, 2);
    outputBuffer[2] = gradients[1];

    fwd_diff(writeScalar)(view, i, diffPair(5.0, 2.0));
    outputBuffer[3] = values[1];
    outputBuffer[4] = gradients[1];
    var valuePair = diffPair(5.0, 0.0);
    bwd_diff(writeScalar)(view, i, valuePair);
    outputBuffer[5] = valuePair.d;
    outputBuffer[6] = gradients[1];

    gradients[1] = 2;
    let oncePair = fwd_diff(readOnce)(view, i);
    outputBuffer[7] = oncePair.p;
    outputBuffer[8] = oncePair.d;
    bwd_diff(readOnce)(view, i, 3);
    outputBuffer[9] = gradients[1];

    fwd_diff(writeOnce)(view, i, diffPair(6.0, 2.0));
    outputBuffer[10] = values[1];
    outputBuffer[11] = gradients[1];
    valuePair = diffPair(6.0, 0.0);
    bwd_diff(writeOnce)(view, i, valuePair);
    outputBuffer[12] = valuePair.d;
    outputBuffer[13] = gradients[1];
}

// CHECK: 9.000000
// CHECK: 12.000000
// CHECK: 10.000000
// CHECK: 15.000000
// CHECK: 6.000000
// CHECK: 18.000000
// CHECK: 0.000000
// CHECK: 75.000000
// CHECK: 10.000000
// CHECK: 15.000000
// CHECK: 42.000000
// CHECK: 14.000000
// CHECK: 98.000000
// CHECK: 14.000000
