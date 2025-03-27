//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj

//TEST_INPUT: Texture1D(size=4, content = one):name t1D
Texture1D<float> t1D;
//TEST_INPUT: Texture2D(size=8, content = one):name t2D
Texture2D<float> t2D;
//TEST_INPUT: Texture3D(size=2, content = one):name t3D
Texture3D<float> t3D;
//TEST_INPUT: TextureCube(size=16, content = one):name tCube
TextureCube<float> tCube;

//TEST_INPUT: Texture1D(size=2, content = one, arrayLength=2):name t1DArray
Texture1DArray<float> t1DArray;
//TEST_INPUT: Texture2D(size=4, content = one, arrayLength=3):name t2DArray
Texture2DArray<float> t2DArray;
//TEST_INPUT: TextureCube(size=16, content = one, arrayLength=1):name tCubeArray
TextureCubeArray<float> tCubeArray;

//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    uint mipWidth = 0;
    uint mipHeight = 0;
    uint mipDepth = 0;
    uint mipCount = 1;
    uint elementCount = 0;
    
    uint width = 0;
    uint height = 0;
    uint depth = 0;
    
    switch (idx)
    {
        case 0:
        {
            t1D.GetDimensions(width);
            t1D.GetDimensions(0, mipWidth, mipCount);
            break;
        }      
    }
   
    width = (mipWidth != width) ? ~0 : width;
    height = (mipHeight != height) ? ~0 : height;
    depth = (mipDepth != depth) ? ~0 : depth;
   
    // Only depth or element count can be set
    if (depth == 0 && elementCount > 0)
    {
        depth = elementCount;
    }
   
    uint val = ((0xff & width) << 24) | ((0xff & height) << 16) | ((0xff & depth) << 8) | (mipCount);
   
    outputBuffer[idx] = val;
}
