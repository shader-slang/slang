# L2 Partition Alignment: How a 512-byte cudaMalloc Can Give You 2x Speedup

## Summary

On NVIDIA RTX 4090 (sm_89), the performance of global-memory atomic reductions
(`red.relaxed.gpu.global.add.noftz.f16x2`) to a small buffer can vary by **2x**
depending on the **byte offset** of that buffer within its physical memory page.

The root cause is the L2 cache partition hash function. A small buffer whose
starting address is **size-aligned** keeps a critical hash input bit constant
across all its cache lines, concentrating atomics onto fewer L2 partitions.
Misaligning the buffer by half its size forces that bit to vary, spreading
atomics across 2x more partitions.

This is **completely undocumented** by NVIDIA.

## The Problem We Hit

We were benchmarking a backward-pass MLP kernel (tiny: 32->16, 2 warps, batch 8192).
The kernel does atomic reductions to a 1KB `grad_weights` buffer from 4096 blocks.

Two benchmark harnesses produced **identical SASS** (verified by `cuobjdump -sass` diff)
but had a consistent **1.9x performance gap**:

| Harness | `grad_weights` offset | Time |
|---|---|---|
| 4-buffer (Slang) | 0x400 (1KB-aligned) | 0.046 ms |
| 8-buffer (Tin2) | 0x600 (misaligned) | 0.024 ms |

## Background: L2 Partitions and Atomics

### What is a cache line?

Memory is accessed in 32-byte chunks called cache lines. A 1KB buffer has
1024 / 32 = 32 cache lines.

### What is L2 partitioning?

The RTX 4090's L2 cache is physically split into multiple independent slices.
The GPU has 12 memory controllers (FBPA units), each managing one L2 slice.
Each slice is further divided into sub-partitions; ncu reports 36 total
L2 sub-partitions (inferred from `lts__` metric sum/avg ratios):

```
Metric                                  sum          avg        sum/avg
-----------------------------------------------------------------------
lts__d_atomic_input_cycles_active   131,072     3,640.89          36
lts__d_sectors                      183,328     5,092.44          36
lts__t_requests                      41,406     1,150.17          36
lts__t_bytes                      5,264,384   146,232.89          36
```

All L2 metrics consistently report 36 units, confirming this is the L2
partition granularity, not just atomic units.

### Why partitioning matters for atomics

A hash function on the physical address assigns each cache line to exactly
one L2 partition. Regular loads can pipeline through a partition, but atomic
operations (read-modify-write) on the same cache line **serialize** at the
partition's atomic unit.

When 4096 blocks all do atomic adds to the same small buffer, the kernel
runtime is bounded by the **busiest partition**. If 32 cache lines cluster
onto 2 partitions, each handles 50% of atomics. If spread across 4 partitions,
each handles 25% → **2x faster**.

## Root Cause: Buffer Alignment vs. Hash Input Diversity

### The key insight

A 1KB buffer has 32 cache lines at addresses `base + j×32` for j=0..31.
The address bits that **vary** across these lines are bits [9:5] (5 bits,
covering 32 values). Bits above bit 9 come from the base address and are
**constant** across all 32 lines.

The L2 hash function uses **bit 10** (among others) to select partitions.
Whether bit 10 varies across the 32 cache lines depends on the buffer's
alignment:

### Case A: 1KB-aligned buffer (offset 0x400) — SLOW

Buffer spans 0x400–0x7FF. Bit 10 = 1 for ALL 32 lines (the buffer fits
entirely within one 1KB-aligned region, never crossing the next boundary
at 0x800):

```
CL  Address   bit10  bit7
 0   0x400      1     0
 1   0x420      1     0
 ...
 4   0x480      1     0
 5   0x4A0      1     0
 ...
15   0x5E0      1     1
16   0x600      1     0
 ...
31   0x7E0      1     1

Bit 10 is ALWAYS 1 → the hash loses one input bit → fewer partition outputs
```

Using a simplified hash model `partition = {bit10, bit7}`:
- All lines have bit10=1 → only partitions 2 and 3 reachable
- **2 partitions, 16 lines each**

### Case B: Misaligned buffer (offset 0x200) — FAST

Buffer spans 0x200–0x5FF. It **crosses the 0x400 boundary** where bit 10 flips:

```
CL  Address   bit10  bit7
 0   0x200      0     0
 1   0x220      0     0
 ...
15   0x3E0      0     1
              ─── 0x400 boundary: bit 10 flips! ───
16   0x400      1     0
17   0x420      1     0
 ...
31   0x5E0      1     1

First half: bit10=0, Second half: bit10=1 → hash gets one extra varying bit
```

Using the same hash `partition = {bit10, bit7}`:
- Lines 0-15: bit10=0 → partitions 0 and 1
- Lines 16-31: bit10=1 → partitions 2 and 3
- **4 partitions, 8 lines each**

### Why bit 10 specifically?

Bit 10 represents a 1KB boundary (2^10 = 1024). Our buffer is exactly 1KB:

- **1KB-aligned**: the buffer fits inside one 1KB region → bit 10 is constant
- **Misaligned by 512 bytes**: the buffer straddles two 1KB regions → bit 10 varies

This is like a 1-meter stick: it fits inside a 1-meter box if placed at the
edge, but sticks out if placed in the middle.

The general principle: a buffer of size S that is S-aligned keeps bit log2(S)
constant. Misaligning by S/2 forces that bit to vary.

### Why bit 9 of the base appeared to control the effect

`cudaMalloc` aligns small allocations to 512 bytes (0x200). So the base address
modulo 0x400 is either 0x000 or 0x200:

- base % 0x400 == 0x000 (bit 9 = 0): buffer is 1KB-aligned → doesn't cross → SLOW
- base % 0x400 == 0x200 (bit 9 = 1): buffer is misaligned → crosses → FAST

Bit 9 is not a direct hash input. It's a **proxy** for whether the 1KB buffer
crosses a bit-10 boundary.

## Direct Evidence

### 1. Identical SASS

Both harnesses compile to the same machine code:
- 352 instructions
- 40 registers, 0 spills
- Same opcode histogram (16 HMMA, 26 MOVM, 40 HADD2, 8 RED.E.ADD, ...)

### 2. ncu: L2 atomic unit load imbalance

Profiled with `ncu --metrics lts__d_atomic_input_cycles_active`:

| Metric | offset 0x400 (aligned) | offset 0x600 (misaligned) |
|---|---|---|
| gpu time | 44.42 us | 23.58 us |
| atomic_cycles.sum | 131,072 | 131,072 |
| **atomic_cycles.max** | **65,536** | **32,768** |
| atomic_cycles.min | 0 | 0 |
| atomic_cycles.avg | 3,640.89 | 3,640.89 |

Same total work (`.sum`), same average (`.avg`), but the **hottest partition**
does 2x more work at offset 0x400. This directly proves uneven partition
distribution.

### 3. VMM offset sweep

Using CUDA's Virtual Memory Management API (`cuMemCreate` + `cuMemMap`), we
mapped a single physical 2MB page and swept the offset of the 1KB atomic target
within it. Same virtual address base, same physical page, only the intra-page
offset varies:

```
offset  bit9  time_ms
0x0000    0   0.0464   (slow - 1KB-aligned)
0x0200    1   0.0226   (fast - misaligned, crosses 0x400)
0x0400    0   0.0466   (slow - 1KB-aligned)
0x0600    1   0.0234   (fast - misaligned, crosses 0x800)
0x0800    0   0.0472   (slow - 1KB-aligned)
0x0a00    1   0.0247   (fast - misaligned, crosses 0xC00)
0x0c00    0   0.0465   (slow - 1KB-aligned)
0x0e00    1   0.0226   (fast - misaligned, crosses 0x1000)
0x1000    0   0.0466   (slow - 1KB-aligned)
0x1200    1   0.0234   (fast)
0x1400    0   0.0472   (slow)
0x1600    1   0.0246   (fast)
0x1800    0   0.0464   (slow)
0x1a00    1   0.0225   (fast)
0x1c00    0   0.0472   (slow)
0x1e00    1   0.0247   (fast)
0x2000    0   0.0472   (slow)
```

**100% correlation** across 17 offsets. Every even-KB offset (bit 9 = 0) is slow,
every odd-0x200 offset (bit 9 = 1) is fast.

Verified across **4 different physical pages** and **2 different virtual addresses**
mapped to the same physical page — the virtual address doesn't matter, only the
physical intra-page offset.

### 4. Crossing more boundaries = more partitions

Larger buffers cross more power-of-2 boundaries, exposing more hash input bits.
Per-cache-line atomic throughput improves as the buffer grows:

```
Buffer size   CLs   Boundaries crossed    Per-CL speedup vs 1KB
    1 KB       32   bit 10                1.0x  (baseline, ~4 partitions)
    2 KB       64   bits 10-11            1.3x
    4 KB      128   bits 10-12            2.4x
    8 KB      256   bits 10-13            4.2x
   16 KB      512   bits 10-14            5.3x
   32 KB     1024   bits 10-15            6.4x
   64 KB     2048   bits 10-15            8.9x
  512 KB    16384   bits 10-15           10.4x
    1 MB    32768   bits 10-15           10.9x  (~44 effective partitions)
```

At ~64KB+ the buffer saturates most of the 36 L2 sub-partitions and per-line
throughput plateaus. Our 1KB buffer was the worst case: tiny buffer + unlucky
alignment = only 2 out of 36 partitions active.

### 5. Pair-wise partition probing

Fixing one cache line at offset 0x000 and pairing with lines at various offsets
reveals the partition hierarchy:

```
B_offset  time_ms  ratio   interpretation
0x000     0.0530   1.00    SAME partition
0x020     0.0398   0.75    same FBPA, different sub-partition
0x040     0.0407   0.77    same FBPA, different sub-partition
0x100     0.0261   0.50    DIFFERENT FBPA (full parallelism)
0x200     0.0393   0.75    same FBPA, different sub-partition
0x300     0.0261   0.50    DIFFERENT FBPA
```

Three distinct affinity levels (1.0x, 0.75x, 0.50x) confirm a two-level hierarchy:
- Partitions (FBPAs) are fully independent for atomics (0.50x ratio)
- Sub-partitions within an FBPA share some resource (0.75x ratio)

### 6. cudaMalloc sub-allocator behavior

Sequential `cudaMalloc` calls without freeing reveal the allocator's granularity:

```
alloc_size → actual_size → gap_to_next
     1B         512B          0x200
     5B         512B          0x200
   512B         512B          0x200
   513B        1024B          0x400
  1024B        1024B          0x400
  2048B        2048B          0x800
  4096B        4096B          0x1000
```

The minimum granularity is 512 bytes (0x200). Back-to-back 1KB allocations
step by 0x400, keeping bit 9 = 0 for all of them. A single 512-byte padding
allocation shifts subsequent addresses by 0x200, flipping bit 9 to 1.

## Reproducer

Minimal self-contained code that demonstrates the effect:

```cuda
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <cstdio>

// Kernel: 4096 blocks, each warp-0 does 8 atomic h2 adds to a 1KB buffer
__global__ void atomic_kernel(half* gw, int n) {
    int s = blockIdx.x * 2 + threadIdx.x / 32;
    if (s >= n) return;
    unsigned l = threadIdx.x % 32;
    if (threadIdx.x / 32 == 0) {
        half2 v = {__float2half(0.001f), __float2half(0.001f)};
        half2* g = (half2*)gw;
        for (int i = 0; i < 8; i++)
            asm("red.relaxed.gpu.global.add.noftz.f16x2 [%0], %1;"
                :: "l"(g + i * 32 + l), "r"(*((int*)&v)));
    }
}

double bench(half* gw, int bs) {
    cudaMemset(gw, 0, 1024);
    dim3 g(bs / 2), b(64);
    for (int i = 0; i < 200; i++)
        atomic_kernel<<<g, b>>>(gw, bs);
    cudaDeviceSynchronize();

    cudaEvent_t t0, t1;
    cudaEventCreate(&t0);
    cudaEventCreate(&t1);
    cudaEventRecord(t0);
    for (int i = 0; i < 2000; i++)
        atomic_kernel<<<g, b>>>(gw, bs);
    cudaEventRecord(t1);
    cudaEventSynchronize(t1);

    float ms;
    cudaEventElapsedTime(&ms, t0, t1);
    cudaEventDestroy(t0);
    cudaEventDestroy(t1);
    return ms / 2000;
}

int main() {
    const int bs = 8192;

    // Case 1: no padding -> grad_weights at offset 0x400 (1KB-aligned, SLOW)
    {
        half *dw, *dgw;
        cudaMalloc(&dw, 1024);
        cudaMalloc(&dgw, 1024);  // lands at 0x400
        printf("No padding:   dgw=%p  time=%.4f ms\n", dgw, bench(dgw, bs));
        cudaFree(dw);
        cudaFree(dgw);
    }

    // Case 2: 512-byte padding -> grad_weights at offset 0x600 (misaligned, FAST)
    {
        half *dw, *dgw;
        char *pad;
        cudaMalloc(&dw, 1024);
        cudaMalloc(&pad, 512);   // shifts next alloc by 0x200
        cudaMalloc(&dgw, 1024);  // lands at 0x600, crosses 0x800 boundary
        printf("With padding: dgw=%p  time=%.4f ms\n", dgw, bench(dgw, bs));
        cudaFree(dw);
        cudaFree(pad);
        cudaFree(dgw);
    }
}
```

Build and run:
```bash
nvcc -O3 -arch=sm_89 -w reproducer.cu -o reproducer && ./reproducer
```

Expected output (RTX 4090):
```
No padding:   dgw=0x...400  time=0.0470 ms
With padding: dgw=0x...600  time=0.0230 ms
```

## Impact On Our Benchmarks

The `transpose_outer` performance gap between Slang and Tin2 documented in
`TINY_BACKWARD_HANDOFF.md` was **entirely caused by this artifact**:

| Benchmark | Without padding | With padding | Tin2 |
|---|---|---|---|
| Slang generated CUDA | 0.046 ms | **0.024 ms** | 0.024 ms |
| WmmaFragment port | 0.047 ms | **0.024 ms** | 0.024 ms |

The Slang-generated kernel and the Tin2 kernel produce **identical SASS** and
have **identical performance** when the allocation alignment is controlled for.

Why Tin2 was accidentally fast:

```
Tin2 8-buffer allocation map:
  d_input       = 0x...c00000   (524KB)
  d_output      = 0x...c80000   (256KB)
  d_grad_output = 0x...cc0000   (256KB)
  d_grad_input  = 0x...d00000   (524KB)
  d_weights     = 0x...d80000   (1KB)     ← 0x400 of offset within sub-region
  d_bias        = 0x...d80400   (32B)     ← rounds to 0x200
  d_grad_weights= 0x...d80600   (1KB)     ← bit 9 = 1 → FAST (by accident!)
  d_grad_bias   = 0x...d80a00   (32B)
```

The two small allocations before `d_grad_weights` (`d_weights` = 1KB + `d_bias`
rounded to 512B = 1.5KB = 0x600) happened to push `d_grad_weights` to a
misaligned offset. Pure coincidence.

## Key Takeaways

1. **Atomic reduction performance can vary 2x based on buffer alignment** on
   NVIDIA GPUs. This is undocumented and architecture-specific.

2. **The root cause is L2 partition hash diversity.** When a small buffer is
   size-aligned, a hash input bit (the one at bit position log2(buffer_size))
   stays constant across all cache lines, halving the reachable partitions.
   Misaligning by half the buffer size forces that bit to vary.

3. **Larger buffers are less susceptible.** A 64KB+ buffer crosses enough
   boundaries to saturate most L2 partitions regardless of alignment. The
   effect is worst for small buffers (1-4KB) used as atomic reduction targets.

4. **Benchmark harness allocation patterns can silently dominate kernel timing**
   for atomic-heavy kernels. Always profile with `ncu` and check
   `lts__d_atomic_input_cycles_active.max` vs `.avg` to detect partition
   imbalance.

5. **The fix is trivial**: insert a `cudaMalloc(pad, buffer_size/2)` before
   the atomic target to misalign it. For a 1KB target, a 512-byte padding
   allocation is sufficient.

## How To Detect

```bash
ncu --metrics lts__d_atomic_input_cycles_active \
    --launch-skip 100 --launch-count 1 \
    ./your_binary
```

If `.max` is significantly larger than `.avg` (e.g., `.max ≈ .sum / 2` instead
of `.max ≈ .sum / 36`), you have partition imbalance. Try adding a padding
allocation before the atomic target buffer, sized at half the target buffer size.
