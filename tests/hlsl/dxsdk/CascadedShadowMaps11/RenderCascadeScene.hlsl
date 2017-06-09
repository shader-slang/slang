//TEST_IGNORE_FILE: Currently failing due to Spire compiler issues.
//TEST:COMPARE_HLSL: -target dxbc-assembly -profile vs_4_0 -entry VSMain -profile ps_4_0 -entry PSMain
//--------------------------------------------------------------------------------------
// File: RenderCascadeScene.hlsl
//
// This is the main shader file.  This shader is compiled with several different flags 
// to provide different customizations based on user controls.
// 
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// This flag uses the derivative information to map the texels in a shadow map to the
// view space plane of the primitive being rendred.  This depth is then used as the 
// comparison depth and reduces self shadowing aliases.  This  technique is expensive
// and is only valid when objects are planer ( such as a ground plane ).
#ifndef USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG
#define USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG 0
#endif

// This flag enables the shadow to blend between cascades.  This is most useful when the 
// the shadow maps are small and artifact can be seen between the various cascade layers.
#ifndef BLEND_BETWEEN_CASCADE_LAYERS_FLAG
#define BLEND_BETWEEN_CASCADE_LAYERS_FLAG 0
#endif

// There are two methods for selecting the proper cascade a fragment lies in.  Interval selection
// compares the depth of the fragment against the frustum's depth partition.
// Map based selection compares the texture coordinates against the acutal cascade maps.
// Map based selection gives better coverage.  
// Interval based selection is easier to extend and understand.
#ifndef SELECT_CASCADE_BY_INTERVAL_FLAG
#define SELECT_CASCADE_BY_INTERVAL_FLAG 0
#endif

// The number of cascades 
#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif


// Most titles will find that 3-4 cascades with 
// BLEND_BETWEEN_CASCADE_LAYERS_FLAG, is good for lower end PCs.
// High end PCs will be able to handle more cascades, and larger blur bands.
// In some cases such as when large PCF kernels are used, derivative based depth offsets could be used 
// with larger PCF blur kernels on high end PCs for the ground plane.

cbuffer cbAllShadowData : register( b0 )
{
    matrix          m_mWorldViewProjection;
    matrix          m_mWorld;
    matrix          m_mWorldView;
    matrix          m_mShadow;
    float4          m_vCascadeOffset[8];
    float4          m_vCascadeScale[8];
    int             m_nCascadeLevels; // Number of Cascades
    int             m_iVisualizeCascades; // 1 is to visualize the cascades in different colors. 0 is to just draw the scene
    int             m_iPCFBlurForLoopStart; // For loop begin value. For a 5x5 Kernal this would be -2.
    int             m_iPCFBlurForLoopEnd; // For loop end value. For a 5x5 kernel this would be 3.

    // For Map based selection scheme, this keeps the pixels inside of the the valid range.
    // When there is no boarder, these values are 0 and 1 respectivley.
    float           m_fMinBorderPadding;     
    float           m_fMaxBorderPadding;
    float           m_fShadowBiasFromGUI;  // A shadow map offset to deal with self shadow artifacts.  
                                           //These artifacts are aggravated by PCF.
    float           m_fShadowPartitionSize; 
    float           m_fCascadeBlendArea; // Amount to overlap when blending between cascades.
    float           m_fTexelSize; 
    float           m_fNativeTexelSizeInX;
    float           m_fPaddingForCB3; // Padding variables exist because CBs must be a multiple of 16 bytes.
    float4          m_fCascadeFrustumsEyeSpaceDepthsFloat[2];  // The values along Z that seperate the cascades.
    float4          m_fCascadeFrustumsEyeSpaceDepthsFloat4[8];  // the values along Z that separte the cascades.  
                                                          // Wastefully stored in float4 so they are array indexable. 
    float3          m_vLightDir;
    float           m_fPaddingCB4;

};



//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txDiffuse                    : register( t0 );
Texture2D    g_txShadow                     : register( t5 );


SamplerState g_samLinear                    : register( s0 );
SamplerComparisonState g_samShadow          : register( s5 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 vPosition                        : POSITION;
    float3 vNormal                          : NORMAL;
    float2 vTexcoord                        : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 vNormal                          : NORMAL;
    float2 vTexcoord                        : TEXCOORD0;
    float4 vTexShadow					    : TEXCOORD1;
    float4 vPosition                        : SV_POSITION;
    float4 vInterpPos                       : TEXCOORD2; 
    float  vDepth                           : TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;

    Output.vPosition = mul( Input.vPosition, m_mWorldViewProjection );
    Output.vNormal = mul( Input.vNormal, (float3x3)m_mWorld );
    Output.vTexcoord = Input.vTexcoord;
    Output.vInterpPos = Input.vPosition;   
    Output.vDepth = mul( Input.vPosition, m_mWorldView ).z ; 
       
    // Transform the shadow texture coordinates for all the cascades.
    Output.vTexShadow = mul( Input.vPosition, m_mShadow );
    return Output;
    
}



static const float4 vCascadeColorsMultiplier[8] = 
{
    float4 ( 1.5f, 0.0f, 0.0f, 1.0f ),
    float4 ( 0.0f, 1.5f, 0.0f, 1.0f ),
    float4 ( 0.0f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 1.5f, 0.0f, 1.0f ),
    float4 ( 1.0f, 1.0f, 1.0f, 1.0f ),
    float4 ( 0.0f, 1.0f, 5.5f, 1.0f ),
    float4 ( 0.5f, 3.5f, 0.75f, 1.0f )
};


void ComputeCoordinatesTransform( in int iCascadeIndex,
                                      in float4 InterpolatedPosition ,
                                      in out float4 vShadowTexCoord , 
                                      in out float4 vShadowTexCoordViewSpace ) 
{
    // Now that we know the correct map, we can transform the world space position of the current fragment                
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        vShadowTexCoord = vShadowTexCoordViewSpace * m_vCascadeScale[iCascadeIndex];
        vShadowTexCoord += m_vCascadeOffset[iCascadeIndex];
    }  
          
    vShadowTexCoord.x *= m_fShadowPartitionSize;  // precomputed (float)iCascadeIndex / (float)CASCADE_CNT
    vShadowTexCoord.x += (m_fShadowPartitionSize * (float)iCascadeIndex ); 


} 


//--------------------------------------------------------------------------------------
// This function calculates the screen space depth for shadow space texels
//--------------------------------------------------------------------------------------
void CalculateRightAndUpTexelDepthDeltas ( in float3 vShadowTexDDX,
                                           in float3 vShadowTexDDY,
                                           out float fUpTextDepthWeight,
                                           out float fRightTextDepthWeight
 ) {
        
    // We use the derivatives in X and Y to create a transformation matrix.  Because these derivives give us the 
    // transformation from screen space to shadow space, we need the inverse matrix to take us from shadow space 
    // to screen space.  This new matrix will allow us to map shadow map texels to screen space.  This will allow 
    // us to find the screen space depth of a corresponding depth pixel.
    // This is not a perfect solution as it assumes the underlying geometry of the scene is a plane.  A more 
    // accureate way of finding the actual depth would be to do a deferred rendering approach and actually 
    //sample the depth.
    
    // Using an offset, or using variance shadow maps is a better approach to reducing these artifacts in most cases.
    
    float2x2 matScreentoShadow = float2x2( vShadowTexDDX.xy, vShadowTexDDY.xy );
    float fDeterminant = determinant ( matScreentoShadow );
    
    float fInvDeterminant = 1.0f / fDeterminant;
    
    float2x2 matShadowToScreen = float2x2 (
        matScreentoShadow._22 * fInvDeterminant, matScreentoShadow._12 * -fInvDeterminant, 
        matScreentoShadow._21 * -fInvDeterminant, matScreentoShadow._11 * fInvDeterminant );

    float2 vRightShadowTexelLocation = float2( m_fTexelSize, 0.0f );
    float2 vUpShadowTexelLocation = float2( 0.0f, m_fTexelSize );  
    
    // Transform the right pixel by the shadow space to screen space matrix.
    float2 vRightTexelDepthRatio = mul( vRightShadowTexelLocation,  matShadowToScreen );
    float2 vUpTexelDepthRatio = mul( vUpShadowTexelLocation,  matShadowToScreen );

    // We can now caculate how much depth changes when you move up or right in the shadow map.
    // We use the ratio of change in x and y times the dervivite in X and Y of the screen space 
    // depth to calculate this change.
    fUpTextDepthWeight = 
        vUpTexelDepthRatio.x * vShadowTexDDX.z 
        + vUpTexelDepthRatio.y * vShadowTexDDY.z;
    fRightTextDepthWeight = 
        vRightTexelDepthRatio.x * vShadowTexDDX.z 
        + vRightTexelDepthRatio.y * vShadowTexDDY.z;
        
}


//--------------------------------------------------------------------------------------
// Use PCF to sample the depth map and return a percent lit value.
//--------------------------------------------------------------------------------------
void CalculatePCFPercentLit ( in float4 vShadowTexCoord, 
                              in float fRightTexelDepthDelta, 
                              in float fUpTexelDepthDelta, 
                              in float fBlurRowSize,
                              out float fPercentLit
                              ) 
{
    fPercentLit = 0.0f;
    // This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
    // This would be performance improvment.
    for( int x = m_iPCFBlurForLoopStart; x < m_iPCFBlurForLoopEnd; ++x ) 
    {
        for( int y = m_iPCFBlurForLoopStart; y < m_iPCFBlurForLoopEnd; ++y ) 
        {
            float depthcompare = vShadowTexCoord.z;
            // A very simple solution to the depth bias problems of PCF is to use an offset.
            // Unfortunately, too much offset can lead to Peter-panning (shadows near the base of object disappear )
            // Too little offset can lead to shadow acne ( objects that should not be in shadow are partially self shadowed ).
            depthcompare -= m_fShadowBiasFromGUI;
            if ( USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG ) 
            {
                // Add in derivative computed depth scale based on the x and y pixel.
                depthcompare += fRightTexelDepthDelta * ( (float) x ) + fUpTexelDepthDelta * ( (float) y );
            }
            // Compare the transformed pixel depth to the depth read from the map.
            fPercentLit += g_txShadow.SampleCmpLevelZero( g_samShadow, 
                float2( 
                    vShadowTexCoord.x + ( ( (float) x ) * m_fNativeTexelSizeInX ) , 
                    vShadowTexCoord.y + ( ( (float) y ) * m_fTexelSize ) 
                    ), 
                depthcompare );
        }
    }
    fPercentLit /= (float)fBlurRowSize;
}

//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval ( in int iCurrentCascadeIndex, 
                                       in out float fPixelDepth, 
                                       in out float fCurrentPixelsBlendBandLocation,
                                       out float fBlendBetweenCascadesAmount
                                       ) 
{

    // We need to calculate the band of the current shadow map where it will fade into the next cascade.
    // We can then early out of the expensive PCF for loop.
    // 
    float fBlendInterval = m_fCascadeFrustumsEyeSpaceDepthsFloat4[ iCurrentCascadeIndex  ].x;
    //if( iNextCascadeIndex > 1 ) 
    int fBlendIntervalbelowIndex = min(0, iCurrentCascadeIndex-1);
    fPixelDepth -= m_fCascadeFrustumsEyeSpaceDepthsFloat4[ fBlendIntervalbelowIndex ].x;
    fBlendInterval -= m_fCascadeFrustumsEyeSpaceDepthsFloat4[ fBlendIntervalbelowIndex ].x;
    
    // The current pixel's blend band location will be used to determine when we need to blend and by how much.
    fCurrentPixelsBlendBandLocation = fPixelDepth / fBlendInterval;
    fCurrentPixelsBlendBandLocation = 1.0f - fCurrentPixelsBlendBandLocation;
    // The fBlendBetweenCascadesAmount is our location in the blend band.
    fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / m_fCascadeBlendArea;
}



//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForMap ( in float4 vShadowMapTextureCoord, 
                                  in out float fCurrentPixelsBlendBandLocation,
                                  out float fBlendBetweenCascadesAmount ) 
{
    // Calcaulte the blend band for the map based selection.
    float2 distanceToOne = float2 ( 1.0f - vShadowMapTextureCoord.x, 1.0f - vShadowMapTextureCoord.y );
    fCurrentPixelsBlendBandLocation = min( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y );
    float fCurrentPixelsBlendBandLocation2 = min( distanceToOne.x, distanceToOne.y );
    fCurrentPixelsBlendBandLocation = 
        min( fCurrentPixelsBlendBandLocation, fCurrentPixelsBlendBandLocation2 );
    fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / m_fCascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// Calculate the shadow based on several options and rende the scene.
//--------------------------------------------------------------------------------------
float4 PSMain( VS_OUTPUT Input ) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    
    float4 vShadowMapTextureCoord = 0.0f;
    float4 vShadowMapTextureCoord_blend = 0.0f;
    
    float4 vVisualizeCascadeColor = float4(0.0f,0.0f,0.0f,1.0f);
    
    float fPercentLit = 0.0f;
    float fPercentLit_blend = 0.0f;

   
    float fUpTextDepthWeight=0;
    float fRightTextDepthWeight=0;
    float fUpTextDepthWeight_blend=0;
    float fRightTextDepthWeight_blend=0;

    int iBlurRowSize = m_iPCFBlurForLoopEnd - m_iPCFBlurForLoopStart;
    iBlurRowSize *= iBlurRowSize;
    float fBlurRowSize = (float)iBlurRowSize;
        
    int iCascadeFound = 0;
    int iNextCascadeIndex = 1;

    float fCurrentPixelDepth;

    // The interval based selection technique compares the pixel's depth against the frustum's cascade divisions.
    fCurrentPixelDepth = Input.vDepth;
    
    // This for loop is not necessary when the frustum is uniformaly divided and interval based selection is used.
    // In this case fCurrentPixelDepth could be used as an array lookup into the correct frustum. 
    int iCurrentCascadeIndex;
    
    float4 vShadowMapTextureCoordViewSpace = Input.vTexShadow;
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        iCurrentCascadeIndex = 0;
        if ( CASCADE_COUNT_FLAG > 1 ) 
        {
            float4 vCurrentPixelDepth = Input.vDepth;
            float4 fComparison = ( vCurrentPixelDepth > m_fCascadeFrustumsEyeSpaceDepthsFloat[0]);
            float4 fComparison2 = ( vCurrentPixelDepth > m_fCascadeFrustumsEyeSpaceDepthsFloat[1]);
            float fIndex = dot( 
                            float4( CASCADE_COUNT_FLAG > 0,
                                    CASCADE_COUNT_FLAG > 1, 
                                    CASCADE_COUNT_FLAG > 2, 
                                    CASCADE_COUNT_FLAG > 3)
                            , fComparison )
                         + dot( 
                            float4(
                                    CASCADE_COUNT_FLAG > 4,
                                    CASCADE_COUNT_FLAG > 5,
                                    CASCADE_COUNT_FLAG > 6,
                                    CASCADE_COUNT_FLAG > 7)
                            , fComparison2 ) ;
                                    
            fIndex = min( fIndex, CASCADE_COUNT_FLAG - 1 );
            iCurrentCascadeIndex = (int)fIndex;
        }
    }
    
    if ( !SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        iCurrentCascadeIndex = 0;
        if ( CASCADE_COUNT_FLAG  == 1 ) 
        {
            vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * m_vCascadeScale[0];
            vShadowMapTextureCoord += m_vCascadeOffset[0];
        }
        if ( CASCADE_COUNT_FLAG > 1 ) {
            for( int iCascadeIndex = 0; iCascadeIndex < CASCADE_COUNT_FLAG && iCascadeFound == 0; ++iCascadeIndex ) 
            {
                vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * m_vCascadeScale[iCascadeIndex];
                vShadowMapTextureCoord += m_vCascadeOffset[iCascadeIndex];

                if ( min( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y ) > m_fMinBorderPadding
                  && max( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y ) < m_fMaxBorderPadding )
                { 
                    iCurrentCascadeIndex = iCascadeIndex;   
                    iCascadeFound = 1; 
                }
            }
        }
    }    
    
    float4 color = 0;   
  
    if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG  ) 
    {
        // Repeat text coord calculations for the next cascade. 
        // The next cascade index is used for blurring between maps.
        iNextCascadeIndex = min ( CASCADE_COUNT_FLAG - 1, iCurrentCascadeIndex + 1 ); 
    }            

    float fBlendBetweenCascadesAmount = 1.0f;
    float fCurrentPixelsBlendBandLocation = 1.0f;
    
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1  ) 
         {
            CalculateBlendAmountForInterval ( iCurrentCascadeIndex, fCurrentPixelDepth, 
                fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
        }   
    }
    else 
    {
    
        if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG ) 
        {
            CalculateBlendAmountForMap ( vShadowMapTextureCoord, 
                fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
        }   
    }
    
    float3 vShadowMapTextureCoordDDX;
    float3 vShadowMapTextureCoordDDY;
    // The derivatives are used to find the slope of the current plane.
    // The derivative calculation has to be inside of the loop in order to prevent divergent flow control artifacts.
    if( USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG ) 
    {
        vShadowMapTextureCoordDDX = ddx( vShadowMapTextureCoordViewSpace );
        vShadowMapTextureCoordDDY = ddy( vShadowMapTextureCoordViewSpace );    
        
        vShadowMapTextureCoordDDX *= m_vCascadeScale[iCurrentCascadeIndex];
        vShadowMapTextureCoordDDY *= m_vCascadeScale[iCurrentCascadeIndex];
    }    
    
    ComputeCoordinatesTransform( iCurrentCascadeIndex, 
                                 Input.vInterpPos, 
                                 vShadowMapTextureCoord, 
                                 vShadowMapTextureCoordViewSpace );    
                                 

    vVisualizeCascadeColor = vCascadeColorsMultiplier[iCurrentCascadeIndex];
         
    if( USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG ) 
    {
         CalculateRightAndUpTexelDepthDeltas ( vShadowMapTextureCoordDDX, vShadowMapTextureCoordDDY,
                                              fUpTextDepthWeight, fRightTextDepthWeight );
    }
    
    CalculatePCFPercentLit ( vShadowMapTextureCoord, fRightTextDepthWeight, 
                            fUpTextDepthWeight, fBlurRowSize, fPercentLit );
                                             
    if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1 ) 
    {
        if( fCurrentPixelsBlendBandLocation < m_fCascadeBlendArea) 
        {  // the current pixel is within the blend band.
    
            // Repeat text coord calculations for the next cascade. 
            // The next cascade index is used for blurring between maps.
            if( !SELECT_CASCADE_BY_INTERVAL_FLAG ) 
            {
                vShadowMapTextureCoord_blend = vShadowMapTextureCoordViewSpace * m_vCascadeScale[iNextCascadeIndex];
                vShadowMapTextureCoord_blend += m_vCascadeOffset[iNextCascadeIndex];
            }
            
            ComputeCoordinatesTransform( iNextCascadeIndex, Input.vInterpPos, 
                                             vShadowMapTextureCoord_blend, 
										     vShadowMapTextureCoordViewSpace );  
       
        // We repeat the calcuation for the next cascade layer, when blending between maps.
            if( fCurrentPixelsBlendBandLocation < m_fCascadeBlendArea) 
            {  // the current pixel is within the blend band.
                if( USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG ) 
                {

                    CalculateRightAndUpTexelDepthDeltas ( vShadowMapTextureCoordDDX,
                                                          vShadowMapTextureCoordDDY,
                                                          fUpTextDepthWeight_blend,
                                                          fRightTextDepthWeight_blend );
                }   
                CalculatePCFPercentLit ( vShadowMapTextureCoord_blend, fRightTextDepthWeight_blend, 
                                        fUpTextDepthWeight_blend, fBlurRowSize, fPercentLit_blend );
                fPercentLit = lerp( fPercentLit_blend, fPercentLit, fBlendBetweenCascadesAmount ); 
                // Blend the two calculated shadows by the blend amount.
            }   
        }   
    }    

    
    if( !m_iVisualizeCascades ) vVisualizeCascadeColor = float4(1.0f,1.0f,1.0f,1.0f);
    
    float3 vLightDir1 = float3( -1.0f, 1.0f, -1.0f ); 
    float3 vLightDir2 = float3( 1.0f, 1.0f, -1.0f ); 
    float3 vLightDir3 = float3( 0.0f, -1.0f, 0.0f );
    float3 vLightDir4 = float3( 1.0f, 1.0f, 1.0f );     
    // Some ambient-like lighting.
    float fLighting = 
                      saturate( dot( vLightDir1 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir2 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir3 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir4 , Input.vNormal ) )*0.05f ;
    
    float4 vShadowLighting = fLighting * 0.5f;
    fLighting += saturate( dot( m_vLightDir , Input.vNormal ) );
    fLighting = lerp( vShadowLighting, fLighting, fPercentLit );
    
    return fLighting * vVisualizeCascadeColor * vDiffuse;

}

