I will start a new feature implementation to slang.
@prelude/slang-cuda-prelude.h:7187-7188
If you look at this code, this is how to implement Cooperative matrix intrinsic for cuda target.
Basically, we just use cuda's wmma ptx instruction here.
You can find the document of wmma ptx instructions definitions here: https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-instructions-wmma

Now, we want to change the implementation to use another instruction set, called mma, you can find it here:
https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-instructions-for-mma

The distinct of mma from wmma is that wmma is simpler instruction set. It doesn't expose the matrix layout, so it provides an instruction for user to load the matrix from global memory or shared memory.

While mma is quite complex, it documents clearly about how the matrix is spread within a warp, so user will need to implement the matrix loading themselves. So we will have more freedom to make this load more efficient.

As a prototype, I only want to implement one matrix shape, m16n8k16, this is the document: https://docs.nvidia.com/cuda/parallel-thread-execution/#warp-level-matrix-fragment-mma-16816-float

So we can just integrate this implementation to "WmmaFragment" struct.

This is something special you will need to pay attention:
1. in all the Store methods, you can simply check the shape, and if the shape is m16n8k16, you can just dispatch to a new matrix load method, we can call it "mmaStoree"
2. same for the load.

Keep the things this way is the easiest way to implement this because we almost don't need to touch compiler code.

---

## MMA m16n8k16 Fragment Layout Reference (f16/bf16)

The `mma.sync.aligned.m16n8k16.row.col` instruction computes D = A*B + C where:
- Matrix A: 16 x 16 (M x K), f16/bf16
- Matrix B: 16 x 8  (K x N), f16/bf16
- Matrix C/D: 16 x 8 (M x N), f16 or f32 accumulator

Each thread in a warp (32 threads, T0-T31) holds a fragment of each matrix.

```
Thread grouping (same for all matrices):
    groupID           = laneid >> 2      (0..7, identifies which row-group)
    threadID_in_group = laneid % 4       (0..3, identifies which column-group)

    laneid:  0  1  2  3 | 4  5  6  7 | 8  9 10 11 | ... | 28 29 30 31
    groupID: 0  0  0  0 | 1  1  1  1 | 2  2  2  2 | ... |  7  7  7  7
    tID_grp: 0  1  2  3 | 0  1  2  3 | 0  1  2  3 | ... |  0  1  2  3
```

### Matrix A Fragment Layout (16x16, f16/bf16)

Each thread holds 8 f16 elements in 4 f16x2 registers:
- reg0 = {a0, a1}, reg1 = {a2, a3}, reg2 = {a4, a5}, reg3 = {a6, a7}

The 16x16 matrix is divided into 4 quadrants:
- Top-left  (rows 0-7,  cols 0-7):  reg0 {a0,a1}
- Top-right (rows 0-7,  cols 8-15): reg2 {a4,a5}
- Bot-left  (rows 8-15, cols 0-7):  reg1 {a2,a3}
- Bot-right (rows 8-15, cols 8-15): reg3 {a6,a7}

```
         |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
row  0   |  T0:a0a1 |  T1:a0a1 |  T2:a0a1 |  T3:a0a1 ||  T0:a4a5 |  T1:a4a5 |  T2:a4a5 |  T3:a4a5 |
row  1   |  T4:a0a1 |  T5:a0a1 |  T6:a0a1 |  T7:a0a1 ||  T4:a4a5 |  T5:a4a5 |  T6:a4a5 |  T7:a4a5 |
  ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
row  7   | T28:a0a1 | T29:a0a1 | T30:a0a1 | T31:a0a1 || T28:a4a5 | T29:a4a5 | T30:a4a5 | T31:a4a5 |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
row  8   |  T0:a2a3 |  T1:a2a3 |  T2:a2a3 |  T3:a2a3 ||  T0:a6a7 |  T1:a6a7 |  T2:a6a7 |  T3:a6a7 |
row  9   |  T4:a2a3 |  T5:a2a3 |  T6:a2a3 |  T7:a2a3 ||  T4:a6a7 |  T5:a6a7 |  T6:a6a7 |  T7:a6a7 |
  ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
row 15   | T28:a2a3 | T29:a2a3 | T30:a2a3 | T31:a2a3 || T28:a6a7 | T29:a6a7 | T30:a6a7 | T31:a6a7 |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
```

```
         |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
row  0   |  T0:a0a1 |  T0:a2a3 |  T0:a4a5 |  T0:a6a7 ||  T1:a0a1 |  T1:a2a3 |  T1:a4a5 |  T1:a6a7 |
row  1   |  T2:a0a1 |  T2:a2a3 |  T2:a4a5 |  T2:a6a7 ||  T3:a0a1 |  T3:a2a3 |  T3:a4a5 |  T3:a6a7 |
  ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
row  7   | T14:a0a1 | T14:a2a3 | T14:a4a5 | T14:a6a7 || T15:a0a1 | T15:a2a3 | T15:a4a5 | T15:a6a7 |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
row  8   | T16:a0a1 | T16:a2a3 | T16:a4a5 | T16:a6a7 || T17:a0a1 | T17:a2a3 | T17:a4a5 | T17:a6a7 |
row  9   | T18:a0a1 | T18:a2a3 | T18:a4a5 | T18:a6a7 || T19:a0a1 | T19:a2a3 | T19:a4a5 | T19:a6a7 |
  ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
row 15   | T30:a0a1 | T30:a2a3 | T30:a4a5 | T30:a6a7 || T31:a0a1 | T31:a2a3 | T31:a4a5 | T31:a6a7 |
---------+----------+----------+----------+----------++----------+----------+----------+----------+
```

Index formula:
```
row = groupID            for a[i] where 0 <= i < 2  or  4 <= i < 6
      groupID + 8        for a[i] where 2 <= i < 4  or  6 <= i < 8

col = threadID_in_group * 2 + (i & 0x1)          for a[i] where i < 4
      threadID_in_group * 2 + (i & 0x1) + 8      for a[i] where i >= 4
```

### Matrix B Fragment Layout (16x8, f16/bf16)

Each thread holds 4 f16 elements in 2 f16x2 registers:
- reg0 = {b0, b1}, reg1 = {b2, b3}

Unlike Matrix A, the f16x2 pairs span 2 adjacent *rows* (not columns).
Each column is owned by one group (groupID), each row-pair by threadID_in_group.

```
         |   col 0   |   col 1   |   col 2   |    ..     |   col 7   |
---------+-----------+-----------+-----------+-----------+-----------+
row  0,1 |  T0:b0,b1 |  T4:b0,b1 |  T8:b0,b1 |    ..     | T28:b0,b1 |
row  2,3 |  T1:b0,b1 |  T5:b0,b1 |  T9:b0,b1 |    ..     | T29:b0,b1 |
row  4,5 |  T2:b0,b1 |  T6:b0,b1 | T10:b0,b1 |    ..     | T30:b0,b1 |
row  6,7 |  T3:b0,b1 |  T7:b0,b1 | T11:b0,b1 |    ..     | T31:b0,b1 |
---------+-----------+-----------+-----------+-----------+-----------+
row  8,9 |  T0:b2,b3 |  T4:b2,b3 |  T8:b2,b3 |    ..     | T28:b2,b3 |
row 10,11|  T1:b2,b3 |  T5:b2,b3 |  T9:b2,b3 |    ..     | T29:b2,b3 |
row 12,13|  T2:b2,b3 |  T6:b2,b3 | T10:b2,b3 |    ..     | T30:b2,b3 |
row 14,15|  T3:b2,b3 |  T7:b2,b3 | T11:b2,b3 |    ..     | T31:b2,b3 |
---------+-----------+-----------+-----------+-----------+-----------+
```

Index formula:
```
row = threadID_in_group * 2 + (i & 0x1)          for b[i] where i < 2
      threadID_in_group * 2 + (i & 0x1) + 8      for b[i] where i >= 2

col = groupID
```

### Accumulator Matrix C/D Fragment Layout (16x8, f32)

Each thread holds 4 f32 elements in 4 separate registers:
- reg0 = c0, reg1 = c1, reg2 = c2, reg3 = c3

Same row-group structure as Matrix A, but only 8 columns (4 column-pairs).

```
         |<------------ columns 0-7 ------------>|
R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    |
---------+----------+----------+----------+----------+
row  0   |  T0:c0c1 |  T1:c0c1 |  T2:c0c1 |  T3:c0c1 |
row  1   |  T4:c0c1 |  T5:c0c1 |  T6:c0c1 |  T7:c0c1 |
  ..     |    ..    |    ..    |    ..    |    ..    |
row  7   | T28:c0c1 | T29:c0c1 | T30:c0c1 | T31:c0c1 |
---------+----------+----------+----------+----------+
row  8   |  T0:c2c3 |  T1:c2c3 |  T2:c2c3 |  T3:c2c3 |
row  9   |  T4:c2c3 |  T5:c2c3 |  T6:c2c3 |  T7:c2c3 |
  ..     |    ..    |    ..    |    ..    |    ..    |
row 15   | T28:c2c3 | T29:c2c3 | T30:c2c3 | T31:c2c3 |
---------+----------+----------+----------+----------+
```

Index formula:
```
row = groupID            for c[i] where i < 2
      groupID + 8        for c[i] where i >= 2

col = threadID_in_group * 2 + (i & 0x1)
```

### Register Summary

| Matrix | Dimensions | Element Type | Regs/Thread | Elements/Thread | Register Format |
|--------|-----------|--------------|-------------|-----------------|-----------------|
| A      | 16 x 16  | f16/bf16     | 4           | 8               | f16x2 (u32)    |
| B      | 16 x 8   | f16/bf16     | 2           | 4               | f16x2 (u32)    |
| C/D    | 16 x 8   | f32          | 4           | 4               | f32             |
| C/D    | 16 x 8   | f16          | 2           | 4               | f16x2 (u32)    |

### PTX Instruction Syntax

```
mma.sync.aligned.m16n8k16.row.col.f32.f16.f16.f32 d, a, b, c;

d:  {%rd0, %rd1, %rd2, %rd3}         -- 4 x f32 output accumulator
a:  {%ra0, %ra1, %ra2, %ra3}         -- 4 x f16x2 (u32) matrix A
b:  {%rb0, %rb1}                      -- 2 x f16x2 (u32) matrix B
c:  {%rc0, %rc1, %rc2, %rc3}         -- 4 x f32 input accumulator
```

### Implementation Approach

1. Add `RegisterCount` specializations for M=16, N=8, K=16
2. Add `mmaLoad` function:
   - Get `laneid` via `__lane_id()` or PTX `mov.u32`
   - Compute `groupID` and `threadID_in_group`
   - For each element index, compute (row, col) from the formulas above
   - Load `buffer[row * stride + col]` into the register
3. Add `mmaStore` function: inverse of load
4. In existing `WmmaFragment::Load` / `Store`, check `if constexpr (M==16 && N==8 && K==16)` and dispatch to `mmaLoad` / `mmaStore`
5. Add MMA PTX inline assembly for the multiply-accumulate