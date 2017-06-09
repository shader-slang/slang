//TEST_IGNORE_FILE:
//--------------------------------------------------------------------------------------
// File: DynamicShaderLinkageFX11.fx
//
// The effect file for the DynamicShaderLinkageFX11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "DynamicShaderLinkageFX11_VS.hlsl"
#include "DynamicShaderLinkageFX11_PS.hlsl"

//
// Settings for static permutations.
// All of the pre-5.0 targets need static specialization
// since they don't support late binding.  The below
// just selects a single specialization but you could
// create any number of them, each one representing
// a new shader with the interfaces compiled out
// due to the compile-time class references.
//

#define StaticMaterial         g_plasticTexturedMaterial
#define StaticAmbientLight     g_ambientLight
#define StaticDirectLight      g_directionalLight
#define StaticEnvironmentLight g_environmentLight

technique11 FeatureLevel10
{
    pass
    {
        SetRasterizerState(g_rasterizerState[g_fillMode]);
        SetVertexShader(CompileShader(vs_4_0,
                                      VSMain()));
        SetPixelShader(CompileShader(ps_4_0,
                                     PSMainUniform(StaticAmbientLight,
                                                   StaticDirectLight,
                                                   StaticEnvironmentLight,
                                                   StaticMaterial)));
    }
}

technique11 FeatureLevel10_1
{
    pass
    {
        SetRasterizerState(g_rasterizerState[g_fillMode]);
        SetVertexShader(CompileShader(vs_4_1,
                                      VSMain()));
        SetPixelShader(CompileShader(ps_4_1,
                                     PSMainUniform(StaticAmbientLight,
                                                   StaticDirectLight,
                                                   StaticEnvironmentLight,
                                                   StaticMaterial)));
    }
}

//
// Variables for dynamic shader linkage.
// There are two variations here for dynamic usage.
// In the first we use the uniform entry point
// and pass in global interface variables.  This
// creates a shader which refers to the global
// interface variables when running and we can bind
// concrete instances in our C++ code by using
// ID3DX11EffectInterfaceVariable::SetClassInstance.
// This approach works well when you have several
// independent variations and want to bind them
// individually in your C++ code, such as the
// different lighting and material parameters in
// this sample.
//

iBaseLight g_abstractAmbientLighting;
iBaseLight g_abstractDirectLighting;
iBaseLight g_abstractEnvironmentLighting;
iBaseMaterial g_abstractMaterial;
    
technique11 FeatureLevel11
{
    pass
    {
        SetRasterizerState(g_rasterizerState[g_fillMode]);
        SetVertexShader(CompileShader(vs_5_0,
                                      VSMain()));
        SetPixelShader(CompileShader(ps_5_0,
                                     PSMainUniform(g_abstractAmbientLighting,
                                                   g_abstractDirectLighting,
                                                   g_abstractEnvironmentLighting,
                                                   g_abstractMaterial)));
    }
}

//
// In this second variation we use the non-uniform
// entry point so that we don't have to specify
// any interfaces when compiling the shader.  We
// then reuse the compiled shader with different
// BindInterfaces calls so that all bindings are
// handled automatically by the effect runtime.
// Below we have multiple techniques where
// we've given a concrete binding for the material.
// Lighting parameters are left as interfaces for
// binding via effect variables, but could also
// be specified concretely if the number of variations
// is manageable.
// This approach works well for a small number of variations
// that are known in advance, as you can just list them
// in your effect and you don't need to do the
// binding work explicitly in your C++ code.
//

VertexShader g_NonUniVS = CompileShader(vs_5_0, VSMain());
PixelShader g_NonUniPS = CompileShader(ps_5_0, PSMainNonUniform());

technique11 FeatureLevel11_g_plasticMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_plasticMaterial));
    }
}

technique11 FeatureLevel11_g_plasticTexturedMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_plasticTexturedMaterial));
    }
}

technique11 FeatureLevel11_g_plasticLightingOnlyMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_plasticLightingOnlyMaterial));
    }
}

technique11 FeatureLevel11_g_roughMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_roughMaterial));
    }
}

technique11 FeatureLevel11_g_roughTexturedMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_roughTexturedMaterial));
    }
}

technique11 FeatureLevel11_g_roughLightingOnlyMaterial
{
    pass
    {
        SetVertexShader(g_NonUniVS);
        SetPixelShader(BindInterfaces(g_NonUniPS,
                                      g_abstractAmbientLighting,
                                      g_abstractDirectLighting,
                                      g_abstractEnvironmentLighting,
                                      g_roughLightingOnlyMaterial));
    }
}
