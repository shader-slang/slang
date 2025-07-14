#include "/home/runner/work/slang/slang/prelude/slang-cuda-prelude.h"


#line 6679 "hlsl.meta.slang"
__device__ bool any_0(bool3  x_0)
{

#line 6679
    bool result_0 = false;

#line 6679
    int i_0 = int(0);

#line 6719
    for(;;)
    {

#line 6719
        if(i_0 < int(3))
        {
        }
        else
        {

#line 6719
            break;
        }

#line 6720
        if(result_0)
        {

#line 6720
            result_0 = true;

#line 6720
        }
        else
        {

#line 6720
            result_0 = (bool((_slang_vector_get_element(x_0, i_0))));

#line 6720
        }

#line 6719
        i_0 = i_0 + int(1);

#line 6719
    }

    return result_0;
}


#line 5 "test-texture-types.slang"
extern "C" __global__ void copyTexture(CUtexObject srcTexture_0, CUsurfObject dstTexture_0)
{

    uint3  _S1 = uint3 {(blockIdx * blockDim + threadIdx).x, (blockIdx * blockDim + threadIdx).y, (blockIdx * blockDim + threadIdx).z};

    uint3  srcDims_0;
    {uint32_t w, h, d; asm("txq.width.b32 %0, [%3]; txq.height.b32 %1, [%3]; txq.depth.b32 %2, [%3];" : "=r"(w), "=r"(h), "=r"(d) : "l"((srcTexture_0))); *((&((&srcDims_0)->x))) = w;*((&((&srcDims_0)->y))) = h;*((&((&srcDims_0)->z))) = d;};
    uint3  dstDims_0;
    {uint32_t w, h, d; asm("txq.width.b32 %0, [%3]; txq.height.b32 %1, [%3]; txq.depth.b32 %2, [%3];" : "=r"(w), "=r"(h), "=r"(d) : "l"((srcTexture_0))); *((&((&dstDims_0)->x))) = w;*((&((&dstDims_0)->y))) = h;*((&((&dstDims_0)->z))) = d;};
    if(any_0(srcDims_0 != dstDims_0))
    {

#line 15
        return;
    }

#line 16
    if(any_0(_S1 >= dstDims_0))
    {

#line 17
        return;
    }

#line 18
    uint4  _S2 = make_uint4 (_S1.x, _S1.y, _S1.z, 0U);

#line 18
    int4  _S3 = make_int4 ((int)_S2.x, (int)_S2.y, (int)_S2.z, (int)_S2.w);

#line 18
    surf3Dwrite_convert<uint>(((tex3Dfetch_int<uint>((srcTexture_0), ((_S3)).x, ((_S3)).y, ((_S3)).z))), (dstTexture_0), ((_S1)).x * 1, ((_S1)).y, ((_S1)).z, SLANG_CUDA_BOUNDARY_MODE);
    return;
}

