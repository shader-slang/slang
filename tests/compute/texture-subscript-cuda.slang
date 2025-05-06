// Test for verifying subscript operator support in cuda.

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST_INPUT: Texture1D(size=4, content = one):name cudaT1D
Texture1D<float> cudaT1D;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D
Texture2D<float> cudaT2D;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D_f2
Texture2D<float2> cudaT2D_f2;
//TEST_INPUT: Texture2D(size=8, content = one):name cudaT2D_f4
Texture2D<float4> cudaT2D_f4;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D
Texture3D<float> cudaT3D;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D_f2
Texture3D<float2> cudaT3D_f2;
//TEST_INPUT: Texture3D(size=8, content = one):name cudaT3D_f4
Texture3D<float4> cudaT3D_f4;
//TEST_INPUT: TextureCube(size=16, content = one):name cudaTCube
TextureCube<float> cudaTCube;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray
Texture2DArray<float> cudaT2DArray;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray_f2
Texture2DArray<float2> cudaT2DArray_f2;
//TEST_INPUT: Texture2D(size=16, content = one, arrayLength=3):name cudaT2DArray_f4
Texture2DArray<float4> cudaT2DArray_f4;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name cudaOutputBuffer
RWStructuredBuffer<float> cudaOutputBuffer;

[numthreads(7, 1, 1)]
[shader("compute")]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    
	switch (idx)
	{
	case 1:
		{
			int var = 0;
			float result = cudaT1D[0];
			// This is not supported in PTX.
			//cudaOutputBuffer[idx] = result;
		}	
	break;
	
	case 2:
		{
			int2 var = int2(1, 2);
			float result = cudaT2D[var];
			cudaOutputBuffer[idx] = result;
			
			float2 result2 = cudaT2D_f2[var];
			cudaOutputBuffer[idx] += result2.x;
			cudaOutputBuffer[idx] += result2.y;

			float4 result4 = cudaT2D_f4[var];
			cudaOutputBuffer[idx] += result4.x;
			cudaOutputBuffer[idx] += result4.y;
			cudaOutputBuffer[idx] += result4.z;
			cudaOutputBuffer[idx] += result4.w;
		}
	break;

	case 3:
		{
			int3 var = int3(1, 1, 1);
			float result = cudaT3D[var];
			cudaOutputBuffer[idx] = result;
			
			float2 result2 = cudaT3D_f2[var];
			cudaOutputBuffer[idx] += result2.x;
			cudaOutputBuffer[idx] += result2.y;

			float4 result4 = cudaT3D_f4[var];
			cudaOutputBuffer[idx] += result4.x;
			cudaOutputBuffer[idx] += result4.y;
			cudaOutputBuffer[idx] += result4.z;
			cudaOutputBuffer[idx] += result4.w;
		}
	break;

	case 4:
		{
			int3 var = int3(0, 0, 1);
			float result = cudaT2DArray[var];
			cudaOutputBuffer[idx] = result;
			
			float2 result2 = cudaT2DArray_f2[var];
			cudaOutputBuffer[idx] += result2.x;
			cudaOutputBuffer[idx] += result2.y;

			float4 result4 = cudaT2DArray_f4[var];
			cudaOutputBuffer[idx] += result4.x;
			cudaOutputBuffer[idx] += result4.y;
			cudaOutputBuffer[idx] += result4.z;
			cudaOutputBuffer[idx] += result4.w;
		}
	break;
	}
} 