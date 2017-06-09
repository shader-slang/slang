//TEST_IGNORE_FILE:
// FixedFuncEMU.fx
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
//

struct VSSceneIn
{
    float3 pos          : POSITION;         //position of the particle
    float3 norm         : NORMAL;           //velocity of the particle
    float2 tex          : TEXTURE0;         //tex coords
};

struct VSSceneOut
{
    float4 pos : SV_Position;               //position
    float2 tex : TEXTURE0;                  //texture coordinate
    float3 wPos : TEXTURE1;                 //world space pos
    float3 wNorm : TEXTURE2;                //world space normal
    float4 colorD : COLOR0;                 //color for gouraud and flat shading
    float4 colorS : COLOR1;                 //color for specular
    float  fogDist : FOGDISTANCE;           //distance used for fog calculations
    float3 planeDist : SV_ClipDistance0;    //clip distance for 3 planes
};

struct PSSceneIn
{
    float4 pos : SV_Position;               //position
    float2 tex : TEXTURE0;                  //texture coordinate
    float3 wPos : TEXTURE1;                 //world space pos
    float3 wNorm : TEXTURE2;                //world space normal
    float4 colorD : COLOR0;                 //color for gouraud and flat shading
    float4 colorS : COLOR1;                 //color for specular
    float  fogDist : FOGDISTANCE;           //distance used for fog calculations
};

struct Light
{
    float4 Position;
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
    float4 Atten;
};

#define FOGMODE_NONE    0
#define FOGMODE_LINEAR  1
#define FOGMODE_EXP     2
#define FOGMODE_EXP2    3
#define E 2.71828

cbuffer cbLights
{
    float4   g_clipplanes[3];
    Light    g_lights[8];
};

cbuffer cbPerFrame
{
    float4x4 g_mWorld;
    float4x4 g_mView;
    float4x4 g_mProj;
    float4x4 g_mInvProj;
    float4x4 g_mLightViewProj;
};

cbuffer cbPerTechnique
{
    bool     g_bEnableLighting = true;
    bool     g_bEnableClipping = true;
    bool     g_bPointScaleEnable = false;
    float    g_pointScaleA;
    float    g_pointScaleB;
    float    g_pointScaleC;
    float    g_pointSize;
    
    //fog params
    int      g_fogMode = FOGMODE_NONE;
    float    g_fogStart;
    float    g_fogEnd;
    float    g_fogDensity;
    float4   g_fogColor;
};
    
cbuffer cbPerViewChange
{
    //viewport params
    float    g_viewportHeight;
    float    g_viewportWidth;
    float    g_nearPlane;
};

cbuffer cbImmutable
{
    float3 g_positions[4] =
    {
        float3( -0.5, 0.5, 0 ),
        float3( 0.5, 0.5, 0 ),
        float3( -0.5, -0.5, 0 ),
        float3( 0.5, -0.5, 0 ),
    };
};

Texture2D g_txDiffuse;
Texture2D g_txProjected;
SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

struct ColorsOutput
{
    float4 Diffuse;
    float4 Specular;
};

ColorsOutput CalcLighting( float3 worldNormal, float3 worldPos, float3 cameraPos )
{
    ColorsOutput output = (ColorsOutput)0.0;
    
    for(int i=0; i<8; i++)
    {
        float3 toLight = g_lights[i].Position.xyz - worldPos;
        float lightDist = length( toLight );
        float fAtten = 1.0/dot( g_lights[i].Atten, float4(1,lightDist,lightDist*lightDist,0) );
        float3 lightDir = normalize( toLight );
        float3 halfAngle = normalize( normalize(-cameraPos) + lightDir );
        
        output.Diffuse += max(0,dot( lightDir, worldNormal ) * g_lights[i].Diffuse * fAtten) + g_lights[i].Ambient;
        output.Specular += max(0,pow( dot( halfAngle, worldNormal ), 64 ) * g_lights[i].Specular * fAtten );
    }
    
    return output;
}

//
// VS for emulating fixed function pipeline
//
VSSceneOut VSScenemain(VSSceneIn input)
{
    VSSceneOut output = (VSSceneOut)0.0;

    //output our final position in clipspace
    float4 worldPos = mul( float4( input.pos, 1 ), g_mWorld );
    float4 cameraPos = mul( worldPos, g_mView ); //Save cameraPos for fog calculations
    output.pos = mul( cameraPos, g_mProj );
    
    //save world pos for later
    output.wPos = worldPos;
    
    //save the fog distance for later
    output.fogDist = cameraPos.z;
    
    //find our clipping planes (fixed function clipping is done in world space)
    if( g_bEnableClipping )
    {
        worldPos.w = 1;
        
        //calc the distance from the 3 clipping planes
        output.planeDist.x = dot( worldPos, g_clipplanes[0] );
        output.planeDist.y = dot( worldPos, g_clipplanes[1] );
        output.planeDist.z = dot( worldPos, g_clipplanes[2] );
    }
    else
    {
        output.planeDist.x = 1;
        output.planeDist.y = 1;
        output.planeDist.z = 1;
    }
    
    //do gouraud lighting
    if( g_bEnableLighting )
    {
        float3 worldNormal = normalize( mul( input.norm, (float3x3)g_mWorld ) );
        output.wNorm = worldNormal;
        ColorsOutput cOut = CalcLighting( worldNormal, worldPos, cameraPos );
        output.colorD = cOut.Diffuse;
        output.colorS = cOut.Specular;
    }
    else
    {
        output.colorD = float4(1,1,1,1);
    }
    
    //propogate texture coordinate
    output.tex = input.tex;
    
    return output;
}

//
// VS for rendering in screen space
//
PSSceneIn VSScreenSpacemain(VSSceneIn input)
{
    PSSceneIn output = (PSSceneIn)0.0;

    //output our final position
    output.pos.x = (input.pos.x / (g_viewportWidth/2.0)) -1;
    output.pos.y = -(input.pos.y / (g_viewportHeight/2.0)) +1;
    output.pos.z = input.pos.z;
    output.pos.w = 1;
    
    //propogate texture coordinate
    output.tex = input.tex;
    output.colorD = float4(1,1,1,1);
    
    return output;
}

//
// GS for flat shaded rendering
//

[maxvertexcount(3)]
void GSFlatmain( triangle VSSceneOut input[3], inout TriangleStream<VSSceneOut> FlatTriStream )
{
    VSSceneOut output;
    
    //
    // Calculate the face normal
    //
    float3 faceEdgeA = input[1].wPos - input[0].wPos;
    float3 faceEdgeB = input[2].wPos - input[0].wPos;

    //
    // Cross product
    //
    float3 faceNormal = cross(faceEdgeA, faceEdgeB);
    
    //
    //calculate the face center
    //
    float3 faceCenter = (input[0].wPos + input[1].wPos + input[2].wPos)/3.0;
    
    //find world pos and camera pos
    float4 worldPos = float4( faceCenter, 1 );
    float4 cameraPos = mul( worldPos, g_mView );
    
    //do shading
    float3 worldNormal = normalize( faceNormal );
    ColorsOutput cOut = CalcLighting( worldNormal, worldPos, cameraPos );
    
    for(int i=0; i<3; i++)
    {
        output = input[i];
        output.colorD = cOut.Diffuse;
        output.colorS = cOut.Specular;
        
        FlatTriStream.Append( output );
    }
    FlatTriStream.RestartStrip();
}

//
// GS for point rendering
//
[maxvertexcount(12)]
void GSPointmain( triangle VSSceneOut input[3], inout TriangleStream<VSSceneOut> PointTriStream )
{
    VSSceneOut output;
    
    //
    // Calculate the point size
    //
    //float fSizeX = (g_pointSize/g_viewportWidth)/4.0;
    float fSizeY = (g_pointSize/g_viewportHeight)/4.0;
    float fSizeX = fSizeY;
    
    for(int i=0; i<3; i++)
    {
        output = input[i];
    
        //find world pos and camera pos
        float4 worldPos = float4(input[i].wPos,1);
        float4 cameraPos = mul( worldPos, g_mView );
        
        //find our size
        if( g_bPointScaleEnable )
        {   
            float dEye = length( cameraPos.xyz );
            fSizeX = fSizeY = g_viewportHeight * g_pointSize * 
                    sqrt( 1.0f/( g_pointScaleA + g_pointScaleB*dEye + g_pointScaleC*(dEye*dEye) ) );
        }
        
        //do shading
        if(g_bEnableLighting)
        {
            float3 worldNormal = input[i].wNorm;
            ColorsOutput cOut = CalcLighting( worldNormal, worldPos, cameraPos );
        
            output.colorD = cOut.Diffuse;
            output.colorS = cOut.Specular;
        }
        else
        {
            output.colorD = float4(1,1,1,1);
        }
        
        output.tex = input[i].tex;
        
        //
        // Emit two new triangles
        //
        for(int i=0; i<4; i++)
        {
            float4 outPos = mul( worldPos, g_mView );
            output.pos = mul( outPos, g_mProj );
            float zoverNear = (outPos.z)/g_nearPlane;
            float4 posSize = float4( g_positions[i].x*fSizeX*zoverNear,
                                     g_positions[i].y*fSizeY*zoverNear,
                                     0,
                                     0 );
            output.pos += posSize;
            
            PointTriStream.Append(output);
        }
        PointTriStream.RestartStrip();
    }
}

//
// Calculates fog factor based upon distance
//
float CalcFogFactor( float d )
{
    float fogCoeff = 1.0;
    
    if( FOGMODE_LINEAR == g_fogMode )
    {
        fogCoeff = (g_fogEnd - d)/(g_fogEnd - g_fogStart);
    }
    else if( FOGMODE_EXP == g_fogMode )
    {
        fogCoeff = 1.0 / pow( E, d*g_fogDensity );
    }
    else if( FOGMODE_EXP2 == g_fogMode )
    {
        fogCoeff = 1.0 / pow( E, d*d*g_fogDensity*g_fogDensity );
    }
    
    return clamp( fogCoeff, 0, 1 );
}

//
// PS for rendering with clip planes
//
float4 PSScenemain(PSSceneIn input) : SV_Target
{   
    //calculate the fog factor  
    float fog = CalcFogFactor( input.fogDist );
    
    //calculate the color based off of the normal, textures, etc
    float4 normalColor = g_txDiffuse.Sample( g_samLinear, input.tex ) * input.colorD + input.colorS;
    
    //calculate the color from the projected texture
    float4 cookieCoord = mul( float4(input.wPos,1), g_mLightViewProj );
    //since we don't have texldp, we must perform the w divide ourselves befor the texture lookup
    cookieCoord.xy = 0.5 * cookieCoord.xy / cookieCoord.w + float2( 0.5, 0.5 ); 
    float4 cookieColor = float4(0,0,0,0);
    if( cookieCoord.z > 0 )
        cookieColor = g_txProjected.Sample( g_samLinear, cookieCoord.xy );
    
    //for standard light-modulating effects just multiply normalcolor and coookiecolor
    normalColor += cookieColor;
    
    return fog * normalColor + (1.0 - fog)*g_fogColor;
}

//
// PS for rendering with alpha test
//
float4 PSAlphaTestmain(PSSceneIn input) : SV_Target
{   
    float4 color =  g_txDiffuse.Sample( g_samLinear, input.tex ) * input.colorD;
    if( color.a < 0.5 )
        discard;
    return color;
}

//
// RenderSceneGouraud - renders gouraud-shaded primitives
//
technique10 RenderSceneGouraud
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
    }  
}

//
// RenderSceneFlat - renders flat-shaded primitives
//
technique10 RenderSceneFlat
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSFlatmain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
    }  
}

//
// RenderScenePoint - replaces d3dfill_point
//
technique10 RenderScenePoint
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSPointmain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
    }  
}

//
// RenderScreneSpace - shows how to render something in screenspace
//
technique10 RenderScreenSpaceAlphaTest
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScreenSpacemain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSAlphaTestmain() ) );
        
        SetDepthStencilState( DisableDepth, 0 );
    }  
}

//
// RenderScreneSpace - shows how to render something in screenspace
//
technique10 RenderTextureOnly
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetDepthStencilState( EnableDepth, 0 );
    }  
}

