//TEST:SIMPLE(filecheck=CUDA): -O0 -target cuda -validate-ir -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -O0

// A CUDA kernel can be launched by external code or called directly by another function.
// `fixEntryPointCallsites` preserves the launchable kernel and redirects ordinary calls to a device
// clone. Resource-global legalization gives each launch its own initialized resource value and
// passes the caller's value to the ordinary clone.

//TEST_INPUT: Texture2D(size=4, content = zero):name=firstTexture
Texture2D<float4> firstTexture;

//TEST_INPUT: Texture2D(size=4, content = one):name=secondTexture
Texture2D<float4> secondTexture;

// The output begins with twos. We expect the ones loaded from `secondTexture`, which proves that
// the device clone receives the caller's current value instead of repeating the initializer.
//TEST_INPUT: ubuffer(data=[2 2 2 2], stride=16):out,name=output
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture = firstTexture;

[CudaKernel]
[noinline]
void readCachedTexture()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    cachedTexture = secondTexture;
    readCachedTexture();
}

// CUDA-LABEL: static __device__ __noinline__ void readCachedTexture_{{[0-9]+}}(CUtexObject cachedTexture_{{[0-9]+}})
// CUDA: cachedTexture_{{[0-9]+}}
// CUDA-LABEL: __global__ void readCachedTexture()
// CUDA-LABEL: extern "C" __global__ void computeMain()
// CUDA: readCachedTexture_{{[0-9]+}}(globalParams_{{[0-9]+}}->secondTexture_{{[0-9]+}});

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
