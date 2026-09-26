// Research fixture: no claim that this reproduces the material until it is compiled.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[0 1 1], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;

struct Payload
{
    float3 absorption;
    bool retroreflective;
};

struct Graph
{
    float3 weights[2];
    uint next;
    Payload payload;
};

// Ordinary helper calls are intentional: downstream O3 may inline them, as it does in the material.
void setWeight(inout float3 weights[2], uint index, float3 weight)
{
    weights[index] = weight;
}

Payload constructPayload(uint index, float absorption, bool retroreflective, out float weight)
{
    float3 weights[2] = {};
    Payload payload = {};
    payload.absorption = float3(absorption);
    payload.retroreflective = retroreflective;
    uint next = index & 1;
    setWeight(weights, next, float3(5.0));
    next = (next + 1) & 1;
    setWeight(weights, next, float3(7.0));
    weight = weights[index & 1].x;
    return payload;
}

uint evaluate(Payload payload, float weight)
{
    float3 transmission = float3(1.0);
    if (max(payload.absorption.x, max(payload.absorption.y, payload.absorption.z)) > 0.0)
        transmission = exp2(-payload.absorption);
    return uint(dot(transmission, float3(8.0))) +
        (payload.retroreflective ? 16u : 0u) + uint(weight);
}

// Compile this entry alone to inspect elimination of the constant-false exponential path.
[numthreads(1, 1, 1)]
void constantMain()
{
    for (uint i = 0; i < 2; ++i)
    {
        float weight;
        Payload payload = constructPayload(inputs[i], 0.0, false, weight);
        outputs[i] = evaluate(payload, weight);
    }
}

// Compile this entry alone as a positive control: input[2] is unknown to the compiler.
[numthreads(1, 1, 1)]
void controlMain()
{
    for (uint i = 0; i < 2; ++i)
    {
        float weight;
        Payload payload = constructPayload(inputs[i], float(inputs[2]), inputs[2] != 0, weight);
        outputs[i + 2] = evaluate(payload, weight);
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Both endpoints are exercised. The second write always targets the other array element.
    for (uint i = 0; i < 2; ++i)
    {
        float constantWeight;
        Payload constantPayload = constructPayload(inputs[i], 0.0, false, constantWeight);
        outputs[i] = evaluate(constantPayload, constantWeight);
        float controlWeight;
        Payload controlPayload = constructPayload(inputs[i], float(inputs[2]), inputs[2] != 0, controlWeight);
        outputs[i + 2] = evaluate(controlPayload, controlWeight);
    }
    // Independent arithmetic oracle: 3 * 8 + 5 = 29; 3 * (1/2) * 8 + 16 + 5 = 33.
    // CHECK: 29
    // CHECK-NEXT: 29
    // CHECK-NEXT: 33
    // CHECK-NEXT: 33
}
