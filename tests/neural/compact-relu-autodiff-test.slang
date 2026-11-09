//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:SIMPLE(filecheck=REPORT): -target spirv -entry computeMain -stage compute -experimental-feature -report-checkpoint-intermediates

import slang.neural;

typealias Vector = InlineVector<half, 64>;

[Differentiable]
Vector evaluate(Vector input)
{
    return CompactReLU().eval<Vector>(input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Vector input = Vector();
    [ForceUnroll]
    for (int i = 0; i < Vector.Size; ++i)
        input[i] = ((i & 1) != 0) ? 2.0h : -2.0h;
    input[2] = 0.0h;

    Vector output = evaluate(input);
    DifferentialPair<Vector> forwardOutput =
        fwd_diff(evaluate)(diffPair(input, Vector(3.0h)));
    var inputPair = diffPair(input, Vector());
    bwd_diff(evaluate)(inputPair, Vector(3.0h));

    outputBuffer[0] = float(output[0]);
    outputBuffer[1] = float(output[1]);
    outputBuffer[2] = float(output[2]);
    outputBuffer[3] = float(inputPair.d[0]);
    outputBuffer[4] = float(inputPair.d[1]);
    outputBuffer[5] = float(inputPair.d[2]);
    outputBuffer[6] = float(forwardOutput.d[0]);
    outputBuffer[7] = float(forwardOutput.d[1]);
    outputBuffer[8] = float(forwardOutput.d[2]);

    // CHECK: 0.0
    // CHECK: 2.0
    // CHECK: 0.0
    // CHECK: 0.0
    // CHECK: 3.0
    // CHECK: 0.0
    // CHECK: 0.0
    // CHECK: 3.0
    // CHECK: 0.0
}

// A 64-element FP16 vector occupies 128 bytes. The custom apply context must retain only its
// 64-bit positivity mask.
// REPORT: checkpointing context of 8 bytes associated with: 'evaluate'
