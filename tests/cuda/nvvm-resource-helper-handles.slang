// All admitted handle shapes use the same local reference and aggregate contract.
// Run separately because combined generic specializations currently expose duplicate symbols.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=1
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=2
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=2
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=4
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=4
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=5
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=5
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=6
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=6
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=7
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=7
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=8
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=8
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=9
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=9
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=10
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=10
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=11
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=11
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=12
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=12
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=13
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=13
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=14
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=14
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=15
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=15
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=16
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=16
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=17
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=17
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=18
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=18
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DFAMILY=19
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DFAMILY=19
#if FAMILY == 0
typedef Texture1D<float4> Handle;
#elif FAMILY == 1
typedef Texture2D<float4> Handle;
#elif FAMILY == 2
typedef Texture3D<float4> Handle;
#elif FAMILY == 3
typedef TextureCube<float4> Handle;
#elif FAMILY == 4
typedef Texture1DArray<float4> Handle;
#elif FAMILY == 5
typedef Texture2DArray<float4> Handle;
#elif FAMILY == 6
typedef TextureCubeArray<float4> Handle;
#elif FAMILY == 7
typedef RWTexture1D<uint4> Handle;
#elif FAMILY == 8
typedef RWTexture2D<uint4> Handle;
#elif FAMILY == 9
typedef RWTexture3D<uint4> Handle;
#elif FAMILY == 10
typedef RWTexture1DArray<uint4> Handle;
#elif FAMILY == 11
typedef RWTexture2DArray<uint4> Handle;
#elif FAMILY == 12
typedef SamplerState Handle;
#elif FAMILY == 13
typedef Sampler1D<float4> Handle;
#elif FAMILY == 14
typedef Sampler2D<float4> Handle;
#elif FAMILY == 15
typedef Sampler3D<float4> Handle;
#elif FAMILY == 16
typedef SamplerCube<float4> Handle;
#elif FAMILY == 17
typedef Sampler1DArray<float4> Handle;
#elif FAMILY == 18
typedef Sampler2DArray<float4> Handle;
#elif FAMILY == 19
typedef SamplerCubeArray<float4> Handle;
#endif
//TEST_INPUT:ubuffer(data=[17 29 41 53 1], stride=4):name=input
StructuredBuffer<uint> input;
//TEST_INPUT:ubuffer(data=[199 0 0 0 0 199], stride=4):out,name=output
RWStructuredBuffer<uint> output;

struct Handles<T> { T pair[2]; }
struct Outer<T> { Handles<T> inner; }
[noinline] T read<T>(__constref T value) { return value; }
[noinline] T readForward<T>(__constref T value) { return read(value); }
[noinline] void initialize<T>(out T value, T source) { value = source; }
[noinline] void replace<T>(inout T value, T source) { initialize(value, source); }
[noinline] Outer<T> copy<T>(Outer<T> value) { return value; }
[noinline] uint2 before<T>(__constref Outer<T> value, uint index)
{ return reinterpret<uint2>(readForward(value.inner.pair[index])); }
[noinline] void update<T>(inout Outer<T> value, uint index, T source)
{ replace(value.inner.pair[index], source); }
[noinline] uint4 transport<T>(uint2 first, uint2 second, uint index)
{
    Outer<T> value;
    initialize(value.inner.pair[0], reinterpret<T>(first));
    initialize(value.inner.pair[1], reinterpret<T>(second));
    value = copy(value);
    uint2 original = before(value, index);
    update(value, index, reinterpret<T>(first));
    return uint4(original, before(value, index));
}
[numthreads(1, 1, 1)]
void computeMain()
{
    uint2 a = uint2(input[0], input[1]);
    uint2 b = uint2(input[2], input[3]);
    uint index = input[4];
    uint4 value = transport<Handle>(a, b, index);
    for (uint lane = 0; lane < 4; ++lane) output[1 + lane] = value[lane];
}
// CHECK: 199
// CHECK: 41
// CHECK: 53
// CHECK: 17
// CHECK: 29
// CHECK: 199
