//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -slang -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

[Differentiable]
float readConditional<let enabled : bool>(Conditional<float, enabled> c)
{
    if (let value = c.get())
        return value * value;
    return 7.0;
}

[Differentiable]
Conditional<float, enabled> constructConditional<let enabled : bool>(float x)
{
    return Conditional<float, enabled>(x * x);
}

[Differentiable]
float constructAndRead<let enabled : bool>(float x)
{
    Conditional<float, enabled> c = x;
    c.set(x * x);
    if (let value = c.get())
        return value * x;
    return x;
}

struct PayloadGradient : IDifferentiable
{
    float dx;
}

struct Payload : IDifferentiable
{
    typealias Differential = PayloadGradient;

    [DerivativeMember(Differential.dx)]
    float x;
    no_diff float offset;
}

[Differentiable]
float readPayload(Conditional<Payload, true> c)
{
    if (let p = c.get())
        return p.x * p.x + p.offset * p.x;
    return 0.0;
}

[Differentiable]
Conditional<Payload, enabled> constructPayload<let enabled : bool>(float x)
{
    return Conditional<Payload, enabled>(Payload(x * x, 5.0));
}

[Differentiable]
float firstDerivative(float x)
{
    return fwd_diff(constructAndRead<true>)(diffPair(x, 1.0)).d;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let c = Conditional<float, true>(3.0);
    let dc = Conditional<float, true>(1.0);
    // CHECK: 6.0
    outputBuffer[0] = fwd_diff(readConditional<true>)(diffPair(c, dc)).d;
    var pair = diffPair(c, Conditional<float, true>.dzero());
    bwd_diff(readConditional<true>)(pair, 1.0);
    // CHECK: 6.0
    outputBuffer[1] = pair.d.get().value;

    // CHECK: 2.0
    outputBuffer[2] = Conditional<float, true>.dadd(dc, dc).get().value;

    let made = fwd_diff(constructConditional<true>)(diffPair(3.0, 1.0));
    // CHECK: 6.0
    outputBuffer[3] = made.d.get().value;
    var x = diffPair(3.0, 0.0);
    bwd_diff(constructConditional<true>)(x, dc);
    // CHECK: 6.0
    outputBuffer[4] = x.d;

    // CHECK: 27.0
    outputBuffer[5] = fwd_diff(constructAndRead<true>)(diffPair(3.0, 1.0)).d;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructAndRead<true>)(x, 1.0);
    // CHECK: 27.0
    outputBuffer[6] = x.d;

    let empty = Conditional<float, false>(3.0);
    // CHECK: 0.0
    outputBuffer[7] = fwd_diff(readConditional<false>)(diffPair(empty, empty)).d;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructConditional<false>)(x, empty);
    // CHECK: 0.0
    outputBuffer[8] = x.d;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructAndRead<false>)(x, 1.0);
    // CHECK: 1.0
    outputBuffer[9] = x.d;

    let payload = Conditional<Payload, true>(Payload(3.0, 5.0));
    let dPayload = Conditional<PayloadGradient, true>(PayloadGradient(1.0));
    // CHECK: 11.0
    outputBuffer[10] = fwd_diff(readPayload)(diffPair(payload, dPayload)).d;
    var payloadPair = diffPair(payload, Conditional<Payload, true>.dzero());
    bwd_diff(readPayload)(payloadPair, 1.0);
    // CHECK: 11.0
    outputBuffer[11] = payloadPair.d.get().value.dx;

    let madePayload = fwd_diff(constructPayload<true>)(diffPair(3.0, 1.0));
    // CHECK: 6.0
    outputBuffer[12] = madePayload.d.get().value.dx;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructPayload<true>)(x, dPayload);
    // CHECK: 6.0
    outputBuffer[13] = x.d;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructPayload<false>)(x, Conditional<PayloadGradient, false>(PayloadGradient(1.0)));
    // CHECK: 0.0
    outputBuffer[14] = x.d;

    // CHECK: 18.0
    outputBuffer[15] = fwd_diff(firstDerivative)(diffPair(3.0, 1.0)).d;
}
