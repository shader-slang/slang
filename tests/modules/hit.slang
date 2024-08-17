import environment;

StructuredBuffer <Environment_sample_data> environment_sample_buffer;

float3 sample_lights(inout uint seed)
{
    return environment_sample(environment_sample_buffer, seed);
}

struct RadianceHitInfo
{
    float3 contribution;
};

struct Attributes
{
    float2 bary;
};

[shader("closesthit")]
void closesthit(inout RadianceHitInfo payload, Attributes attrib) {}
