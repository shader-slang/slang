//TEST:SIMPLE(filecheck=CHECK): -target spirv -O0

//CHECK: OpEntryPoint

struct Result
{
    float3 value;
};

struct Indirect
{
    float scale;
};

struct PushConstant
{
    Indirect *ptr;
};

struct Payload
{
    uint seed;
};

ConstantBuffer<PushConstant> pushConstants;

Result f3(Indirect ss, float2 randomSample)
{
    Result result;
    result.value = randomSample.x;
    return result;
}

float3 f2(inout uint seed)
{
    return f3(*pushConstants.ptr, float2(seed)).value;
}

float3 f1(inout Payload payload)
{
    return f2(payload.seed);
}

[shader("closesthit")]
void main(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    f1(payload);
}