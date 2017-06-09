//TEST_IGNORE_FILE:
//--------------------------------------------------------------------------------------
// File: Instancing.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------
struct VSInstIn
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
    row_major float4x4 mTransform : mTransform;
};

struct VSSceneIn
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
};

struct VSGrassIn
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
    row_major float4x4 mTransform : mTransform;
    uint VertexID : SV_VertexID;
};

struct VSGrassOut
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
    uint VertexID : VERTID;
};

struct VSQuadIn
{
    float3 pos : POSITION;
    float2 tex : TEXTURE0;
    row_major float4x4 mTransform : mTransform;
    float fOcc : fOcc;
    uint InstanceId : SV_InstanceID;
};

struct PSSceneIn
{
    float4 pos : SV_Position;
    float2 tex : TEXTURE0;
    float4 color : COLOR0;
};

struct PSQuadIn
{
    float4 pos : SV_Position;
    float3 tex : TEXTURE0;
    float4 color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer crarely
{
    float4x4 g_mTreeMatrices[50];
    uint g_iNumTrees;
};

cbuffer ceveryframe
{
    float4x4 g_mWorldViewProj;
    float4x4 g_mWorldView;
};

cbuffer cmultipleperframe
{
    float g_GrassWidth;
    float g_GrassHeight;
    uint g_iGrassCoverage;
};

cbuffer cusercontrolled
{
    float g_GrassMessiness;
};

struct light_struct
{
    float4 direction;
    float4 color;
};

cbuffer cimmutable
{
    light_struct g_lights[4] = { 
                    { float4(0.620275,  0.683659, 0.384537, 1),  float4(0.75, 0.599, 0.405, 1) },		//sun
                    { float4(0.063288, -0.987444, 0.144735, 1),  float4(0.192, 0.273, 0.275, 1) },		//bottom
                    { float4(0.23007,   0.785579, -0.574422, 1),  float4(0.300, 0.292, 0.223, 1) },		//highlight
                    { float4(-0.620275,  -0.683659, -0.384537, 1),  float4(0.0, 0.0, 0.1, 1) }			//blue rim-light
                    };
    
    float4 g_ambient = float4(0.4945,0.465,0.5,1);
    
    float g_occDimHeight = 2400.0;	//scalar that tells us how much to darken the tree near the top
};

cbuffer cgrassblade
{
    float3 g_positions[6] =
    {
        float3( -1, 0, 0 ),
        float3( -1, 2, 0 ),
        float3( 1, 0, 0 ),
        float3( 1, 2, 0 ),
        
        float3( -1, 0, 0 ),
        float3( -1, 2, 0 ),
    };
    float2 g_texcoords[6] = 
    { 
        float2(0,1), 
        float2(0,0),
        float2(1,1),
        float2(1,0),
        
        float2(0,1),
        float2(0,0),
    };
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D g_txDiffuse;
Texture2DArray g_tx2dArray;
SamplerState g_samLinear
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture1D g_txRandom;
SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// State structures
//--------------------------------------------------------------------------------------
BlendState QuadAlphaBlendState
{
    AlphaToCoverageEnable = TRUE;
    RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState EnableMSAA
{
    CullMode = BACK;
    MultisampleEnable = TRUE;
};

DepthStencilState DisableDepthTestWrite
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState EnableDepthTestWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// Sky vertex shader
//--------------------------------------------------------------------------------------
PSSceneIn VSSkymain(VSSceneIn input)
{
    PSSceneIn output;
    
    //
    // Transform the vert to view-space
    //
    float4 v4Position = mul(float4(input.pos, 1), g_mWorldViewProj);
    output.pos = v4Position;
    
    //  
    // Transfer the rest
    //
    output.tex = input.tex;
    
    output.color = float4(1,1,1,1);
    
    return output;
}

//--------------------------------------------------------------------------------------
// CalcLighting helper function.  Calculates lighting from 4 light sources, adds ambient
// and attenuates for depth.  Used by all techniques for lighting.
//--------------------------------------------------------------------------------------
float4 CalcLighting( float3 norm, float depth )
{
    float4 color = float4(0,0,0,0);
    
    // add the contributions of 4 directional lights
    [unroll] for( int i=0; i<4; i++ )
    {
        color += saturate( dot(g_lights[i].direction,norm) )*g_lights[i].color;
    }
    
    // give some attenuation due to depth
    float attenuate = depth / 10000.0;
    float4 attenColor = float4(0.15, 0.2, 0.3, 0);
    
    // add it all up plus ambient
    return (1-attenuate*0.23)*(color + g_ambient) + attenColor*attenuate;
}

//--------------------------------------------------------------------------------------
// Instancing vertex shader.  Positions the vertices based upon the matrix stored
// in the second vertex stream.
//--------------------------------------------------------------------------------------
PSSceneIn VSInstmain(VSInstIn input)
{
    PSSceneIn output;
    
    //
    // Transform by our Sceneance matrix
    //
    float4 InstancePosition = mul(float4(input.pos, 1), input.mTransform);
    float4 ViewPos = mul(InstancePosition, g_mWorldView );
    
    //
    // Transform the vert to view-space
    //
    float4 v4Position = mul(InstancePosition, g_mWorldViewProj);
    output.pos = v4Position;
    
    //  
    // Transfer the rest
    //
    output.tex = input.tex;
    
    //
    // dot the norm with the light dir
    //
    float3 norm = mul(input.norm,(float3x3)input.mTransform);
    output.color = CalcLighting( norm, ViewPos.z );
    
    //
    // Dim the color by how far up the tree we are.  
    // This is a nice way to fake occlusion of the branches by the leaves.
    //
    output.color *= 1.0f - saturate(input.pos.y/g_occDimHeight);
    
    
    return output;
}

//--------------------------------------------------------------------------------------
// Quad (leaf) vertex shader.  Instances the quad over multiple leaf positions and 
// multiple trees.  This demonstrates how to do double instancing.
//--------------------------------------------------------------------------------------
PSQuadIn VSQuadmain(VSQuadIn input)
{
    PSQuadIn output;
    
    // base our leaf texture upon which instance id we are
    uint iLeaf = input.InstanceId/g_iNumTrees;
    uint iLeafTex = iLeaf%3;
    output.tex = float3(input.tex, float(iLeafTex) );

    //
    // Transform the position by the Instance matrix
    //
    int iTree = input.InstanceId - (input.InstanceId/g_iNumTrees)*g_iNumTrees;
    float4 vInstancePos = mul( float4(input.pos, 1), input.mTransform  );
    float4 InstancePosition = mul(vInstancePos, g_mTreeMatrices[iTree] );
    float4 ViewPos = mul(InstancePosition, g_mWorldView );
        
    //  
    // Transform the Instance position to view-space
    //
    output.pos = mul(InstancePosition, g_mWorldViewProj);
    
    // pack distance from the eye into the color alpha channel
    output.color = float4(input.fOcc,input.fOcc,input.fOcc,ViewPos.z);
    
    return output;
}

//--------------------------------------------------------------------------------------
// Grass vertex shader.  Basically a passthrough except for instancing the island base
// mesh.
//--------------------------------------------------------------------------------------
VSGrassOut VSGrassmain(VSGrassIn input)
{
    // simple transform into the instance space
    VSGrassOut output;
    output.pos = mul(float4(input.pos, 1), input.mTransform);
    output.norm = mul(input.norm, (float3x3)input.mTransform);
    output.tex = input.tex;
    output.VertexID = input.VertexID;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Quad (leaf) GS.  Calculates the normal and lighting for the leaf.
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void GSQuadmain(triangle PSQuadIn input[3], inout TriangleStream<PSQuadIn> QuadStream)
{
    PSQuadIn output;

    //
    // Calculate the face normal
    //
    float4 faceNormalA = input[1].pos.xyzw - input[0].pos.xyzw;
    float4 faceNormalB = input[2].pos.xyzw - input[0].pos.xyzw;

    //
    // Cross product
    //
    float3 faceNormal = cross(faceNormalA, faceNormalB);

    //
    // Normalize face normal
    //  
    faceNormal = normalize(faceNormal);

    //
    // Dot face normal with some arbitrary light vectors
    //
    float4 color1 = CalcLighting( faceNormal, input[0].color.a );
    color1 *= input[0].color;

    //
    // Make sure we always have an alpha of 1
    //  
    color1.a = 1.0;

    //
    // Emit out the new tri
    //
    for(int i=0; i<3; i++)
    {
        output.pos = input[i].pos;
        output.color = color1;
        output.tex = input[i].tex;  
        QuadStream.Append(output);
    }
    QuadStream.RestartStrip();
}

//--------------------------------------------------------------------------------------
// RandomDir helper.  Samples a random dir out of our 1d random texture.  In this case
// we use a texture because the offset could be anywhere.  If we were sampling linearly
// then we would probably just use a buffer and load from that.
//--------------------------------------------------------------------------------------
float3 RandomDir(float fOffset)
{   
    float tCoord = (fOffset) / 300.0;
    return g_txRandom.SampleLevel( g_samPoint, tCoord, 0 );
}

//--------------------------------------------------------------------------------------
// Helper to determing if a point is within a triangle
//--------------------------------------------------------------------------------------
bool IsInTriangle( float3 P, float3 A, float3 B, float3 C )
{
    float3 crossA = cross( B-A, P-A );
    float3 crossB = cross( C-B, P-B );
    float3 crossC = cross( A-C, P-C );
    
    if( dot( crossA, crossB ) > 0 &&
        dot( crossB, crossC ) > 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------
// Gets a random orientation matrix based upon the RandomDir funciton
//--------------------------------------------------------------------------------------
float4x4 GetRandomOrientation( float3 Pos, float3 Norm, float fRandOffset )
{
    float3 Tangent = RandomDir(fRandOffset);
    
    float3 Bitangent = normalize( cross( Tangent, Norm ) );
    Tangent = normalize( cross( Bitangent, Norm ) );
    
    float4x4 matWorld = { float4( Tangent, 0 ),
                          float4( Norm, 0 ),
                          float4( Bitangent, 0 ),
                          float4( Pos, 1 ) };
    return matWorld;
}

//--------------------------------------------------------------------------------------
// Generates an actual grass blade
//--------------------------------------------------------------------------------------
void OutputGrassBlade( VSGrassOut midPoint, inout TriangleStream<PSQuadIn> GrassStream, int iGrassTex )
{
    PSQuadIn output;
    
    float4x4 mWorld = GetRandomOrientation( midPoint.pos, midPoint.norm, (float)midPoint.VertexID );
    float4 ViewPos = mul( midPoint.pos, g_mWorldView );
    
    float3 grassNorm = midPoint.norm;
    float4 color1 = CalcLighting( grassNorm, ViewPos.z );
    
    for(int v=0; v<6; v++)
    {
        float3 pos = g_positions[v];
        pos.x *= g_GrassWidth;
        pos.y *= g_GrassHeight;
        
        output.pos = mul( float4(pos,1), mWorld );
        output.pos = mul( output.pos, g_mWorldViewProj );
        output.tex = float3( g_texcoords[v], iGrassTex );
        output.color = color1;
    
        GrassStream.Append( output );
    }
    
    GrassStream.RestartStrip();
}

//--------------------------------------------------------------------------------------
// Midpoint of the three vertices A,B,C
//--------------------------------------------------------------------------------------
VSGrassOut CalcMidPoint( VSGrassOut A, VSGrassOut B, VSGrassOut C )
{
    VSGrassOut MidPoint;
    
    MidPoint.pos = (A.pos + B.pos + C.pos)/3.0f;
    MidPoint.norm = (A.norm + B.norm + C.norm)/3.0f;
    MidPoint.tex = (A.tex + B.tex + C.tex)/3.0f;
    MidPoint.VertexID = A.VertexID + B.VertexID + C.VertexID;
    
    return MidPoint;
}

//--------------------------------------------------------------------------------------
// The actual grass geometry shader.  This generates grass blades based upon an input
// mesh (the tops of the islands) and a coverage texture.  Each of the textures channels
// determines how much of each of the 4 types of grass to place at a particular spot.
//--------------------------------------------------------------------------------------
[maxvertexcount(90)]
void GSGrassmain(triangle VSGrassOut input[3], inout TriangleStream<PSQuadIn> GrassStream )
{
    VSGrassOut MidPoint = CalcMidPoint( input[0], input[1], input[2] );
    
    float4 CoverageMask = g_tx2dArray.SampleLevel( g_samPoint, float3(MidPoint.tex,4), 0 );
    float cm[4];
    cm[0] = CoverageMask.r;
    cm[1] = CoverageMask.g;
    cm[2] = CoverageMask.b;
    cm[3] = CoverageMask.a;
    
    for(int g=0; g<4; g++)
    {
        float MaxBlades = float(g_iGrassCoverage)*cm[g];
        for(float i=0; i<MaxBlades; i++)
        {	
            float randOffset = g*5 + (i+1);
            float3 Tan = RandomDir( MidPoint.pos.x + randOffset );
            float3 Len = normalize( RandomDir( MidPoint.pos.z + randOffset ) );
            float3 Shift = Len.x*g_GrassMessiness*normalize( cross( Tan, MidPoint.norm ) );
            VSGrassOut grassPoint = MidPoint;
            grassPoint.VertexID += randOffset;
            grassPoint.pos += Shift; 
                
            //uncomment this to make the grass strictly conform to the mesh
            //if( IsInTriangle( grassPoint.pos, input[0].pos, input[1].pos, input[2].pos ) )
            {
                OutputGrassBlade( grassPoint, GrassStream, g );
            }
        }
    }
}

//--------------------------------------------------------------------------------------
// PS for non-leaf or grass items.
//--------------------------------------------------------------------------------------
float4 PSScenemain(PSSceneIn input) : SV_Target
{
    float4 color = g_txDiffuse.Sample( g_samLinear, input.tex ) * input.color;
    return color;
}

//--------------------------------------------------------------------------------------
// PS for leaves and grass
//--------------------------------------------------------------------------------------
float4 PSQuadmain(PSQuadIn input) : SV_Target
{
    float4 color = g_tx2dArray.Sample( g_samLinear, input.tex );
    color.xyz *= input.color.xyz;
    return color;
}

//--------------------------------------------------------------------------------------
// Render instanced meshes with vertex lighting
//--------------------------------------------------------------------------------------
technique10 RenderInstancedVertLighting
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSInstmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAA );
    }  
}

//--------------------------------------------------------------------------------------
// Skybox
//--------------------------------------------------------------------------------------
technique10 RenderSkybox
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSSkymain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAA );
    }  
}

//--------------------------------------------------------------------------------------
// Render leaves
//--------------------------------------------------------------------------------------
technique10 RenderQuad
{
    pass p0
    {
        
        SetVertexShader( CompileShader( vs_4_0, VSQuadmain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSQuadmain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSQuadmain() ) );
        
        SetBlendState( QuadAlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAA );
    }  
}

//--------------------------------------------------------------------------------------
// Render grass
//--------------------------------------------------------------------------------------
technique10 RenderGrass
{
    pass p0
    {
        
        SetVertexShader( CompileShader( vs_4_0, VSGrassmain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassmain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSQuadmain() ) );
        
        SetBlendState( QuadAlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAA );
    }  
}
