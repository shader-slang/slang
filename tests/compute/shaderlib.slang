//TEST(compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:Texture2D(size=4, content = one) : dxbinding(0),glbinding(0)
//TEST_INPUT: Sampler : dxbinding(0),glbinding(0,1,2,3,4,5,6)
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
struct SurfacePosition
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 color;
};
struct LightSample
{
    float3 direction;
    float3 intensity;
};
interface ILight
{
    int getSampleCount(SurfacePosition pos);
    LightSample getSample(SurfacePosition pos, int n, int i);
}

struct View
{
    float3 cameraPos;
    float3 cameraDir;
    float2 viewportSize;
    float4x4 viewTransform, projectionTransform, worldTransform,
             viewProjTransform, invViewTransform, invWorldTransform;
    SamplerState samplerState;
};

interface IBRDFEvaluator
{
    associatedtype SurfaceProperty;
    SurfaceProperty evalSurfaceProperty(SurfacePosition pos, View view);
    float3 evalLighting(SurfaceProperty surf, LightSample lightSample);
    float getOpacity();
}

interface IMaterial
{
    associatedtype BRDFEvaluator : IBRDFEvaluator;
    BRDFEvaluator evalPattern(View view, SurfacePosition surf);
}

struct DirectionalLight : ILight
{
    float3 direction;
    float3 intensity;
    int getSampleCount(SurfacePosition pos)
    {
        return 1;
    }
    LightSample getSample(int n, int i)
    {
        LightSample ls;
        ls.direction = direction;
        ls.intensity = intensity;
        return ls;
    }
};

float3 EnvBRDFApprox( float3 SpecularColor, float Roughness, float NoV )
{
    float4 c0 = float4(-1, -0.0275, -0.572, 0.022);
    float4 c1 = float4(1, 0.0425, 1.04, -0.04);
    float4 r = Roughness * c0 + c1;
    float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    float2 AB = float2( -1.04, 1.04 ) * a004 + r.zw;
    AB.y *= min(50.0 * SpecularColor.g, 1.0);
    return SpecularColor * AB.x + AB.y;
}

float PhongApprox(float Roughness, float RoL)
{
    float a = Roughness * Roughness;	
    a = max(a, 0.008);					
    float a2 = a * a;						
    float rcp_a2 = 1.0/(a2);					
    float c = 0.72134752 * rcp_a2 + 0.39674113;	
    float p = rcp_a2 * exp2(c * RoL - c);	
    // Total 7 instr
    return min(p, rcp_a2);
}

struct DisneyBRDFSurfaceProperty
{
    float3 diffuseColor;
    float3 fspecularColor;
    float3 R;
};

struct DisneyBRDFEvaluator : IBRDFEvaluator
{
    float3 baseColor;
    float3 normal;
    float3 emissive;    
    float roughness, metallic, specular, opacity;
    
    typedef DisneyBRDFSurfaceProperty SurfaceProperty;

    DisneyBRDFSurfaceProperty evalSurfaceProperty(SurfacePosition pos, View view)
    {
        DisneyBRDFSurfaceProperty rs;
        float3 viewDir = normalize(pos.position - view.cameraPos);
        rs.diffuseColor = baseColor * (1.0-metallic);
        float dielectricSpecluar = 0.02 * specular;
        float3 specularColor = float3(dielectricSpecluar - dielectricSpecluar * metallic) 
            + baseColor * metallic;
        float NoV = max(dot(normal, viewDir), 0.0);
        rs.fspecularColor = EnvBRDFApprox(specularColor, roughness, NoV);
        rs.R = reflect(-viewDir, normal);
        return rs;
    }

    float3 evalLighting(DisneyBRDFSurfaceProperty surf, LightSample lightSample)
    {
        float RoL = max(0, dot(surf.R, lightSample.direction));
        float dotNL = clamp(dot(normal, lightSample.direction), 0.01, 0.99);
        float3 color = lightSample.intensity.xyz * dotNL * 
                (surf.diffuseColor + surf.fspecularColor * PhongApprox(roughness, RoL)) + emissive;
        return color;
    }

    float getOpacity()
    {
        return opacity;    
    }
};

struct DisneyMaterial0 : IMaterial
{
    typedef DisneyBRDFEvaluator BRDFEvaluator;
    Texture2D diffuseMap;
    DisneyBRDFEvaluator evalPattern(View view, SurfacePosition surf)
    {
        DisneyBRDFEvaluator rs;
        rs.baseColor = diffuseMap.Sample(view.samplerState, surf.uv).xyz;
        rs.normal = float3(0.0, 1.0, 0.0);
        rs.emissive = float3(0.0);
        rs.roughness = 1.0;
        rs.metallic = 0.0;
        rs.specular = 1.0;
        rs.opacity = 0.5;
        return rs;
    }
};

__generic<TMaterial : IMaterial, TLight : ILight>
float4 computeShading(View view, SurfacePosition surfPos, TMaterial mat, TLight light)
{
    TMaterial.BRDFEvaluator brdf = mat.evalPattern(view, surfPos);
    int lightSampleCount = light.getSampleCount(surfPos);
    float3 color = 0;
    TMaterial.BRDFEvaluator.SurfaceProperty surfProp = brdf.evalSurfaceProperty(surfPos, view);
    for (int i = 0; i < lightSampleCount; i++)
    {
        color += brdf.evalLighting(surfProp, light.getSample(surfPos, lightSampleCount, i));
    }
    return float4(color, brdf.getOpacity());
}

Texture2D diffuseMap;
SamplerState samplerState;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    View view;
    view.cameraPos = 0.0;
    view.cameraDir = float3(0.0, 0.0, -1.0);
    view.viewportSize = float2(1920.0, 1080.0);
    view.samplerState = samplerState;

    DisneyMaterial0 material;
    material.diffuseMap = diffuseMap;

    DirectionalLight dirLight;
    dirLight.direction = float3(0.0, 1.0, 0.0);
    dirLight.intensity = float3(1.0, 1.0, 1.0);

    SurfacePosition surfPos;
    surfPos.position = float3(0.0, -10.0, -10.0);
    surfPos.normal = float3(0.0, 1.0, 0.0);
    surfPos.uv = float2(0.5, 0.5);
    surfPos.color = 0.0;

	float4 outVal = computeShading<DisneyMaterial0, DirectionalLight>(view, surfPos, material, dirLight);
	
    outputBuffer[0] = outVal.x;
	outputBuffer[1] = outVal.y;
	outputBuffer[2] = outVal.z;
	outputBuffer[3] = outVal.w;
}