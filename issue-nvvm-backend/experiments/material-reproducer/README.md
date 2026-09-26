# Reproduce material constant retention

This research fixture reduces the tiled brass material's constant-loss difference without changing
compiler code. Use the accepted262 compiler/provider and CUDA12.9 environment in
[RESULTS](../../RESULTS.md). It is outside the registered correctness inventories and does not claim
material runtime correctness or a GPU performance improvement.

The six wrappers include one shared graph. Each exposes exactly `computeMain`, so the standalone
PTX inspection and GPU test exercise the same entry body. The runtime harness currently defaults to
`computeMain` even when `-entry` names another function; the initial failed attempt is retained in
`build/nvvm-material-reproducer266/final-runtime.log`.

## Experiment and oracle

`makeGraph` initializes the whole graph, stores a runtime direction, calls `populate`, and returns the
graph by value. `populate` writes zero absorption for the constant case, calls a normal helper twice,
then writes one layer. The helper branches on the graph's zero `hints` field, so both normals must be
`(0, 0, 1)`. The normal indices are exactly 0 and 1 and the layer index is 0. Clearing the high bit
models the material's closure-kind decoding; it is not a bounds check.

The constant entry writes `[24, 1, 1, 1, 0, 0, 0, 0]`: three channels each contribute
`8 * exp2(0)`, both normal z components and the layer x component equal 1, and four untouched outputs
retain their initialized zeros. The control entry reads absorption values 1 and 2 from the input
buffer and writes `[12, 1, 1, 1, 6, 1, 1, 1]`, because `3 * 8 * exp2(-1) = 12` and
`3 * 8 * exp2(-2) = 6`. These are independent exact arithmetic oracles, not NVRTC reference outputs.

All 18 runtime cells pass: six wrappers times NVRTC O3, NVVM O0 and NVVM O3. All 12 standalone O3
PTX modules assemble for SM80. Each module has one entry and no retained calls. Static `ex2`
instruction counts on the unchanged accepted tools are:

| Variant                  | Constant NVRTC / NVVM | Control NVRTC / NVVM |
| ------------------------ | --------------------- | -------------------- |
| Masked graph             | 0 / 3                 | 2 / 6                |
| Unmasked indices         | 0 / 3                 | 2 / 6                |
| Branchless normal helper | 0 / 0                 | 2 / 2                |

Removing the normal helper's conditional restores elimination in both backends while retaining
runtime exponential work in the positive control. Removing the index mask does not restore
elimination in the final fixture. An earlier larger reduction was mask-sensitive; that observation
does not establish a universal mask requirement. Further combined deletions also remove the gap;
this is a small explanatory reproducer, not a claim of strict minimality.

## Reproduce

From the repository root, after setting the environment in RESULTS:

```bash
build/RelWithDebInfo/bin/slang-test \
  issue-nvvm-backend/experiments/material-reproducer/masked-constant.slang \
  issue-nvvm-backend/experiments/material-reproducer/masked-control.slang \
  issue-nvvm-backend/experiments/material-reproducer/unmasked-constant.slang \
  issue-nvvm-backend/experiments/material-reproducer/unmasked-control.slang \
  issue-nvvm-backend/experiments/material-reproducer/branchless-constant.slang \
  issue-nvvm-backend/experiments/material-reproducer/branchless-control.slang \
  -use-test-server -server-count 1 -disable-retries
```

For each wrapper, compile with `slangc <file> -target ptx -stage compute -entry computeMain -O3
-capability cuda_sm_8_0 -o <unique.ptx>`, once normally and once adding `-emit-cuda-via-nvvm`.
Assemble with `/usr/local/cuda-12.9/bin/ptxas -arch=sm_80 -v <unique.ptx> -o <unique.cubin>`.
For untimed inspection add `-dump-intermediates -dump-intermediate-prefix <unique-prefix>`;
`-dump-ir` separately captures shared Slang IR. Preserve logs and hashes of both the selected wrapper
and `graph.slangh`. The reference run's commands, all reductions, failures and full dumps are under
`build/nvvm-material-reproducer266/`.

## What the reduction establishes

The original material freshly reproduces zero NVRTC versus six NVVM exponential instructions in
both entries. A simple absorption-only consumer of the original constructor gives zero versus
three. Simplifying graph construction preserves that difference; removing its normal helper or
late layer write removes it. The standalone graph then preserves the difference with observable
normal/layer outputs and no texture or material runtime dependencies.

The source and input IR are valid: initialized aggregates, ordinary typed field addresses, and
in-range counters. Direct NVVM output still reloads counters and absorption around dynamically
indexed sibling-field stores even though the helper branch and calls eventually disappear. The
small fixture needs no zero-initialization loop. Returning only the payload also retained the gap
in an earlier controlled variant, so whole-graph return alone is not the cause.

This establishes a downstream optimization difference. It does not identify a particular libNVVM
pass or prove that any new alias annotation is sound. The next bounded experiment should inspect
memory promotion and optimization ordering at this branch/store boundary, retaining these oracles
and testing mutation, aliasing and escaping-reference cases before proposing a compiler change.
