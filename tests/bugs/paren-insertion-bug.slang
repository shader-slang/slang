// paren-insertion-bug.slang

// Confirm that precedence is correctly handled
// for cast from scalar to vector.

//TEST(compute):COMPARE_COMPUTE:

int test(float a)
{
	// This line performs a cast from a scalar result to a vector
	float3 b = pow(a, 2.0);

	// If the computation of `b` above gets folded into this
	// line of code (and we expect it to) we need to correctly
	// parenthesize the generated cast so that the `.xyz` swizzle
	// applies to the result of the cast, rather than the input.
	//
	return int(float4(b.xyz * 2.0, 1.0).x);
}


//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	uint inVal = tid;
	uint outVal = test(inVal);

	outputBuffer[tid] = outVal;
}




/*

[[vk::binding(1, 1)]]
Texture2D diffuseTexture : register(t1);

[[vk::binding(2, 1)]]
Texture2D normalTexture : register(t2);

[[vk::binding(3, 1)]]
SamplerState samplerState : register(s3);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
};

float4 main(PS_INPUT input) : SV_Target
{
    // Following constants should be moved to cbuffer
    const float3 eyePosition = { 1.5f, 1.5f, 1.5f }; // position of camera
    const float3 lightDirection = { 0.0f, -0.1f, -1.0f };
    const float specularPower = 10.0f;

    float3 lightVec = normalize(-lightDirection); // reverse direction
    float4 textureColor = diffuseTexture.Sample(samplerState, input.texCoord);
    float4 normalColor = normalTexture.Sample(samplerState, input.texCoord);

    // normal to world space
    float3 bumpNormal = 2 * (normalColor.xyz - 0.5); // texture [0-1] to [-1,1] range
    bumpNormal = (bumpNormal.x * input.tangent) + (bumpNormal.y * input.binormal) + (bumpNormal.z * input.normal);
    bumpNormal = normalize(bumpNormal);
    float diffuseLighting = saturate(dot(bumpNormal.xyz, lightVec));
    
    float3 toEye = normalize(eyePosition.xyz - input.worldPosition);
    // reflect = 2 * (N.L) * N - L
    float3 reflectVec = normalize((2.0f * diffuseLighting * bumpNormal) - lightVec);
    float specAngle = max(dot(reflectVec, toEye), 0.0f);
    float3 specular = pow(specAngle, specularPower);

    float4 color = float4(saturate(
        (textureColor.xyz * 0.4) +
        (textureColor.xyz * diffuseLighting) +
        (specular.xyz * 0.5)),
        textureColor.w);

    return color;
}

*/