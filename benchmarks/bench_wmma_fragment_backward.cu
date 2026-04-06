/**
 * WmmaFragment-backed Single Layer Backward Benchmark
 *
 * Full port of Tin2's backward benchmark using WmmaFragment (from slang-cuda-prelude.h)
 * as the core MMA tile type. All Tin2 operations (MatrixBase, Vector, Reducers, etc.)
 * are re-implemented on top of WmmaFragment.
 */

#include <cuda_runtime.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cuda_fp16.h>
#include <vector>
#include <cstring>
#include <algorithm>

// ============================================================================
// PART 1: Constants
// ============================================================================

static constexpr uint32_t WARP_SZ = 32;

enum class ReduceMode { STORE, ATOMIC_ADD };

// ============================================================================
// PART 2: PTX intrinsics (ported from tin_ptx.h)
// ============================================================================

__forceinline__ __device__ unsigned ptx_lane_id() {
    unsigned r; asm volatile("mov.u32 %0, %laneid;" : "=r"(r)); return r;
}

__forceinline__ __device__ unsigned ptx_warp_id() {
    return (threadIdx.z * blockDim.y * blockDim.x + threadIdx.y * blockDim.x + threadIdx.x) / warpSize;
}

__forceinline__ __device__ uint32_t ptx_movmatrix_trans(uint32_t in) {
    uint32_t r; asm("movmatrix.sync.trans.aligned.m8n8.b16 %0, %1;" : "=r"(r) : "r"(in)); return r;
}

__forceinline__ __device__ void ptx_red_add_h2(half2* addr, half2 in) {
    int v = *((int*)&in);
    asm("red.relaxed.gpu.global.add.noftz.f16x2 [%0], %1;" :: "l"(addr), "r"(v));
}

__forceinline__ __device__ uint2 ptx_hmma_m16n8k16(uint4& a, uint2& b, uint2& c) {
    uint2 D;
    unsigned const* A = reinterpret_cast<unsigned const*>(&a);
    unsigned const* B = reinterpret_cast<unsigned const*>(&b);
    unsigned const* C = reinterpret_cast<unsigned const*>(&c);
    asm volatile(
        "mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 {%0,%1},{%2,%3,%4,%5},{%6,%7},{%8,%9};"
        : "=r"(D.x), "=r"(D.y)
        : "r"(A[0]),"r"(A[1]),"r"(A[2]),"r"(A[3]),
          "r"(B[0]),"r"(B[1]),
          "r"(C[0]),"r"(C[1]));
    return D;
}

// ============================================================================
// PART 3: WmmaFragment (from slang-cuda-prelude.h)
// ============================================================================

enum MatrixUse : int { MU_A = 0, MU_B = 1 };

template<typename T, int M, int N, int K, MatrixUse R>
struct WmmaRegCount;
template<int M, int N, int K> struct WmmaRegCount<half,M,N,K,MU_A> { static constexpr int value = 4; };
template<int M, int N, int K> struct WmmaRegCount<half,M,N,K,MU_B> { static constexpr int value = 4; };

template<typename T, int M, int N, int K, MatrixUse R>
struct alignas(16) WmmaFragment
{
    void __device__ fill(T value) {
        unsigned packed = __half_as_ushort(value) | ((unsigned)__half_as_ushort(value) << 16);
#pragma unroll
        for (int i = 0; i < RegsCount; i++) regs[i] = packed;
    }

    void __device__ clear() {
#pragma unroll
        for (int i = 0; i < RegsCount; i++) regs[i] = 0U;
    }

    static constexpr int RegsCount = WmmaRegCount<T,M,N,K,R>::value;
    unsigned regs[RegsCount];
};

using FragA = WmmaFragment<half,16,16,16,MU_A>;
using FragB = WmmaFragment<half,16,16,16,MU_B>;

// ============================================================================
// PART 4: Fragment register layout constants
//
// Each WmmaFragment holds a 16x16 half tile via 2x2 = 4 registers.
// Sub-register dimensions depend on ROW_MAJOR:
//   ROW_MAJOR(A): SubRowsPacked=8, SubColsPacked=4
//   COL_MAJOR(B): SubRowsPacked=4, SubColsPacked=8
// ============================================================================

template<bool ROW_MAJOR> struct FragLayout {
    static constexpr uint32_t RegRows = 2, RegCols = 2, NumRegs = 4;
    static constexpr uint32_t NumOuter = 8;
    static constexpr uint32_t InnerPacked = WARP_SZ / NumOuter;
    static constexpr uint32_t SubRowsPacked = ROW_MAJOR ? NumOuter : InnerPacked;
    static constexpr uint32_t SubColsPacked = ROW_MAJOR ? InnerPacked : NumOuter;
    static constexpr uint32_t NumInner = InnerPacked * 2;
    static constexpr uint32_t SubRows = ROW_MAJOR ? NumOuter : NumInner;
    static constexpr uint32_t SubCols = ROW_MAJOR ? NumInner : NumOuter;
    static constexpr uint32_t TileRows = RegRows * SubRows;
    static constexpr uint32_t TileCols = RegCols * SubCols;
    static constexpr uint32_t TileRowsPacked = RegRows * SubRowsPacked;
    static constexpr uint32_t TileColsPacked = RegCols * SubColsPacked;

    static __device__ __host__ uint32_t reg_idx(uint32_t r, uint32_t c) { return c * RegRows + r; }
    static __device__ __host__ uint2 lane_to_sub_rc(uint32_t lid) {
        uint2 rc;
        if constexpr (ROW_MAJOR) { rc.x = lid / SubColsPacked; rc.y = lid % SubColsPacked; }
        else { rc.y = lid / SubRowsPacked; rc.x = lid % SubRowsPacked; }
        return rc;
    }
    static __device__ __host__ uint32_t get_reg_row(uint32_t rp) { return rp / SubRowsPacked; }
    static __device__ __host__ uint32_t get_reg_col(uint32_t cp) { return cp / SubColsPacked; }
    static __device__ __host__ uint32_t get_sub_row(uint32_t rp) { return rp % SubRowsPacked; }
    static __device__ __host__ uint32_t get_sub_col(uint32_t cp) { return cp % SubColsPacked; }
};

// ============================================================================
// PART 5: Fragment-level operations
// ============================================================================

template<MatrixUse U>
__device__ WmmaFragment<half,16,16,16, U==MU_A ? MU_B : MU_A>
frag_transpose(const WmmaFragment<half,16,16,16,U>& x) {
    constexpr MatrixUse DU = (U==MU_A) ? MU_B : MU_A;
    WmmaFragment<half,16,16,16,DU> r;
    r.regs[0] = x.regs[0];
    r.regs[1] = x.regs[2];
    r.regs[2] = x.regs[1];
    r.regs[3] = x.regs[3];
    return r;
}

template<MatrixUse U>
__device__ WmmaFragment<half,16,16,16, U==MU_A ? MU_B : MU_A>
frag_change_major(const WmmaFragment<half,16,16,16,U>& x) {
    constexpr MatrixUse DU = (U==MU_A) ? MU_B : MU_A;
    WmmaFragment<half,16,16,16,DU> r;
#pragma unroll
    for (int i = 0; i < 4; i++) r.regs[i] = ptx_movmatrix_trans(x.regs[i]);
    return r;
}

__device__ FragA frag_mad(const FragA& a, const FragB& b, const FragA& c) {
    uint4 ar = {a.regs[0], a.regs[1], a.regs[2], a.regs[3]};
    uint2 b0 = {b.regs[0], b.regs[1]}, c0 = {c.regs[0], c.regs[1]};
    uint2 b1 = {b.regs[2], b.regs[3]}, c1 = {c.regs[2], c.regs[3]};
    uint2 d0 = ptx_hmma_m16n8k16(ar, b0, c0);
    uint2 d1 = ptx_hmma_m16n8k16(ar, b1, c1);
    FragA r; r.regs[0]=d0.x; r.regs[1]=d0.y; r.regs[2]=d1.x; r.regs[3]=d1.y;
    return r;
}

template<MatrixUse U>
__device__ WmmaFragment<half,16,16,16,U>
frag_add(const WmmaFragment<half,16,16,16,U>& a, const WmmaFragment<half,16,16,16,U>& b) {
    WmmaFragment<half,16,16,16,U> r;
#pragma unroll
    for (int i = 0; i < 4; i++)
        *((half2*)&r.regs[i]) = __hadd2(*((const half2*)&a.regs[i]), *((const half2*)&b.regs[i]));
    return r;
}

__device__ void frag_reduce_row_sum(const FragA& x, half* dest) {
    using FL = FragLayout<true>;
    auto* dest_v = (half2*)dest;
    unsigned lid = ptx_lane_id();
    uint2 src = FL::lane_to_sub_rc(lid);
    uint32_t r[4]; for (int i=0;i<4;i++) r[i]=x.regs[i];

#pragma unroll
    for (uint32_t rc = 0; rc < FL::RegCols; rc++) {
        uint32_t si = FL::reg_idx(1, rc), di = FL::reg_idx(0, rc);
        *((half2*)&r[di]) = __hadd2(*((half2*)&r[di]), *((half2*)&r[si]));

        half2& reg = *((half2*)&r[di]);
#pragma unroll
        for (uint32_t j = 1; j < FL::SubRowsPacked; j <<= 1) {
            half2 s = __shfl_down_sync(0xFFFFFFFF, reg, FL::SubColsPacked * j);
            reg = __hadd2(reg, s);
        }
        if (src.x == 0) dest_v[rc * FL::SubColsPacked + src.y] = reg;
    }
}

// ============================================================================
// PART 6: TiledMatrix — array of WmmaFragment tiles
// ============================================================================

template<uint32_t N_ROWS, uint32_t N_COLS, bool ROW_MAJOR>
class TiledMatrix {
public:
    using FL = FragLayout<ROW_MAJOR>;
    static constexpr MatrixUse FU = ROW_MAJOR ? MU_A : MU_B;
    using Frag = WmmaFragment<half,16,16,16,FU>;

    static constexpr uint32_t MRows = N_ROWS / FL::TileRows;
    static constexpr uint32_t MCols = N_COLS / FL::TileCols;
    static constexpr uint32_t NumTiles = MRows * MCols;
    static constexpr uint32_t ColsPacked = MCols * FL::TileColsPacked;
    static constexpr uint32_t RowsPacked = MRows * FL::TileRowsPacked;

    Frag m_tiles[NumTiles];

    static __device__ __host__ uint32_t tidx(uint32_t r, uint32_t c) { return c * MRows + r; }
    static __device__ __host__ uint32_t get_tile_row(uint32_t rp) { return rp / FL::TileRowsPacked; }
    static __device__ __host__ uint32_t get_tile_col(uint32_t cp) { return cp / FL::TileColsPacked; }
    static __device__ __host__ uint32_t get_subtile_row(uint32_t rp) { return rp % FL::TileRowsPacked; }
    static __device__ __host__ uint32_t get_subtile_col(uint32_t cp) { return cp % FL::TileColsPacked; }

    __device__ void clear() {
#pragma unroll
        for (uint32_t i = 0; i < NumTiles; i++) m_tiles[i].clear();
    }

    __device__ void load_native(const half* addr) {
        unsigned lid = ptx_lane_id();
        auto src = (const Frag*)addr;
#pragma unroll
        for (uint32_t i = 0; i < NumTiles; i++)
            m_tiles[i] = src[i * WARP_SZ + lid];
    }

    __device__ void store_native(half* addr) const {
        unsigned lid = ptx_lane_id();
        auto dst = (Frag*)addr;
#pragma unroll
        for (uint32_t i = 0; i < NumTiles; i++)
            dst[i * WARP_SZ + lid] = m_tiles[i];
    }

    __device__ TiledMatrix<N_COLS, N_ROWS, !ROW_MAJOR> transpose() const {
        TiledMatrix<N_COLS, N_ROWS, !ROW_MAJOR> r;
#pragma unroll
        for (uint32_t i = 0; i < MCols; i++)
#pragma unroll
            for (uint32_t j = 0; j < MRows; j++) {
                r.m_tiles[r.tidx(i, j)] = frag_transpose(m_tiles[tidx(j, i)]);
            }
        return r;
    }
};

template<uint32_t R, uint32_t C, bool RM>
__device__ TiledMatrix<R,C,RM>
operator+(const TiledMatrix<R,C,RM>& a, const TiledMatrix<R,C,RM>& b) {
    TiledMatrix<R,C,RM> r;
#pragma unroll
    for (uint32_t i = 0; i < r.NumTiles; i++)
        r.m_tiles[i] = frag_add(a.m_tiles[i], b.m_tiles[i]);
    return r;
}

template<uint32_t R, uint32_t C> using HMatrixA = TiledMatrix<R, C, true>;
template<uint32_t R, uint32_t C> using HMatrixB = TiledMatrix<R, C, false>;

// ============================================================================
// PART 7: HVector — extends HMatrixA<WARP_SZ, N> with from_array / to_array
// ============================================================================

template<uint32_t N_COLS>
class HVector : public HMatrixA<WARP_SZ, N_COLS> {
    using Base = HMatrixA<WARP_SZ, N_COLS>;
    using FL = FragLayout<true>;
public:
    __device__ HVector() {}
    __device__ HVector(const Base& m) { *((Base*)this) = m; }

    template<uint32_t N>
    __device__ void from_array(const half* input) {
        auto ip = (const uint32_t*)input;
        const uint32_t lid = ptx_lane_id();

#pragma unroll
        for (uint32_t cp = 0; cp < Base::ColsPacked; cp += FL::SubColsPacked) {
            uint32_t rs[FL::SubColsPacked];
#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                rs[i] = (cp + i < N / 2) ? ip[cp + i] : 0U;
#pragma unroll
                for (uint32_t j = 1; j < FL::SubColsPacked; j++) {
                    uint32_t val = ip[cp + (i + j) % FL::SubColsPacked];
                    if (lid / FL::SubRowsPacked == j) rs[i] = val;
                }
            }
#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                uint32_t si = ((lid + (FL::SubColsPacked - i)) % FL::SubColsPacked)
                              * FL::SubRowsPacked + lid / FL::SubColsPacked;
                rs[i] = __shfl_sync(0xFFFFFFFF, rs[i], si);
            }

            uint32_t tc = Base::get_tile_col(cp);
            uint32_t sc = Base::get_subtile_col(cp);
            uint32_t rc = FL::get_reg_col(sc);

#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                uint32_t rr = i % FL::RegRows;
                uint32_t tr = i / FL::RegRows;
                uint32_t ti = Base::tidx(tr, tc);
                uint32_t ri = FL::reg_idx(rr, rc);
                this->m_tiles[ti].regs[ri] = rs[0];
#pragma unroll
                for (uint32_t j = 1; j < FL::SubColsPacked; j++) {
                    if (lid % FL::SubColsPacked == (i + j) % FL::SubColsPacked)
                        this->m_tiles[ti].regs[ri] = rs[j];
                }
            }
        }
    }

    template<uint32_t N>
    __device__ void to_array(half* output) const {
        uint32_t lid = ptx_lane_id();

#pragma unroll
        for (uint32_t cp = 0; cp < Base::ColsPacked; cp += FL::SubColsPacked) {
            uint32_t rs[FL::SubColsPacked];
            uint32_t tc = Base::get_tile_col(cp);
            uint32_t sc = Base::get_subtile_col(cp);
            uint32_t rc = FL::get_reg_col(sc);
            uint32_t ri0 = FL::reg_idx(0, rc);
            uint32_t ti0 = Base::tidx(0, tc);

#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                rs[i] = this->m_tiles[ti0].regs[ri0];
#pragma unroll
                for (uint32_t j = 1; j < FL::SubColsPacked; j++) {
                    uint32_t rr = j % FL::RegRows;
                    uint32_t tr = j / FL::RegRows;
                    uint32_t ri = FL::reg_idx(rr, rc);
                    uint32_t ti = Base::tidx(tr, tc);
                    if (lid % FL::SubColsPacked == (i + j) % FL::SubColsPacked)
                        rs[i] = this->m_tiles[ti].regs[ri];
                }
            }
#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                uint32_t si = (lid % FL::SubRowsPacked) * FL::SubColsPacked
                              + (lid / FL::SubRowsPacked + i) % FL::SubColsPacked;
                rs[i] = __shfl_sync(0xFFFFFFFF, rs[i], si);
            }
#pragma unroll
            for (uint32_t i = 0; i < FL::SubColsPacked; i++) {
                uint32_t op = rs[0];
#pragma unroll
                for (uint32_t j = 1; j < FL::SubColsPacked; j++) {
                    if (lid / FL::SubRowsPacked == (FL::SubColsPacked - j + i) % FL::SubColsPacked)
                        op = rs[j];
                }
#pragma unroll
                for (uint32_t k = 0; k < 2; k++) {
                    uint32_t idx = (cp + i) * 2 + k;
                    if (idx < N) output[idx] = ((const half*)&op)[k];
                }
            }
        }
    }
};

// ============================================================================
// PART 8: Matrix/vector operations
// ============================================================================

template<uint32_t R, uint32_t C, bool RM>
__device__ TiledMatrix<R,C,!RM>
mat_change_major(const TiledMatrix<R,C,RM>& x) {
    TiledMatrix<R,C,!RM> r;
#pragma unroll
    for (uint32_t i = 0; i < x.MCols; i++)
#pragma unroll
        for (uint32_t j = 0; j < x.MRows; j++) {
            uint32_t idx = x.tidx(j, i);
            r.m_tiles[idx] = frag_change_major(x.m_tiles[idx]);
        }
    return r;
}

template<uint32_t R0, uint32_t C0, uint32_t C1>
__device__ HMatrixA<R0, C1>
mat_mad(const HMatrixA<R0,C0>& a, const HMatrixB<C0,C1>& b, const HMatrixA<R0,C1>& c) {
    HMatrixA<R0,C1> r = c;
    constexpr uint32_t K = a.MCols;
#pragma unroll
    for (uint32_t k = 0; k < K; k++)
#pragma unroll
        for (uint32_t mc = 0; mc < r.MCols; mc++)
#pragma unroll
            for (uint32_t mr = 0; mr < r.MRows; mr++) {
                uint32_t ri = r.tidx(mr, mc);
                uint32_t ai = a.tidx(mr, k);
                uint32_t bi = b.tidx(k, mc);
                r.m_tiles[ri] = frag_mad(a.m_tiles[ai], b.m_tiles[bi], r.m_tiles[ri]);
            }
    return r;
}

template<uint32_t R0, uint32_t C0, uint32_t C1>
__device__ HMatrixA<R0,C1>
mat_mul(const HMatrixA<R0,C0>& a, const HMatrixB<C0,C1>& b) {
    HMatrixA<R0,C1> c; c.clear();
    return mat_mad(a, b, c);
}

// mul: HVector<C0> * HMatrixB<C0,C1> → HVector<C1>
template<uint32_t C0, uint32_t C1>
__device__ HVector<C1>
vec_mul(const HVector<C0>& a, const HMatrixB<C0,C1>& b) {
    return mat_mul((const HMatrixA<WARP_SZ,C0>&)a, b);
}

// ============================================================================
// PART 9: Reduction operations
// ============================================================================

template<uint32_t R, uint32_t C>
__device__ void mat_reduce_row_sum(const HMatrixA<R,C>& a, half* dest) {
    constexpr uint32_t TR = R / 16, TC = C / 16;
    HMatrixA<R,C> acc = a;

#pragma unroll
    for (uint32_t tc = 0; tc < TC; tc++) {
#pragma unroll
        for (uint32_t j = 2; j < 2 * TR; j <<= 1)
#pragma unroll
            for (uint32_t tr = 0; tr < TR; tr += j)
                if (tr + j/2 < TR) {
                    uint32_t si = acc.tidx(tr + j/2, tc), di = acc.tidx(tr, tc);
                    acc.m_tiles[di] = frag_add(acc.m_tiles[di], acc.m_tiles[si]);
                }
    }
#pragma unroll
    for (uint32_t tc = 0; tc < TC; tc++)
        frag_reduce_row_sum(acc.m_tiles[acc.tidx(0, tc)], dest + tc * 16);
}

template<uint32_t N>
__device__ void vec_reduce_sum(const HVector<N>& a, half* dest) {
    mat_reduce_row_sum((const HMatrixA<WARP_SZ, N>&)a, dest);
}

template<uint32_t N0, uint32_t N1>
__device__ HMatrixB<N0, N1>
outer_product_reduce(const HVector<N0>& a, const HVector<N1>& b) {
    HMatrixB<WARP_SZ, N0> at = mat_change_major((const HMatrixA<WARP_SZ, N0>&)a);
    HMatrixA<N1, WARP_SZ> bt = mat_change_major(((const HMatrixA<WARP_SZ, N1>&)b).transpose());
    HMatrixA<N1, N0> w = mat_mul(bt, at);
    return w.transpose();
}

// ============================================================================
// PART 10: Cross-warp reducers
// ============================================================================

template<uint32_t NUM_THREADS, uint32_t N_COLS, ReduceMode MODE>
class SumReducer {
    static constexpr uint32_t NumWarps = NUM_THREADS / WARP_SZ;
    static constexpr uint32_t CPacked = N_COLS / 2;
public:
    __device__ __host__ static constexpr uint32_t shared_mem_size() { return NumWarps * N_COLS; }

    static __device__ void reduce_store(const HVector<N_COLS>& a, half* smem, half* dest) {
        uint32_t wid = ptx_warp_id(), lid = ptx_lane_id();
        vec_reduce_sum(a, smem + wid * N_COLS);
        __syncthreads();

        constexpr uint32_t ColsPerT = CPacked / NUM_THREADS;
        constexpr uint32_t NumIter = CPacked % NUM_THREADS ? ColsPerT + 1 : ColsPerT;
        uint32_t ci = wid * WARP_SZ + lid;
        auto* smh2 = (half2*)smem;
        auto* dsth2 = (half2*)dest;

#pragma unroll
        for (uint32_t j = 0; j < NumIter; j++) {
            if (ci < CPacked) {
                half2 acc = {__float2half(0.f), __float2half(0.f)};
#pragma unroll
                for (uint32_t w = 0; w < NumWarps; w++)
                    acc = __hadd2(acc, smh2[ci + w * CPacked]);
                if constexpr (MODE == ReduceMode::ATOMIC_ADD) ptx_red_add_h2(dsth2 + ci, acc);
                else dsth2[ci] = acc;
            }
            ci += NUM_THREADS;
        }
        __syncthreads();
    }
};

template<uint32_t NUM_THREADS, uint32_t N0, uint32_t N1, ReduceMode MODE>
class OuterProductReducer {
    static constexpr uint32_t NumWarps = NUM_THREADS / WARP_SZ;
    static constexpr uint32_t MatSz = N0 * N1;
    static constexpr uint32_t SmSz = NumWarps > 1 ? MatSz * NumWarps / 2 : MatSz;
public:
    __device__ __host__ static constexpr uint32_t shared_mem_size() { return SmSz; }

    static __device__ void reduce_store_native(
        const HVector<N0>& a, const HVector<N1>& b, half* smem, half* dst)
    {
        auto wt = outer_product_reduce(a, b);
        uint32_t wid = ptx_warp_id();

#pragma unroll
        for (uint32_t j = 2; j <= NumWarps; j <<= 1) {
            if (wid % j == j / 2)
                wt.store_native(&smem[(wid / j) * MatSz]);
            __syncthreads();
            if (wid % j == 0) {
                HMatrixB<N0, N1> tmp;
                tmp.load_native(&smem[(wid / j) * MatSz]);
                wt = wt + tmp;
            }
            __syncthreads();
        }

        if (wid == 0) {
            wt.store_native(smem);
            __syncwarp();

            constexpr uint32_t TR = N0 / 16, TC = N1 / 16, NR = 4;
            unsigned lid = ptx_lane_id();
            auto* dh2 = (half2*)dst;
            auto* sh2 = (half2*)smem;
#pragma unroll
            for (uint32_t tc = 0; tc < TC; tc++)
#pragma unroll
                for (uint32_t tr = 0; tr < TR; tr++) {
                    uint32_t base = (tc * TR + tr) * NR * WARP_SZ;
#pragma unroll
                    for (uint32_t i = 0; i < NR; i++) {
                        uint32_t off = base + i * WARP_SZ + lid;
                        if constexpr (MODE == ReduceMode::ATOMIC_ADD)
                            ptx_red_add_h2(dh2 + off, sh2[off]);
                        else
                            dh2[off] = sh2[off];
                    }
                }
        }
        __syncthreads();
    }
};

// ============================================================================
// PART 11: Backward kernel
// ============================================================================

int g_batch_size = 256;
constexpr int WARMUP_ITERATIONS = 100;
constexpr int BENCHMARK_ITERATIONS = 1000;
std::string g_mode = "full";

template<int Z_IP, int Z_OP, int NUM_THREADS = 64, bool ENABLE_BIAS = true, bool ENABLE_OUTER = true>
__global__ void backward_kernel(
    const half* __restrict__ input,
    const half* __restrict__ weights,
    const half* __restrict__ bias,
    half* __restrict__ output,
    const half* __restrict__ grad_output,
    half* __restrict__ grad_weights,
    half* __restrict__ grad_bias,
    half* __restrict__ grad_input,
    int batch_size)
{
    static constexpr uint32_t SPB = NUM_THREADS / WARP_SZ;
    int sid = blockIdx.x * SPB + (threadIdx.x / WARP_SZ);
    if (sid >= batch_size) return;

    using RO = OuterProductReducer<NUM_THREADS, Z_IP, Z_OP, ReduceMode::ATOMIC_ADD>;
    using RS = SumReducer<NUM_THREADS, Z_OP, ReduceMode::ATOMIC_ADD>;
    __shared__ half smem[std::max(RO::shared_mem_size(), RS::shared_mem_size())];

    HVector<Z_IP> x;
    { half a[Z_IP]; for (int i=0;i<Z_IP;i++) a[i]=__float2half(1.f); x.template from_array<Z_IP>(a); }

    HVector<Z_OP> grad_out;
    { half a[Z_OP]; for (int i=0;i<Z_OP;i++) a[i]=__float2half(1.f); grad_out.template from_array<Z_OP>(a); }

    HMatrixB<Z_IP,Z_OP> w;
    w.load_native(weights);

    auto wt = mat_change_major(w.transpose());
    HVector<Z_IP> grad_in = vec_mul(grad_out, wt);

    if constexpr (ENABLE_BIAS)  RS::reduce_store(grad_out, smem, grad_bias);
    if constexpr (ENABLE_OUTER) RO::reduce_store_native(x, grad_out, smem, grad_weights);

    half ga[Z_IP]; grad_in.template to_array<Z_IP>(ga);
    half s = __float2half(0.f);
#pragma unroll
    for (int i = 0; i < Z_IP; i++) s = __hadd(s, ga[i]);
    grad_input[sid * WARP_SZ + (threadIdx.x % WARP_SZ)] = s;
}

// ============================================================================
// PART 12: Benchmark harness
// ============================================================================

template<int Z_IP, int Z_OP, int NW = 2, bool EB = true, bool EO = true>
double run_bench(int bs, int warm, int iter) {
    constexpr int NT = NW * 32;
    size_t weights_size = Z_IP * Z_OP * sizeof(half);
    size_t bias_size = Z_OP * sizeof(half);

    half *d_weights, *d_grad_weights, *d_grad_bias, *d_grad_input;
    char *d_pad;

    cudaMalloc(&d_weights, weights_size);
    cudaMalloc(&d_pad, 512);
    cudaMalloc(&d_grad_weights, weights_size);
    cudaMalloc(&d_grad_bias, bias_size);
    cudaMalloc(&d_grad_input, bs * 32 * sizeof(half));

    std::vector<half> h_weights(Z_IP * Z_OP);
    srand(42);
    for (auto& v : h_weights) v = __float2half((rand() / (float)RAND_MAX - 0.5f) * 0.1f);
    cudaMemcpy(d_weights, h_weights.data(), weights_size, cudaMemcpyHostToDevice);

    dim3 grid(bs / NW), block(NT);
    for (int i = 0; i < warm; i++)
        backward_kernel<Z_IP,Z_OP,NT,EB,EO><<<grid,block>>>(
            nullptr, d_weights, nullptr, nullptr, nullptr,
            d_grad_weights, d_grad_bias, d_grad_input, bs);
    cudaDeviceSynchronize();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_weights); cudaFree(d_pad); cudaFree(d_grad_weights);
        cudaFree(d_grad_bias); cudaFree(d_grad_input);
        return -1.0;
    }

    cudaEvent_t t0, t1; cudaEventCreate(&t0); cudaEventCreate(&t1);
    cudaEventRecord(t0);
    for (int i = 0; i < iter; i++)
        backward_kernel<Z_IP,Z_OP,NT,EB,EO><<<grid,block>>>(
            nullptr, d_weights, nullptr, nullptr, nullptr,
            d_grad_weights, d_grad_bias, d_grad_input, bs);
    cudaEventRecord(t1); cudaEventSynchronize(t1);

    float ms; cudaEventElapsedTime(&ms, t0, t1);
    cudaEventDestroy(t0); cudaEventDestroy(t1);
    cudaFree(d_weights); cudaFree(d_pad); cudaFree(d_grad_weights);
    cudaFree(d_grad_bias); cudaFree(d_grad_input);
    return ms / iter;
}

template<int Z_IP, int Z_OP, int NW>
double run_mode(int bs, int w, int it) {
    if (g_mode == "transpose_only")  return run_bench<Z_IP,Z_OP,NW,false,false>(bs,w,it);
    if (g_mode == "transpose_bias")  return run_bench<Z_IP,Z_OP,NW,true,false>(bs,w,it);
    if (g_mode == "transpose_outer") return run_bench<Z_IP,Z_OP,NW,false,true>(bs,w,it);
    return run_bench<Z_IP,Z_OP,NW,true,true>(bs,w,it);
}

template<int ZIP, int ZOP, int NW>
void print_row(const char* n, int bs, double ms) {
    int p = ZIP*ZOP+ZOP;
    double thr = ms > 0 ? bs/(ms/1000.0) : 0;
    char net[32]; snprintf(net,sizeof(net),"%3d -> %3d",ZIP,ZOP);
    if (ms > 0)
        std::cout << std::left << std::setw(10) << n << net << "       "
                  << std::setw(5) << p << "      "
                  << std::fixed << std::setprecision(4) << ms << "       "
                  << std::setprecision(0) << thr << " samples/s\n";
    else
        std::cout << std::left << std::setw(10) << n << net << "       "
                  << std::setw(5) << p << "      FAILED\n";
}

int main(int argc, char** argv) {
    int only_warps = 0; std::string only_size;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i],"--batch-size") && i+1<argc) g_batch_size = atoi(argv[++i]);
        else if (!strcmp(argv[i],"--warps") && i+1<argc) only_warps = atoi(argv[++i]);
        else if (!strcmp(argv[i],"--size") && i+1<argc) only_size = argv[++i];
        else if (!strcmp(argv[i],"--mode") && i+1<argc) g_mode = argv[++i];
        else if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")) {
            std::cout << "Usage: " << argv[0] << " [--batch-size N] [--warps 1|2] [--size NAME] [--mode MODE]\n";
            return 0;
        }
    }
    if (g_batch_size <= 0 || g_batch_size % 2 != 0) { std::cerr << "batch size must be positive even\n"; return 1; }

    std::cout << "========================================\n"
              << "WmmaFragment Backward Benchmark\n"
              << "(all ops ported from Tin2 onto WmmaFragment)\n"
              << "========================================\n";
    int dev; cudaGetDevice(&dev);
    cudaDeviceProp prop; cudaGetDeviceProperties(&prop, dev);
    std::cout << "Device: " << prop.name << "\nBatch size: " << g_batch_size
              << "\nMode: " << g_mode << "\n";

#define RUN(n,z0,z1,nw) \
    if ((only_size.empty()||only_size==n)&&(only_warps==0||only_warps==nw)) \
        print_row<z0,z1,nw>(n, g_batch_size, run_mode<z0,z1,nw>(g_batch_size, WARMUP_ITERATIONS, BENCHMARK_ITERATIONS));

    if (only_warps==0||only_warps==1) {
        std::cout << "\n=== 1-warp (32 threads/block) ===\n"
                  << "Size       Network        Params      Time (ms)    Throughput\n"
                  << "----------------------------------------------------------------\n";
        RUN("tiny",32,16,1); RUN("small",64,16,1); RUN("medium",128,32,1);
        RUN("large",256,64,1); RUN("xlarge",128,128,1);
    }
    if (only_warps==0||only_warps==2) {
        std::cout << "\n=== 2-warp (64 threads/block) ===\n"
                  << "Size       Network        Params      Time (ms)    Throughput\n"
                  << "----------------------------------------------------------------\n";
        RUN("tiny",32,16,2); RUN("small",64,16,2); RUN("medium",128,32,2);
        RUN("large",256,64,2); RUN("xlarge",128,128,2);
    }
#undef RUN
    std::cout << std::endl;
    return 0;
}
