//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Falcor's texture LOD setup asks for the dimensions at a mip and the number of levels.
// Keep the mip dynamic and check uint, int and float output overloads independently.
// Mip overloads retain CUDA's existing zero layer-count output for arrays.
//TEST_INPUT: Texture2D(size=16, content=one, mipMaps=5):name texture
Texture2D<float4> texture;
//TEST_INPUT: Texture1D(size=16, content=one, arrayLength=3, mipMaps=5):name array1D
Texture1DArray<float4> array1D;
//TEST_INPUT: Texture2D(size=16, content=one, arrayLength=3, mipMaps=5):name array2D
Texture2DArray<float4> array2D;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void raygenMain()
{
    uint mip = DispatchRaysIndex().x;
    uint w, h, levels;
    int iw, ih, ilevels;
    float fw, fh, flevels;
    texture.GetDimensions(mip, w, h, levels);
    texture.GetDimensions(mip, iw, ih, ilevels);
    texture.GetDimensions(mip, fw, fh, flevels);
    uint aw, ah, layers, alevels;
    float bw, blayers, blevels;
    array2D.GetDimensions(mip, aw, ah, layers, alevels);
    array1D.GetDimensions(mip, bw, blayers, blevels);
    uint expected = 16 >> mip;
    output[mip] = uint(w == expected && h == expected && levels == 5 &&
        iw == int(expected) && ih == int(expected) && ilevels == 5 &&
        fw == float(expected) && fh == float(expected) && flevels == 5 &&
        aw == expected && ah == expected && alevels == 5 && layers == 0 &&
        bw == float(expected) && blevels == 5 && blayers == 0);
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
