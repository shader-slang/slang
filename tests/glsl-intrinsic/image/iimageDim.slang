//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 1) in;

//TEST_INPUT: RWTexture1D(format=R32_SINT, size=4, content=one, mipMaps = 1):name i_1d
uniform layout(binding=0,r32i) iimage1D i_1d;
//TEST_INPUT: RWTextureBuffer(format=R32_SINT, stride=8, data=[1 1 1 1]):name i_ib
uniform layout(binding=1,r32i) iimageBuffer i_ib;
//TEST_INPUT: RWTexture1D(format=R32_SINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_1da
uniform layout(binding=2,r32i) iimage1DArray i_1da;
//TEST_INPUT: RWTexture2D(format=R32_SINT, size=4, content=one, mipMaps = 1):name i_2d
uniform layout(binding=3,r32i) iimage2D i_2d;
//TEST_INPUT: RWTexture2D(format=R32_SINT, size=4, content=one, mipMaps = 1):name i_2dr
uniform layout(binding=4,r32i) iimage2DRect i_2dr;
//TEST_INPUT: RWTexture2D(format=R32_SINT, size=4, content=one, mipMaps = 1, sampleCount = two):name i_2dms
uniform layout(binding=5,r32i) iimage2DMS i_2dms;
//TEST_INPUT: RWTexture2D(format=R32_SINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_2da
uniform layout(binding=6,r32i) iimage2DArray i_2da;
//TEST_INPUT: RWTexture3D(format=R32_SINT, size=4, content=one, mipMaps = 1):name i_3d
uniform layout(binding=7,r32i) iimage3D i_3d;
//TEST_INPUT: RWTextureCube(format=R32_SINT, size=4, content=one, mipMaps = 1):name i_c
uniform layout(binding=8,r32i) iimageCube i_c;
//TEST_INPUT: RWTextureCube(format=R32_SINT, size=4, content=one, mipMaps = 1, arrayLength=2):name i_ca
uniform layout(binding=9,r32i) iimageCubeArray i_ca;
//TEST_INPUT: RWTexture2D(format=R32_SINT, size=4, content=one, mipMaps = 1, arrayLength=2, sampleCount = two):name i_2dmsa
uniform layout(binding=10,r32i) iimage2DMSArray i_2dmsa;

bool checkAllImageSize()
{
    return true
        && imageSize(i_1d) == int(4)
        && imageSize(i_ib) == int(4)
        && imageSize(i_1da) == ivec2(4,2)
        && imageSize(i_2d) == ivec2(4)
        && imageSize(i_2da) == ivec3(4,4,2)
        && imageSize(i_2dr) == ivec2(4)
        && imageSize(i_2dms) == ivec2(4)
        && imageSize(i_3d) == ivec3(4)
        && imageSize(i_c) == ivec2(4)
        && imageSize(i_ca) == ivec3(4,4,2)
        && imageSize(i_2dmsa) == ivec3(4,4,2)
        ;
}
bool checkAllImageLoad()
{
    return true
        && imageLoad(i_1d,0).x == ivec4(1).x
        && imageLoad(i_ib,0).x == ivec4(1).x
        && imageLoad(i_1da,ivec2(0)).x == ivec4(1).x
        && imageLoad(i_2d,ivec2(0)).x == ivec4(1).x
        && imageLoad(i_2dr,ivec2(0)).x == ivec4(1).x
        && imageLoad(i_2dms,ivec2(0),1).x == ivec4(1).x
        && imageLoad(i_2da,ivec3(0)).x == ivec4(1).x
        && imageLoad(i_3d,ivec3(0)).x == ivec4(1).x
        && imageLoad(i_c,ivec3(0)).x == ivec4(1).x
        && imageLoad(i_ca,ivec3(0)).x == ivec4(1).x
        && imageLoad(i_ca,ivec3(0)).x == ivec4(1).x
        && imageLoad(i_2dmsa,ivec3(0),1).x == ivec4(1).x
        ;
}
bool resetAllImageValues()
{
    imageStore(i_1d,0,ivec4(1));
    imageStore(i_ib,0,ivec4(1));
    imageStore(i_1da,ivec2(0),ivec4(1));
    imageStore(i_2d,ivec2(0),ivec4(1));
    imageStore(i_2dr,ivec2(0),ivec4(1));
    imageStore(i_2dms,ivec2(0), 1, ivec4(1));
    imageStore(i_2da,ivec3(0),ivec4(1));
    imageStore(i_3d,ivec3(0),ivec4(1));
    imageStore(i_c,ivec3(0),ivec4(1));
    imageStore(i_ca,ivec3(0),ivec4(1));
    imageStore(i_2dmsa,ivec3(0),1,ivec4(1));
    return true;
}
int load_1d()
{
    return imageLoad(i_1d,0).x;
}
int load_ib()
{
    return imageLoad(i_ib,0).x;
}
int load_1da()
{
    return imageLoad(i_1da,ivec2(0)).x;
}
int load_2d()
{
    return imageLoad(i_2d,ivec2(0)).x;
}
int load_2dr()
{
    return imageLoad(i_2dr,ivec2(0)).x;
}
int load_2dms()
{
    return imageLoad(i_2dms,ivec2(0),1).x;
}
int load_2da()
{
    return imageLoad(i_2da,ivec3(0)).x;
}
int load_3d()
{
    return imageLoad(i_3d,ivec3(0)).x; 
}
int load_c()
{
    return imageLoad(i_c,ivec3(0)).x;
}
int load_ca()
{
    return imageLoad(i_ca,ivec3(0)).x;     
}
int load_2dmsa()
{
    return imageLoad(i_2dmsa,ivec3(0),1).x;      
}
// requires ImageLoad test to pass 
bool checkAllImageStore()
{
    bool loadCheck = true;

    imageStore(i_1d,0,ivec4(0));
    loadCheck = loadCheck && load_1d() == 0;
    imageStore(i_ib,0,ivec4(0));
    loadCheck = loadCheck && load_ib() == 0;
    imageStore(i_1da,ivec2(0),ivec4(0));
    loadCheck = loadCheck && load_1da() == 0;
    imageStore(i_2d,ivec2(0),ivec4(0));
    loadCheck = loadCheck && load_2d() == 0;
    imageStore(i_2dr,ivec2(0),ivec4(0));
    loadCheck = loadCheck && load_2dr() == 0;
    imageStore(i_2dms,ivec2(0), 1, ivec4(0));
    loadCheck = loadCheck && load_2dms() == 0;
    imageStore(i_2da,ivec3(0),ivec4(0));
    loadCheck = loadCheck && load_2da() == 0;
    imageStore(i_3d,ivec3(0),ivec4(0));
    loadCheck = loadCheck && load_3d() == 0;
    imageStore(i_c,ivec3(0),ivec4(0));
    loadCheck = loadCheck && load_c() == 0;
    imageStore(i_ca,ivec3(0),ivec4(0));
    loadCheck = loadCheck && load_ca() == 0;
    imageStore(i_2dmsa,ivec3(0),1,ivec4(0));
    loadCheck = loadCheck && load_2dmsa() == 0;
    resetAllImageValues();
    return loadCheck;
        ;
}
bool checkAllImageSamples()
{
    resetAllImageValues();
    return true
        && imageSamples(i_2dms) == 2
        && imageSamples(i_2dmsa) == 2
        ;
}
bool checkAllImageAtomicAdd()
{
    resetAllImageValues();
    return true
        && imageAtomicAdd(i_1d,0,0) == 1
        && load_1d() == 1
        && imageAtomicAdd(i_ib,0,2) == 1
        && load_ib() == 3
        && imageAtomicAdd(i_1da,ivec2(0),0) == 1
        && load_1da() == 1
        && imageAtomicAdd(i_2d,ivec2(0),2) == 1
        && load_2d() == 3
        && imageAtomicAdd(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 3
        && imageAtomicAdd(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 3
        && imageAtomicAdd(i_2da,ivec3(0),0) == 1
        && load_2da() == 1
        && imageAtomicAdd(i_3d,ivec3(0),2) == 1
        && load_3d() == 3
        && imageAtomicAdd(i_c,ivec3(0),2) == 1
        && load_c() == 3
        && imageAtomicAdd(i_ca,ivec3(0),2) == 1
        && load_ca() == 3
        && imageAtomicAdd(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 3
        ;
}
bool checkAllImageAtomicExchange()
{
    resetAllImageValues();
    return true
        && imageAtomicExchange(i_1d,0,0) == 1
        && load_1d() == 0
        && imageAtomicExchange(i_ib,0,2) == 1
        && load_ib() == 2
        && imageAtomicExchange(i_1da,ivec2(0),0) == 1
        && load_1da() == 0
        && imageAtomicExchange(i_2d,ivec2(0),2) == 1
        && load_2d() == 2
        && imageAtomicExchange(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 2
        && imageAtomicExchange(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 2
        && imageAtomicExchange(i_2da,ivec3(0),0) == 1
        && load_2da() == 0
        && imageAtomicExchange(i_3d,ivec3(0),2) == 1
        && load_3d() == 2
        && imageAtomicExchange(i_c,ivec3(0),2) == 1
        && load_c() == 2
        && imageAtomicExchange(i_ca,ivec3(0),2) == 1
        && load_ca() == 2
        && imageAtomicExchange(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 2
        ;
}
bool checkAllImageAtomicMin()
{
    resetAllImageValues();
    return true
        && imageAtomicMin(i_1d,0,0) == 1
        && load_1d() == 0
        && imageAtomicMin(i_ib,0,2) == 1
        && load_ib() == 1
        && imageAtomicMin(i_1da,ivec2(0),0) == 1
        && load_1da() == 0
        && imageAtomicMin(i_2d,ivec2(0),2) == 1
        && load_2d() == 1
        && imageAtomicMin(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 1
        && imageAtomicMin(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 1
        && imageAtomicMin(i_2da,ivec3(0),0) == 1
        && load_2da() == 0
        && imageAtomicMin(i_3d,ivec3(0),2) == 1
        && load_3d() == 1
        && imageAtomicMin(i_c,ivec3(0),2) == 1
        && load_c() == 1
        && imageAtomicMin(i_ca,ivec3(0),2) == 1
        && load_ca() == 1
        && imageAtomicMin(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 1
        ;
}
bool checkAllImageAtomicMax()
{
    resetAllImageValues();
    return true
        && imageAtomicMax(i_1d,0,0) == 1
        && load_1d() == 1
        && imageAtomicMax(i_ib,0,2) == 1
        && load_ib() == 2
        && imageAtomicMax(i_1da,ivec2(0),0) == 1
        && load_1da() == 1
        && imageAtomicMax(i_2d,ivec2(0),2) == 1
        && load_2d() == 2
        && imageAtomicMax(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 2
        && imageAtomicMax(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 2
        && imageAtomicMax(i_2da,ivec3(0),0) == 1
        && load_2da() == 1
        && imageAtomicMax(i_3d,ivec3(0),2) == 1
        && load_3d() == 2
        && imageAtomicMax(i_c,ivec3(0),2) == 1
        && load_c() == 2
        && imageAtomicMax(i_ca,ivec3(0),2) == 1
        && load_ca() == 2
        && imageAtomicMax(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 2
        ;
}
bool checkAllImageAtomicAnd()
{
    resetAllImageValues();
    return true
        && imageAtomicAnd(i_1d,0,1) == 1
        && load_1d() == 1
        && imageAtomicAnd(i_ib,0,2) == 1
        && load_ib() == 0
        && imageAtomicAnd(i_1da,ivec2(0),1) == 1
        && load_1da() == 1
        && imageAtomicAnd(i_2d,ivec2(0),2) == 1
        && load_2d() == 0
        && imageAtomicAnd(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 0
        && imageAtomicAnd(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 0
        && imageAtomicAnd(i_2da,ivec3(0),1) == 1
        && load_2da() == 1
        && imageAtomicAnd(i_3d,ivec3(0),2) == 1
        && load_3d() == 0
        && imageAtomicAnd(i_c,ivec3(0),2) == 1
        && load_c() == 0
        && imageAtomicAnd(i_ca,ivec3(0),2) == 1
        && load_ca() == 0
        && imageAtomicAnd(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 0
        ;
}
bool checkAllImageAtomicOr()
{
    resetAllImageValues();
    return true
        && imageAtomicOr(i_1d,0,1) == 1
        && load_1d() == 1
        && imageAtomicOr(i_ib,0,2) == 1
        && load_ib() == 3
        && imageAtomicOr(i_1da,ivec2(0),1) == 1
        && load_1da() == 1
        && imageAtomicOr(i_2d,ivec2(0),2) == 1
        && load_2d() == 3
        && imageAtomicOr(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 3
        && imageAtomicOr(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 3
        && imageAtomicOr(i_2da,ivec3(0),1) == 1
        && load_2da() == 1
        && imageAtomicOr(i_3d,ivec3(0),2) == 1
        && load_3d() == 3
        && imageAtomicOr(i_c,ivec3(0),2) == 1
        && load_c() == 3
        && imageAtomicOr(i_ca,ivec3(0),2) == 1
        && load_ca() == 3
        && imageAtomicOr(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 3
        ;
}
bool checkAllImageAtomicXor()
{
    resetAllImageValues();
    return true
        && imageAtomicXor(i_1d,0,1) == 1
        && load_1d() == 0
        && imageAtomicXor(i_ib,0,2) == 1
        && load_ib() == 3
        && imageAtomicXor(i_1da,ivec2(0),1) == 1
        && load_1da() == 0
        && imageAtomicXor(i_2d,ivec2(0),2) == 1
        && load_2d() == 3
        && imageAtomicXor(i_2dr,ivec2(0),2) == 1
        && load_2dr() == 3
        && imageAtomicXor(i_2dms,ivec2(0),1,2) == 1
        && load_2dms() == 3
        && imageAtomicXor(i_2da,ivec3(0),1) == 1
        && load_2da() == 0
        && imageAtomicXor(i_3d,ivec3(0),2) == 1
        && load_3d() == 3
        && imageAtomicXor(i_c,ivec3(0),2) == 1
        && load_c() == 3
        && imageAtomicXor(i_ca,ivec3(0),2) == 1
        && load_ca() == 3
        && imageAtomicXor(i_2dmsa,ivec3(0),1,2) == 1
        && load_2dmsa() == 3
        ;
}
bool checkAllImageAtomicCompSwap()
{
    resetAllImageValues();
    return true
        && imageAtomicCompSwap(i_1d,0,0,2) == 1
        && load_1d() == 1
        && imageAtomicCompSwap(i_ib,0,1,2) == 1
        && load_ib() == 2
        && imageAtomicCompSwap(i_1da,ivec2(0),0,2) == 1
        && load_1da() == 1
        && imageAtomicCompSwap(i_2d,ivec2(0),1,2) == 1
        && load_2d() == 2
        && imageAtomicCompSwap(i_2dr,ivec2(0),1,2) == 1
        && load_2dr() == 2
        && imageAtomicCompSwap(i_2dms,ivec2(0),1,1,2) == 1
        && load_2dms() == 2
        && imageAtomicCompSwap(i_2da,ivec3(0),0,2) == 1
        && load_2da() == 1
        && imageAtomicCompSwap(i_3d,ivec3(0),1,2) == 1
        && load_3d() == 2
        && imageAtomicCompSwap(i_c,ivec3(0),1,2) == 1
        && load_c() == 2
        && imageAtomicCompSwap(i_ca,ivec3(0),1,2) == 1
        && load_ca() == 2
        && imageAtomicCompSwap(i_2dmsa,ivec3(0),1,1,2) == 1
        && load_2dmsa() == 2
        ;
}
bool imageOperations()
{
    return true
        && checkAllImageSize()
        && checkAllImageLoad()
        && checkAllImageStore()
        && checkAllImageSamples()
        && checkAllImageAtomicAdd()
        && checkAllImageAtomicExchange()
        && checkAllImageAtomicMin()
        && checkAllImageAtomicMax()
        && checkAllImageAtomicAnd()
        && checkAllImageAtomicOr()
        && checkAllImageAtomicXor()
        && checkAllImageAtomicCompSwap()
        ;
}

// CHECK_GLSL: void main(
// CHECK_SPV: OpEntryPoint
void computeMain()
{
    outputBuffer.data[0] = true
        && imageOperations()
        ;
    // BUF: 1
}
