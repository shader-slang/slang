// Test for verifying subscript operator support with uint types in cuda.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
//TEST_INPUT: Texture1D(size=4, content = one):name cudaT1D_u
Texture1D<uint> cudaT1D_u;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D_u
Texture2D<uint> cudaT2D_u;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D_u2
Texture2D<uint2> cudaT2D_u2;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D_u4
Texture2D<uint4> cudaT2D_u4;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D_u
Texture3D<uint> cudaT3D_u;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D_u2
Texture3D<uint2> cudaT3D_u2;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D_u4
Texture3D<uint4> cudaT3D_u4;
//TEST_INPUT: TextureCube(size=16, content = one):name cudaTCube_u
TextureCube<uint> cudaTCube_u;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray_u
Texture2DArray<uint> cudaT2DArray_u;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray_u2
Texture2DArray<uint2> cudaT2DArray_u2;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray_u4
Texture2DArray<uint4> cudaT2DArray_u4;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name cudaOutputBuffer
RWStructuredBuffer<uint> cudaOutputBuffer;

[numthreads(3, 1, 1)]
[shader("compute")]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
    switch (idx)
    {   
    case 0:
    {
        int2 var = int2(1, 2);
        uint result = cudaT2D_u[var];
        cudaOutputBuffer[idx] = result;
            
        uint2 result2 = cudaT2D_u2[var];
        cudaOutputBuffer[idx] += result2.x;

        uint4 result4 = cudaT2D_u4[var];
        cudaOutputBuffer[idx] += result4.x;
        cudaOutputBuffer[idx] += result4.y;
    }
    break;

    case 1:
    {
        int3 var = int3(1, 1, 1);
        uint result = cudaT3D_u[var];
        cudaOutputBuffer[idx] = result;
            
        uint2 result2 = cudaT3D_u2[var];
        cudaOutputBuffer[idx] += result2.x;

        uint4 result4 = cudaT3D_u4[var];
        cudaOutputBuffer[idx] += result4.x;
        cudaOutputBuffer[idx] += result4.w;
    }
    break;

    case 2:
    {
        int3 var = int3(0, 0, 1);
        uint result = cudaT2DArray_u[var];
        cudaOutputBuffer[idx] = result;
            
        uint2 result2 = cudaT2DArray_u2[var];
        cudaOutputBuffer[idx] += result2.y;

        uint4 result4 = cudaT2DArray_u4[var];
        cudaOutputBuffer[idx] += result4.x;
        cudaOutputBuffer[idx] += result4.y;
    }
    break;
    }
} 
// CHECK:FE000000
// CHECK:FE000000
// CHECK:FE000000