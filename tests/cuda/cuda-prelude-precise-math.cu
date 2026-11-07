// Compiles the prelude's `SLANG_CUDA_ENABLE_PRECISE_MATH` bodies, which a `-target cuda` filecheck
// never does because the prelude reaches the emitted source only as an `#include` line.
//
// The slang-test lanes compile this file through NVRTC without `-fp-mode precise`, so NVRTC keeps
// its default `--fmad=true` and would fuse a plain `a * b + c` into `fma.rn`. A PTX `mul`, `add` or
// `sub` without a rounding modifier is also contractable later, by ptxas. Each operator kernel must
// therefore contain only the explicitly rounded `mul.rn`/`add.rn`/`sub.rn` forms, which come from
// the `_rn` intrinsics, and no `fma.rn`. The file also builds with offline nvcc
// (`nvcc -std=c++17 -arch=compute_80 -c <this file> -I prelude`), which takes the non-NVRTC branch
// of the prelude.

// CHECK verifies that each kernel produces its rounded arithmetic. NOFMA verifies that nothing
// before the explicit-fma control, which is the last kernel, is fused or unrounded.
//TEST:SIMPLE(filecheck=CHECK): -pass-through nvrtc -target ptx -entry vectorFloat3 -capability cuda_sm_9_0 -I prelude
//TEST:SIMPLE(filecheck=NOFMA): -pass-through nvrtc -target ptx -entry vectorFloat3 -capability cuda_sm_9_0 -I prelude

#define SLANG_CUDA_ENABLE_HALF 1
#define SLANG_CUDA_ENABLE_BF16 1
#define SLANG_CUDA_ENABLE_PRECISE_MATH 1
#include "slang-cuda-prelude.h"

extern "C" __global__ void vectorFloat2(float2* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void vectorFloat3(float3* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void vectorFloat4(float4* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void vectorDouble3(double3* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void vectorHalf3(__half3* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void vectorHalf2(__half2* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
    __half2 acc = p[8];
    acc *= p[9];
    acc += p[10];
    acc -= p[11];
    p[12] = acc;
}

extern "C" __global__ void matrixFloat(Matrix<float, 3, 3>* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void matrixDouble(Matrix<double, 2, 2>* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

extern "C" __global__ void matrixHalf(Matrix<__half, 2, 2>* p)
{
    p[3] = p[0] * p[1] + p[2];
    p[7] = p[4] - p[5] * p[6];
}

// Cooperative-matrix fragments, in the load / arithmetic / store shape Slang emits for `CoopMat`.
template<typename Frag, typename T>
__device__ void coopMatArithmetic(T* p)
{
    Frag a = Frag::template Load<Slang_CUDA_WMMA::RowMajor>(p, 0, 16);
    Frag b = Frag::template Load<Slang_CUDA_WMMA::RowMajor>(p, 256, 16);
    Frag c = Frag::template Load<Slang_CUDA_WMMA::RowMajor>(p, 512, 16);
    (a * b + c).template Store<Slang_CUDA_WMMA::RowMajor>(p, 768, 16);
    (c - a * b).template Store<Slang_CUDA_WMMA::RowMajor>(p, 1024, 16);
    (a * p[0] + c).template Store<Slang_CUDA_WMMA::RowMajor>(p, 1280, 16);
}

extern "C" __global__ void coopMatFloat(float* p)
{
    coopMatArithmetic<Slang_CUDA_WMMA::WmmaFragment<float, 16, 16, 16, Slang_CUDA_WMMA::MatrixC>>(p);
}

extern "C" __global__ void coopMatHalf(__half* p)
{
    coopMatArithmetic<Slang_CUDA_WMMA::WmmaFragment<__half, 16, 16, 16, Slang_CUDA_WMMA::MatrixC>>(p);
}

// bf16 arithmetic has unfused `mul`/`add`/`sub` PTX forms only from sm_90 on, hence the lanes'
// `cuda_sm_9_0`.
extern "C" __global__ void coopMatBFloat16(__nv_bfloat16* p)
{
    coopMatArithmetic<
        Slang_CUDA_WMMA::WmmaFragment<__nv_bfloat16, 16, 16, 16, Slang_CUDA_WMMA::MatrixA>>(p);
}

// An explicit fma() stays fused under the define; this kernel shows the lanes can see `fma.rn`.
extern "C" __global__ void explicitFma(float* p)
{
    p[3] = F32_fma(p[0], p[1], p[2]);
}

//CHECK-LABEL: .entry vectorFloat2(
//CHECK-DAG: mul.rn.f32
//CHECK-DAG: add.rn.f32
//CHECK-DAG: sub.rn.f32
//CHECK-LABEL: .entry vectorFloat3(
//CHECK-DAG: mul.rn.f32
//CHECK-DAG: add.rn.f32
//CHECK-DAG: sub.rn.f32
//CHECK-LABEL: .entry vectorFloat4(
//CHECK-DAG: mul.rn.f32
//CHECK-DAG: add.rn.f32
//CHECK-DAG: sub.rn.f32
//CHECK-LABEL: .entry vectorDouble3(
//CHECK-DAG: mul.rn.f64
//CHECK-DAG: add.rn.f64
//CHECK-DAG: sub.rn.f64
//CHECK-LABEL: .entry vectorHalf3(
//CHECK-DAG: mul.rn.f16
//CHECK-DAG: add.rn.f16
//CHECK-DAG: sub.rn.f16
//CHECK-LABEL: .entry vectorHalf2(
//CHECK-DAG: mul.rn.f16x2
//CHECK-DAG: add.rn.f16x2
//CHECK-DAG: sub.rn.f16x2
//CHECK-LABEL: .entry matrixFloat(
//CHECK-DAG: mul.rn.f32
//CHECK-DAG: add.rn.f32
//CHECK-DAG: sub.rn.f32
//CHECK-LABEL: .entry matrixDouble(
//CHECK-DAG: mul.rn.f64
//CHECK-DAG: add.rn.f64
//CHECK-DAG: sub.rn.f64
//CHECK-LABEL: .entry matrixHalf(
//CHECK-DAG: mul.rn.f16
//CHECK-DAG: add.rn.f16
//CHECK-DAG: sub.rn.f16
//CHECK-LABEL: .entry coopMatFloat(
//CHECK-DAG: mul.rn.f32
//CHECK-DAG: add.rn.f32
//CHECK-DAG: sub.rn.f32
//CHECK-LABEL: .entry coopMatHalf(
//CHECK-DAG: mul.rn.f16x2
//CHECK-DAG: add.rn.f16x2
//CHECK-DAG: sub.rn.f16x2
//CHECK-LABEL: .entry coopMatBFloat16(
//CHECK-DAG: mul.rn.bf16x2
//CHECK-DAG: add.rn.bf16x2
//CHECK-DAG: sub.rn.bf16x2
//CHECK-LABEL: .entry explicitFma(
//CHECK: fma.rn.f32

//NOFMA-NOT: {{fma.rn|(mul|add|sub)\.(f16|f32|f64|bf16)}}
//NOFMA: .entry explicitFma(
//NOFMA: fma.rn.f32
//NOFMA-NOT: {{(mul|add|sub)\.(f16|f32|f64|bf16)}}
