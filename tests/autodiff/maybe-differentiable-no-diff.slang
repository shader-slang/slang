//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// Clarification is part of code generation, so it must not depend on the validation passes.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -disable-non-essential-validations
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)

interface IMaybe
{
    [MaybeDifferentiable]
    float compute(float x);
}

struct ForwardOnly : IMaybe
{
    [ForwardDifferentiable]
    [NoDiffThis]
    float compute(float x) { return x * x; }
}

[Differentiable]
float clarified<T : IMaybe>(no_diff T t, float x)
{
    return no_diff(t.compute(x)) + x;
}

[Differentiable]
float constant<T : IMaybe>(no_diff T t, float x)
{
    return t.compute(2.0) + x;
}

[Differentiable]
float clarifiedSlope<T : IMaybe>(no_diff T t, float x)
{
    return __fwd_diff(clarified<T>)(t, diffPair(x, 1.0)).d;
}

// no_diff clarifies a call; it preserves a derivative when the requested mode is available.
[Differentiable]
void scale(inout float x) { x *= 3.0; }

[Differentiable]
float clarifiedMutation(float x)
{
    var y = x;
    no_diff(scale(y));
    return y * x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ForwardOnly t;
    let fwd = __fwd_diff(clarified<ForwardOnly>)(t, diffPair(3.0, 1.0));
    // CHECK: {{^}}7.000000{{$}}
    outputBuffer[0] = fwd.d;
    var bwd = diffPair(3.0);
    __bwd_diff(clarified<ForwardOnly>)(t, bwd, 1.0);
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[1] = bwd.d;
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[2] = __fwd_diff(constant<ForwardOnly>)(t, diffPair(3.0, 1.0)).d;
    var c = diffPair(3.0);
    __bwd_diff(constant<ForwardOnly>)(t, c, 1.0);
    // CHECK-NEXT: {{^}}1.000000{{$}}
    outputBuffer[3] = c.d;
    // CHECK-NEXT: {{^}}12.000000{{$}}
    outputBuffer[4] = fwd.p;
    // CHECK-NEXT: {{^}}3.000000{{$}}
    outputBuffer[5] = bwd.p;
    // CHECK-NEXT: {{^}}18.000000{{$}}
    outputBuffer[6] = __fwd_diff(clarifiedMutation)(diffPair(3.0, 1.0)).d;
    var mutated = diffPair(3.0);
    __bwd_diff(clarifiedMutation)(mutated, 1.0);
    // CHECK-NEXT: {{^}}18.000000{{$}}
    outputBuffer[7] = mutated.d;
    // CHECK-NEXT: {{^}}2.000000{{$}}
    outputBuffer[8] = __fwd_diff(clarifiedSlope<ForwardOnly>)(t, diffPair(3.0, 1.0)).d;
    var second = diffPair(3.0);
    __bwd_diff(clarifiedSlope<ForwardOnly>)(t, second, 1.0);
    // CHECK-NEXT: {{^}}2.000000{{$}}
    outputBuffer[9] = second.d;
}
