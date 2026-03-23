#include "/home/zhangkai/Documents/slangwork/slang-core-ecosys/another-slang/prelude/slang-cuda-prelude.h"

#ifndef SLANG_OUTER_PRODUCT_TAIL_IMPL
#define SLANG_OUTER_PRODUCT_TAIL_IMPL 0
#endif


__device__ __shared__ FixedArray<uint4 , 64>  data_0;


struct BindlessAddress_0
{
    RWStructuredBuffer<__half> handle_0;
    uint baseIndex_0;
};


__device__ BindlessAddress_0 BindlessAddress_x24init_0(RWStructuredBuffer<__half> handle_1)
{

    BindlessAddress_0 _S1;

    (&_S1)->handle_0 = handle_1;

    (&_S1)->baseIndex_0 = 0U;

    return _S1;
}


__device__ BindlessAddress_0 BindlessAddress_getOffset_0(BindlessAddress_0 * this_0, int elements_0)
{

    uint newBaseIndex_0 = this_0->baseIndex_0 + uint(elements_0);

    BindlessAddress_0 _S2 = BindlessAddress_x24init_0(this_0->handle_0);

    BindlessAddress_0 address_0 = _S2;

    (&address_0)->baseIndex_0 = newBaseIndex_0;

    return address_0;
}

using OuterProductTailFragment_0 = Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixC>;
using OuterProductTailFragments_0 = FixedArray<OuterProductTailFragment_0, 2>;
using OuterProductMatAFragment_0 = Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixA>;
using OuterProductMatBFragment_0 = Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixB>;
using OuterProductMatAFragments2_0 = FixedArray<OuterProductMatAFragment_0, 2>;
using OuterProductMatAFragments4_0 = FixedArray<OuterProductMatAFragment_0, 4>;
using OuterProductMatBFragments4_0 = FixedArray<OuterProductMatBFragment_0, 4>;

__device__ __forceinline__ uint _outerProductTailSharedBase_0()
{
    return (unsigned)__cvta_generic_to_shared(data_0.m_data);
}

__device__ __forceinline__ uint4 _outerProductTailFragmentToUint4_0(OuterProductTailFragment_0 fragment_0)
{
    return make_uint4(
        fragment_0.FragmentRead((int(0))),
        fragment_0.FragmentRead((int(1))),
        fragment_0.FragmentRead((int(2))),
        fragment_0.FragmentRead((int(3))));
}

__device__ __forceinline__ void _outerProductTailStoreShared_0(uint sharedBase_0, uint fragmentIndex_0, OuterProductTailFragment_0 fragment_0)
{
    uint4 packed_0 = _outerProductTailFragmentToUint4_0(fragment_0);
    const unsigned* packedPtr_0 = reinterpret_cast<const unsigned*>(&packed_0);
    asm("st.shared.v4.u32 [%0], {%1, %2, %3, %4};"
        :
        : "r"(sharedBase_0 + fragmentIndex_0 * 16U), "r"(packedPtr_0[0]), "r"(packedPtr_0[1]), "r"(packedPtr_0[2]), "r"(packedPtr_0[3]));
}

__device__ void _outerProductReduceSharedStore_0(uint slot_0, uint laneId_0, const OuterProductTailFragments_0& fragments_0)
{
    __half2* redMemH2_0 = reinterpret_cast<__half2*>(data_0.m_data);
    constexpr uint MMARows_0 = 2U;
    constexpr uint MMACols_0 = 1U;
    constexpr uint NumRegs_0 = OuterProductTailFragment_0::RegsCount;
    uint slotBase_0 = slot_0 * MMARows_0 * NumRegs_0 * 32U;

    #pragma unroll
    for(uint mmaCol_0 = 0; mmaCol_0 < MMACols_0; ++mmaCol_0)
    {
        #pragma unroll
        for(uint mmaRow_0 = 0; mmaRow_0 < MMARows_0; ++mmaRow_0)
        {
            uint fragmentIdx_0 = mmaCol_0 * MMARows_0 + mmaRow_0;
            const __half2* fragmentRegs_0 = reinterpret_cast<const __half2*>(fragments_0[fragmentIdx_0].regs);
            uint mma_idx_0 = slotBase_0 + fragmentIdx_0 * NumRegs_0 * 32U;

            #pragma unroll
            for(uint regIdx_0 = 0; regIdx_0 < NumRegs_0; ++regIdx_0)
            {
                uint offset_0 = mma_idx_0 + regIdx_0 * 32U + laneId_0;
                redMemH2_0[offset_0] = fragmentRegs_0[regIdx_0];
            }
        }
    }
}

__device__ void _outerProductReduceSharedLoad_0(uint slot_0, uint laneId_0, OuterProductTailFragments_0& fragments_0)
{
    __half2* redMemH2_0 = reinterpret_cast<__half2*>(data_0.m_data);
    constexpr uint MMARows_0 = 2U;
    constexpr uint MMACols_0 = 1U;
    constexpr uint NumRegs_0 = OuterProductTailFragment_0::RegsCount;
    uint slotBase_0 = slot_0 * MMARows_0 * NumRegs_0 * 32U;

    #pragma unroll
    for(uint mmaCol_0 = 0; mmaCol_0 < MMACols_0; ++mmaCol_0)
    {
        #pragma unroll
        for(uint mmaRow_0 = 0; mmaRow_0 < MMARows_0; ++mmaRow_0)
        {
            uint fragmentIdx_0 = mmaCol_0 * MMARows_0 + mmaRow_0;
            __half2* fragmentRegs_0 = reinterpret_cast<__half2*>(fragments_0[fragmentIdx_0].regs);
            uint mma_idx_0 = slotBase_0 + fragmentIdx_0 * NumRegs_0 * 32U;

            #pragma unroll
            for(uint regIdx_0 = 0; regIdx_0 < NumRegs_0; ++regIdx_0)
            {
                uint offset_0 = mma_idx_0 + regIdx_0 * 32U + laneId_0;
                fragmentRegs_0[regIdx_0] = redMemH2_0[offset_0];
            }
        }
    }
}

__device__ __forceinline__ __half2 _outerProductTailLoadSharedHalf2_0(uint sharedBase_0, int h2Idx_0)
{
    unsigned packed_0;
    asm("ld.shared.u32 %0, [%1];" : "=r"(packed_0) : "r"(sharedBase_0 + uint(h2Idx_0 * int(4))));
    return *reinterpret_cast<__half2*>(&packed_0);
}

__device__ __forceinline__ void _outerProductTailReduceSharedHalf2_0(
    uint sharedBase_0,
    int h2Idx_0,
    BindlessAddress_0 weightGradAddr_0)
{
    __half2 value_0 = _outerProductTailLoadSharedHalf2_0(sharedBase_0, h2Idx_0);
    __slang_atomic_reduce_add(
        reinterpret_cast<__half2 *>((weightGradAddr_0.handle_0).data + (weightGradAddr_0.baseIndex_0 + uint(h2Idx_0 * int(2)))),
        value_0,
        0);
}

__device__ __forceinline__ void _outerProductTailOriginal_0(
    uint warpId_0,
    OuterProductTailFragments_0& fragments_0,
    BindlessAddress_0 weightGradAddr_0)
{
    if(warpId_0 == 0U)
    {
        uint laneId_0 = (_getLaneId());
        uint sharedBase_0 = _outerProductTailSharedBase_0();
        _outerProductTailStoreShared_0(sharedBase_0, laneId_0, fragments_0[int(0)]);
        _outerProductTailStoreShared_0(sharedBase_0, laneId_0 + 32U, fragments_0[int(1)]);
        __syncwarp();

        int h2Idx_0 = int(laneId_0);
        if(h2Idx_0 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_0, weightGradAddr_0);

        int h2Idx_1 = h2Idx_0 + int(32);
        if(h2Idx_1 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_1, weightGradAddr_0);

        int h2Idx_2 = h2Idx_0 + int(64);
        if(h2Idx_2 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_2, weightGradAddr_0);

        int h2Idx_3 = h2Idx_0 + int(96);
        if(h2Idx_3 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_3, weightGradAddr_0);

        int h2Idx_4 = h2Idx_0 + int(128);
        if(h2Idx_4 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_4, weightGradAddr_0);

        int h2Idx_5 = h2Idx_0 + int(160);
        if(h2Idx_5 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_5, weightGradAddr_0);

        int h2Idx_6 = h2Idx_0 + int(192);
        if(h2Idx_6 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_6, weightGradAddr_0);

        int h2Idx_7 = h2Idx_0 + int(224);
        if(h2Idx_7 < int(256))
            _outerProductTailReduceSharedHalf2_0(sharedBase_0, h2Idx_7, weightGradAddr_0);
    }

    __syncthreads();
}

__device__ OuterProductTailFragments_0 _outerProductReduceWarpLocal_0(
    const OuterProductMatAFragments2_0& dOutTiles_0,
    const OuterProductMatAFragments4_0& inputTiles_0)
{
    OuterProductMatAFragments2_0 dOutTilesPrepared_0 = dOutTiles_0;
    OuterProductMatAFragments4_0 inputTilesPrepared_0 = inputTiles_0;

    Slang_CUDA_WMMA::mmaChangeMajor(*(&inputTilesPrepared_0[int(0)]));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&inputTilesPrepared_0[int(1)]));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&inputTilesPrepared_0[int(2)]));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&inputTilesPrepared_0[int(3)]));

    uint dOut0Reg1_0 = (((dOutTilesPrepared_0[int(0)])).FragmentRead((int(1))));
    uint dOut0Reg2_0 = (((dOutTilesPrepared_0[int(0)])).FragmentRead((int(2))));
    ((&dOutTilesPrepared_0[int(0)]))->FragmentWrite((int(1)), (dOut0Reg2_0));
    ((&dOutTilesPrepared_0[int(0)]))->FragmentWrite((int(2)), (dOut0Reg1_0));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&dOutTilesPrepared_0[int(0)]));

    uint dOut1Reg1_0 = (((dOutTilesPrepared_0[int(1)])).FragmentRead((int(1))));
    uint dOut1Reg2_0 = (((dOutTilesPrepared_0[int(1)])).FragmentRead((int(2))));
    ((&dOutTilesPrepared_0[int(1)]))->FragmentWrite((int(1)), (dOut1Reg2_0));
    ((&dOutTilesPrepared_0[int(1)]))->FragmentWrite((int(2)), (dOut1Reg1_0));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&dOutTilesPrepared_0[int(1)]));

    OuterProductMatBFragments4_0 inputB_0;
    inputB_0[int(0)] = reinterpret_cast<const OuterProductMatBFragment_0&>(inputTilesPrepared_0[int(0)]);
    inputB_0[int(1)] = reinterpret_cast<const OuterProductMatBFragment_0&>(inputTilesPrepared_0[int(1)]);
    inputB_0[int(2)] = reinterpret_cast<const OuterProductMatBFragment_0&>(inputTilesPrepared_0[int(2)]);
    inputB_0[int(3)] = reinterpret_cast<const OuterProductMatBFragment_0&>(inputTilesPrepared_0[int(3)]);

    OuterProductTailFragments_0 matC_1;
    ((&matC_1[int(0)]))->clear();
    ((&matC_1[int(1)]))->clear();

    OuterProductTailFragment_0 accum00_0 = (Slang_CUDA_WMMA::coopMatMulAdd<
                                               OuterProductMatAFragment_0::ElementType,
                                               OuterProductMatBFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductMatAFragment_0::m_M,
                                               OuterProductMatAFragment_0::m_N,
                                               OuterProductMatAFragment_0::m_K,
                                               false
                                              >((dOutTilesPrepared_0[int(0)]), (inputB_0[int(0)]), (matC_1[int(0)])));
    matC_1[int(0)] = accum00_0;

    OuterProductTailFragment_0 accum01_0 = (Slang_CUDA_WMMA::coopMatMulAdd<
                                               OuterProductMatAFragment_0::ElementType,
                                               OuterProductMatBFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductMatAFragment_0::m_M,
                                               OuterProductMatAFragment_0::m_N,
                                               OuterProductMatAFragment_0::m_K,
                                               false
                                              >((dOutTilesPrepared_0[int(0)]), (inputB_0[int(2)]), (matC_1[int(1)])));
    matC_1[int(1)] = accum01_0;

    OuterProductTailFragment_0 accum10_0 = (Slang_CUDA_WMMA::coopMatMulAdd<
                                               OuterProductMatAFragment_0::ElementType,
                                               OuterProductMatBFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductMatAFragment_0::m_M,
                                               OuterProductMatAFragment_0::m_N,
                                               OuterProductMatAFragment_0::m_K,
                                               false
                                              >((dOutTilesPrepared_0[int(1)]), (inputB_0[int(1)]), (matC_1[int(0)])));
    matC_1[int(0)] = accum10_0;

    OuterProductTailFragment_0 accum11_0 = (Slang_CUDA_WMMA::coopMatMulAdd<
                                               OuterProductMatAFragment_0::ElementType,
                                               OuterProductMatBFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductTailFragment_0::ElementType,
                                               OuterProductMatAFragment_0::m_M,
                                               OuterProductMatAFragment_0::m_N,
                                               OuterProductMatAFragment_0::m_K,
                                               false
                                              >((dOutTilesPrepared_0[int(1)]), (inputB_0[int(3)]), (matC_1[int(1)])));
    matC_1[int(1)] = accum11_0;

    return matC_1;
}

__device__ OuterProductMatAFragments4_0 _mmaTranspose_0(
    const OuterProductMatAFragments2_0& dOutTiles_0,
    const OuterProductMatAFragments4_0& inputTiles_0,
    BindlessAddress_0 weightAddr_0)
{
    OuterProductMatAFragments2_0 dOutTilesForMma_0 = dOutTiles_0;
    OuterProductMatAFragments4_0 inputTilesPrepared_0 = inputTiles_0;

    BindlessAddress_0 weightTile0Addr_0 = BindlessAddress_getOffset_0(&weightAddr_0, int(0));
    BindlessAddress_0 weightTile1Addr_0 = BindlessAddress_getOffset_0(&weightAddr_0, int(256));

    FixedArray<Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixB>, 2> wTiles_0;
    wTiles_0[int(0)] = Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixB>::LoadRaw(((weightTile0Addr_0.handle_0)).data + (weightTile0Addr_0.baseIndex_0));
    wTiles_0[int(1)] = Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixB>::LoadRaw(((weightTile1Addr_0.handle_0)).data + (weightTile1Addr_0.baseIndex_0));

    FixedArray<Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixA>, 2> wTilesA_0;
    ((&wTilesA_0[int(0)]))->copyFrom((wTiles_0[int(0)]));
    ((&wTilesA_0[int(1)]))->copyFrom((wTiles_0[int(1)]));

    uint wTile0Reg1_0 = (((wTilesA_0[int(0)])).FragmentRead((int(1))));
    uint wTile0Reg2_0 = (((wTilesA_0[int(0)])).FragmentRead((int(2))));
    ((&wTilesA_0[int(0)]))->FragmentWrite((int(1)), (wTile0Reg2_0));
    ((&wTilesA_0[int(0)]))->FragmentWrite((int(2)), (wTile0Reg1_0));

    uint wTile1Reg1_0 = (((wTilesA_0[int(1)])).FragmentRead((int(1))));
    uint wTile1Reg2_0 = (((wTilesA_0[int(1)])).FragmentRead((int(2))));
    ((&wTilesA_0[int(1)]))->FragmentWrite((int(1)), (wTile1Reg2_0));
    ((&wTilesA_0[int(1)]))->FragmentWrite((int(2)), (wTile1Reg1_0));

    FixedArray<Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixB>, 2> wtTilesB_0;
    ((&wtTilesB_0[int(0)]))->copyFrom((wTilesA_0[int(0)]));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&wtTilesB_0[int(0)]));
    ((&wtTilesB_0[int(1)]))->copyFrom((wTilesA_0[int(1)]));
    Slang_CUDA_WMMA::mmaChangeMajor(*(&wtTilesB_0[int(1)]));

    FixedArray<Slang_CUDA_WMMA::WmmaFragment<__half,16, 16, 16, Slang_CUDA_WMMA::MatrixC>, 4> matC_0;
    ((&matC_0[int(0)]))->clear();
    ((&matC_0[int(1)]))->clear();
    ((&matC_0[int(2)]))->clear();
    ((&matC_0[int(3)]))->clear();

    matC_0[int(0)] = (Slang_CUDA_WMMA::coopMatMulAdd<
                        OuterProductMatAFragment_0::ElementType,
                        OuterProductMatBFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductMatAFragment_0::m_M,
                        OuterProductMatAFragment_0::m_N,
                        OuterProductMatAFragment_0::m_K,
                        false
                       >((dOutTilesForMma_0[int(0)]), (wtTilesB_0[int(0)]), (matC_0[int(0)])));

    matC_0[int(1)] = (Slang_CUDA_WMMA::coopMatMulAdd<
                        OuterProductMatAFragment_0::ElementType,
                        OuterProductMatBFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductMatAFragment_0::m_M,
                        OuterProductMatAFragment_0::m_N,
                        OuterProductMatAFragment_0::m_K,
                        false
                       >((dOutTilesForMma_0[int(1)]), (wtTilesB_0[int(0)]), (matC_0[int(1)])));

    matC_0[int(2)] = (Slang_CUDA_WMMA::coopMatMulAdd<
                        OuterProductMatAFragment_0::ElementType,
                        OuterProductMatBFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductMatAFragment_0::m_M,
                        OuterProductMatAFragment_0::m_N,
                        OuterProductMatAFragment_0::m_K,
                        false
                       >((dOutTilesForMma_0[int(0)]), (wtTilesB_0[int(1)]), (matC_0[int(2)])));

    matC_0[int(3)] = (Slang_CUDA_WMMA::coopMatMulAdd<
                        OuterProductMatAFragment_0::ElementType,
                        OuterProductMatBFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductTailFragment_0::ElementType,
                        OuterProductMatAFragment_0::m_M,
                        OuterProductMatAFragment_0::m_N,
                        OuterProductMatAFragment_0::m_K,
                        false
                       >((dOutTilesForMma_0[int(1)]), (wtTilesB_0[int(1)]), (matC_0[int(3)])));

    OuterProductMatAFragments4_0 resultTiles_0;
    ((&resultTiles_0[int(0)]))->copyFrom((matC_0[int(0)]));
    ((&resultTiles_0[int(1)]))->copyFrom((matC_0[int(1)]));
    ((&resultTiles_0[int(2)]))->copyFrom((matC_0[int(2)]));
    ((&resultTiles_0[int(3)]))->copyFrom((matC_0[int(3)]));
    return resultTiles_0;
}

__device__ void _outerProductComputeReduce_0(
    uint warpId_0,
    const OuterProductMatAFragments2_0& dOutTiles_0,
    const OuterProductMatAFragments4_0& inputTiles_0,
    BindlessAddress_0 weightGradAddr_0)
{
    OuterProductTailFragments_0 wt_0 = _outerProductReduceWarpLocal_0(dOutTiles_0, inputTiles_0);
    uint laneId_0 = (_getLaneId());
    uint numWarps_0 = uint(blockDim.x / warpSize);

    for(uint j_0 = 2U; j_0 <= numWarps_0; j_0 <<= 1U)
    {
        if(warpId_0 % j_0 == j_0 / 2U)
            _outerProductReduceSharedStore_0(warpId_0 / j_0, laneId_0, wt_0);

        __syncthreads();

        if(warpId_0 % j_0 == 0U)
        {
            OuterProductTailFragments_0 fragments_1;
            _outerProductReduceSharedLoad_0(warpId_0 / j_0, laneId_0, fragments_1);
            wt_0[int(0)] = wt_0[int(0)] + fragments_1[int(0)];
            wt_0[int(1)] = wt_0[int(1)] + fragments_1[int(1)];
        }

        __syncthreads();
    }


    if(warpId_0 == 0U)
    {
        _outerProductReduceSharedStore_0(0U, laneId_0, wt_0);
        __syncwarp();

        __half2* dstH2_0 = reinterpret_cast<__half2*>((weightGradAddr_0.handle_0).data + weightGradAddr_0.baseIndex_0);
        __half2* redMemH2_0 = reinterpret_cast<__half2*>(data_0.m_data);

        constexpr uint MMARows_0 = 2U;
        constexpr uint MMACols_0 = 1U;
        constexpr uint NumRegs_0 = OuterProductTailFragment_0::RegsCount;

        #pragma unroll
        for(uint mmaCol_0 = 0; mmaCol_0 < MMACols_0; ++mmaCol_0)
        {
            #pragma unroll
            for(uint mmaRow_0 = 0; mmaRow_0 < MMARows_0; ++mmaRow_0)
            {
                uint mma_idx_0 = (mmaCol_0 * MMARows_0 + mmaRow_0) * NumRegs_0 * 32U;

                #pragma unroll
                for(uint regIdx_0 = 0; regIdx_0 < NumRegs_0; ++regIdx_0)
                {
                    uint offset_0 = mma_idx_0 + regIdx_0 * 32U + laneId_0;
                    __half2 reg_0 = redMemH2_0[offset_0];
                    __slang_atomic_reduce_add(dstH2_0 + offset_0, reg_0, 0);
                }
            }
        }
    }

    __syncthreads();
}


extern "C" __global__ void compute_backward(RWStructuredBuffer<__half> params_0, RWStructuredBuffer<uint> outputs_0, RWStructuredBuffer<__half> weightGrad_0, RWStructuredBuffer<__half> biasGrad_0, int batch_size_0)
{

    uint3  _S3 = ((threadIdx));

    uint tidx_0 = _S3.x;



    uint laneId_0 = tidx_0 % 32U;
    int sampleIdx_0 = int(((blockIdx)).x) * int(2) + int(tidx_0 / 32U);

    if(sampleIdx_0 >= batch_size_0)
    {

        return;
    }
    BindlessAddress_0 _S4 = BindlessAddress_x24init_0(params_0);

    BindlessAddress_0 _S5 = _S4;

    BindlessAddress_0 _S6 = BindlessAddress_getOffset_0(&_S5, int(0));
    BindlessAddress_0 _S7 = BindlessAddress_x24init_0(weightGrad_0);

    BindlessAddress_0 _S8 = _S7;

    BindlessAddress_0 _S9 = BindlessAddress_getOffset_0(&_S8, int(0));
    BindlessAddress_0 _S10 = BindlessAddress_x24init_0(biasGrad_0);

    FixedArray<uint, 8>  dOutPacked_0;


    dOutPacked_0[int(0)] = 1006648320U;

    dOutPacked_0[int(1)] = 1006648320U;

    dOutPacked_0[int(2)] = 1006648320U;

    dOutPacked_0[int(3)] = 1006648320U;

    dOutPacked_0[int(4)] = 1006648320U;

    dOutPacked_0[int(5)] = 1006648320U;

    dOutPacked_0[int(6)] = 1006648320U;

    dOutPacked_0[int(7)] = 1006648320U;

    FixedArray<uint, 16>  inputPacked_0;


    inputPacked_0[int(0)] = 1006648320U;

    inputPacked_0[int(1)] = 1006648320U;

    inputPacked_0[int(2)] = 1006648320U;

    inputPacked_0[int(3)] = 1006648320U;

    inputPacked_0[int(4)] = 1006648320U;

    inputPacked_0[int(5)] = 1006648320U;

    inputPacked_0[int(6)] = 1006648320U;

    inputPacked_0[int(7)] = 1006648320U;

    inputPacked_0[int(8)] = 1006648320U;

    inputPacked_0[int(9)] = 1006648320U;

    inputPacked_0[int(10)] = 1006648320U;

    inputPacked_0[int(11)] = 1006648320U;

    inputPacked_0[int(12)] = 1006648320U;

    inputPacked_0[int(13)] = 1006648320U;

    inputPacked_0[int(14)] = 1006648320U;

    inputPacked_0[int(15)] = 1006648320U;


    OuterProductMatAFragments2_0 dOutTiles_0;
    Slang_CUDA_WMMA::mmaBatchFromArray_MatA_16x16<16>(((&dOutTiles_0))->m_data, (dOutPacked_0));

    OuterProductMatAFragments4_0 inputTiles_0;
    Slang_CUDA_WMMA::mmaBatchFromArray_MatA_16x16<32>(((&inputTiles_0))->m_data, (inputPacked_0));

    OuterProductMatAFragments4_0 resultTiles_0 = _mmaTranspose_0(dOutTiles_0, inputTiles_0, _S6);

    OuterProductMatAFragments2_0 _S23 = dOutTiles_0;
    OuterProductMatAFragments4_0 _S24 = inputTiles_0;
    uint3  blockDim_0 = ((blockDim));
    uint _S27 = blockDim_0.x;
    uint flattenedTid_0 = tidx_0 + _S3.y * _S27 + _S3.z * _S27 * blockDim_0.y;
    uint _S29 = flattenedTid_0 / ((warpSize));

    _outerProductComputeReduce_0(_S29, _S23, _S24, _S9);

    uint _S92 = (((resultTiles_0[int(0)])).FragmentRead((int(0))));
    uint _S93 = (((resultTiles_0[int(0)])).FragmentRead((int(1))));

    uint checksum_0 = _S92 + _S93;
    uint _S94 = (((resultTiles_0[int(0)])).FragmentRead((int(2))));

    uint checksum_1 = checksum_0 + _S94;
    uint _S95 = (((resultTiles_0[int(0)])).FragmentRead((int(3))));

    uint checksum_2 = checksum_1 + _S95;

    uint _S96 = (((resultTiles_0[int(1)])).FragmentRead((int(0))));

    uint checksum_3 = checksum_2 + _S96;
    uint _S97 = (((resultTiles_0[int(1)])).FragmentRead((int(1))));

    uint checksum_4 = checksum_3 + _S97;
    uint _S98 = (((resultTiles_0[int(1)])).FragmentRead((int(2))));

    uint checksum_5 = checksum_4 + _S98;
    uint _S99 = (((resultTiles_0[int(1)])).FragmentRead((int(3))));

    uint checksum_6 = checksum_5 + _S99;

    uint _S100 = (((resultTiles_0[int(2)])).FragmentRead((int(0))));

    uint checksum_7 = checksum_6 + _S100;
    uint _S101 = (((resultTiles_0[int(2)])).FragmentRead((int(1))));

    uint checksum_8 = checksum_7 + _S101;
    uint _S102 = (((resultTiles_0[int(2)])).FragmentRead((int(2))));

    uint checksum_9 = checksum_8 + _S102;
    uint _S103 = (((resultTiles_0[int(2)])).FragmentRead((int(3))));

    uint checksum_10 = checksum_9 + _S103;

    uint _S104 = (((resultTiles_0[int(3)])).FragmentRead((int(0))));

    uint checksum_11 = checksum_10 + _S104;
    uint _S105 = (((resultTiles_0[int(3)])).FragmentRead((int(1))));

    uint checksum_12 = checksum_11 + _S105;
    uint _S106 = (((resultTiles_0[int(3)])).FragmentRead((int(2))));

    uint checksum_13 = checksum_12 + _S106;
    uint _S107 = (((resultTiles_0[int(3)])).FragmentRead((int(3))));

    *(&(outputs_0)[sampleIdx_0 * int(32) + int(laneId_0)]) = checksum_13 + _S107;
    return;
}


#include <cuda_runtime.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <cstring>

static constexpr int INPUT_SIZE_BENCH = 32;
static constexpr int OUTPUT_SIZE_BENCH = 16;
static constexpr int SUBGROUP_COUNT_BENCH = 2;
static constexpr int THREADS_PER_BLOCK_BENCH = 64;
static constexpr int TILED_WEIGHT_COUNT_BENCH = ((OUTPUT_SIZE_BENCH + 15) / 16) * ((INPUT_SIZE_BENCH + 15) / 16) * 256;
static int g_batch_size_generated = 256;
static constexpr int WARMUP_ITERATIONS_GENERATED = 100;
static constexpr int BENCHMARK_ITERATIONS_GENERATED = 1000;

static void linear_to_tiled_generated(const __half* weights_linear, __half* tiled, int output_size, int input_size)
{
    int tile_size = 16;
    int n_tile_rows = (output_size + tile_size - 1) / tile_size;
    int n_tile_cols = (input_size + tile_size - 1) / tile_size;
    int padded_m = n_tile_rows * tile_size;
    int padded_k = n_tile_cols * tile_size;

    std::vector<__half> w_padded(padded_m * padded_k, __float2half(0.0f));
    for (int r = 0; r < output_size; r++)
        for (int c = 0; c < input_size; c++)
            w_padded[r * padded_k + c] = weights_linear[r * input_size + c];

    for (int tr = 0; tr < n_tile_rows; tr++)
    {
        for (int tc = 0; tc < n_tile_cols; tc++)
        {
            int tile_index = tr * n_tile_cols + tc;
            for (int r = 0; r < tile_size; r++)
                for (int c = 0; c < tile_size; c++)
                    tiled[tile_index * 256 + r * tile_size + c] =
                        w_padded[(tr * tile_size + r) * padded_k + tc * tile_size + c];
        }
    }
}

static double run_generated_benchmark(int batch_size, int warmup, int iterations)
{
    size_t weights_linear_size = INPUT_SIZE_BENCH * OUTPUT_SIZE_BENCH * sizeof(__half);
    size_t weights_tiled_size = TILED_WEIGHT_COUNT_BENCH * sizeof(__half);
    size_t outputs_size = batch_size * 32 * sizeof(uint);
    size_t bias_grad_size = OUTPUT_SIZE_BENCH * sizeof(__half);

    __half *d_weights_tiled = nullptr;
    uint *d_outputs = nullptr;
    __half *d_weight_grad = nullptr;
    __half *d_bias_grad = nullptr;
    void *d_pad = nullptr;

    cudaMalloc(&d_weights_tiled, weights_tiled_size);
    cudaMalloc(&d_pad, 512);
    cudaMalloc(&d_outputs, outputs_size);
    cudaMalloc(&d_weight_grad, weights_tiled_size);
    cudaMalloc(&d_bias_grad, bias_grad_size);

    std::vector<__half> h_weights_linear(INPUT_SIZE_BENCH * OUTPUT_SIZE_BENCH);
    std::vector<__half> h_weights_tiled(TILED_WEIGHT_COUNT_BENCH, __float2half(0.0f));
    srand(42);
    for (auto& v : h_weights_linear)
        v = __float2half((rand() / (float)RAND_MAX - 0.5f) * 0.1f);
    linear_to_tiled_generated(h_weights_linear.data(), h_weights_tiled.data(), OUTPUT_SIZE_BENCH, INPUT_SIZE_BENCH);

    cudaMemcpy(d_weights_tiled, h_weights_tiled.data(), weights_tiled_size, cudaMemcpyHostToDevice);
    cudaMemset(d_outputs, 0, outputs_size);
    cudaMemset(d_weight_grad, 0, weights_tiled_size);
    cudaMemset(d_bias_grad, 0, bias_grad_size);

    RWStructuredBuffer<__half> params_handle{};
    params_handle.data = d_weights_tiled;
    params_handle.count = TILED_WEIGHT_COUNT_BENCH;

    RWStructuredBuffer<uint> outputs_handle{};
    outputs_handle.data = d_outputs;
    outputs_handle.count = batch_size * 32;

    RWStructuredBuffer<__half> weight_grad_handle{};
    weight_grad_handle.data = d_weight_grad;
    weight_grad_handle.count = TILED_WEIGHT_COUNT_BENCH;

    RWStructuredBuffer<__half> bias_grad_handle{};
    bias_grad_handle.data = d_bias_grad;
    bias_grad_handle.count = OUTPUT_SIZE_BENCH;

    dim3 grid(batch_size / SUBGROUP_COUNT_BENCH);
    dim3 block(THREADS_PER_BLOCK_BENCH);

    for (int i = 0; i < warmup; ++i)
    {
        compute_backward<<<grid, block>>>(params_handle, outputs_handle, weight_grad_handle, bias_grad_handle, batch_size);
    }
    cudaDeviceSynchronize();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess)
    {
        std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_weights_tiled);
        cudaFree(d_pad);
        cudaFree(d_outputs);
        cudaFree(d_weight_grad);
        cudaFree(d_bias_grad);
        return -1.0;
    }

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    for (int i = 0; i < iterations; ++i)
    {
        compute_backward<<<grid, block>>>(params_handle, outputs_handle, weight_grad_handle, bias_grad_handle, batch_size);
    }
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float total_ms = 0.0f;
    cudaEventElapsedTime(&total_ms, start, stop);
    double avg_ms = total_ms / iterations;

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(d_weights_tiled);
    cudaFree(d_pad);
    cudaFree(d_outputs);
    cudaFree(d_weight_grad);
    cudaFree(d_bias_grad);

    return avg_ms;
}

static void print_usage_generated(const char* prog)
{
    std::cout << "Usage: " << prog << " [--batch-size N]" << std::endl;
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if ((std::strcmp(argv[i], "--batch-size") == 0) && i + 1 < argc)
            g_batch_size_generated = std::atoi(argv[++i]);
        else if ((std::strcmp(argv[i], "--help") == 0) || (std::strcmp(argv[i], "-h") == 0))
        {
            print_usage_generated(argv[0]);
            return 0;
        }
    }

    std::cout << "======================================================================" << std::endl;
    std::cout << "Generated CUDA Benchmark (Slang transpose_outer tiny)" << std::endl;
    std::cout << "======================================================================" << std::endl;

    int device = 0;
    cudaGetDevice(&device);
    cudaDeviceProp prop{};
    cudaGetDeviceProperties(&prop, device);
    std::cout << "Device: " << prop.name << std::endl;
    std::cout << "Mode: transpose_outer" << std::endl;
    std::cout << "Network: " << INPUT_SIZE_BENCH << " -> " << OUTPUT_SIZE_BENCH << std::endl;
    std::cout << "Batch size: " << g_batch_size_generated << std::endl;
    std::cout << "Warps per block: 2" << std::endl;
    std::cout << "Warmup: " << WARMUP_ITERATIONS_GENERATED << ", Iterations: " << BENCHMARK_ITERATIONS_GENERATED << std::endl;

    double time_ms = run_generated_benchmark(g_batch_size_generated, WARMUP_ITERATIONS_GENERATED, BENCHMARK_ITERATIONS_GENERATED);
    if (time_ms < 0.0)
        return 1;

    double throughput = g_batch_size_generated / (time_ms / 1000.0);
    std::cout << "\nResults:" << std::endl;
    std::cout << "  Avg time: " << std::fixed << std::setprecision(4) << time_ms << " ms" << std::endl;
    std::cout << "  Throughput: " << std::setprecision(0) << throughput << " samples/s" << std::endl;
    return 0;
}
