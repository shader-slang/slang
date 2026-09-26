# Narrow a receiver snapshot without deleting its branch

These source counterfactuals use the unchanged accepted262 compiler. Compare them with
[`material-reproducer`](../material-reproducer/README.md). Only the normal helper's inputs change:
`Graph.adjust(normal)` becomes `adjust(hints, direction, normal)`. The conditional, graph fields,
initialization, counter updates, masked indices, payload writes and observable outputs remain.
This is research evidence, not a compiler optimization or material GPU performance claim.

The exact `computeMain` wrappers retain independent integer oracles. The constant entry writes
`[24, 1, 1, 1, 0, 0, 0, 0]`; absorption inputs 1 and 2 in the control entry produce
`[12, 1, 1, 1, 6, 1, 1, 1]`. Both normal z components and the layer x component are checked.

| O3 module          | Original NVRTC / NVVM exponentials | Narrow NVRTC / NVVM exponentials |
| ------------------ | ---------------------------------- | -------------------------------- |
| Constant           | 0 / 3                              | 0 / 0                            |
| Runtime absorption | 2 / 6                              | 2 / 2                            |

A separate raw runtime-hints probe supplies hints 0 and 1 and direction `(1, 2, 3)`, so both
conditional arms execute. Its independent result is `[12, 1, 1, 1, 6, 4, 4, 1]`; the original and
narrow versions pass all three modes. The branch has not been removed by the source rewrite.

Use the accepted262 environment in [RESULTS](../../RESULTS.md) to reproduce this historical source
comparison. The slice267 compiler also optimizes the original source, so it no longer exhibits the
same before/after difference. This checkout preserves accepted262's matching library/module layout
under `build/nvvm-receiver-snapshot267/baseline-layout`. Run from the repository root:

```bash
build/nvvm-receiver-snapshot267/baseline-layout/RelWithDebInfo/bin/slang-test \
  issue-nvvm-backend/experiments/receiver-snapshot/masked-constant.slang \
  issue-nvvm-backend/experiments/receiver-snapshot/masked-control.slang \
  -use-test-server -server-count 1 -disable-retries
```

For each wrapper, compile with `slangc <file> -target ptx -stage compute -entry computeMain -O3
-capability cuda_sm_8_0 -o <unique.ptx>`, once normally and once with `-emit-cuda-via-nvvm`.
Assemble with `/usr/local/cuda-12.9/bin/ptxas -arch=sm_80 -v <unique.ptx> -o <unique.cubin>`.
Capture Slang IR with `-dump-ir`, or CUDA/LLVM intermediates with
`-dump-intermediates -dump-intermediate-prefix <unique-prefix>`. Hash the included `graph.slangh`
as well as the wrapper. These flags were verified against the accepted local compiler.

## Material relevance and optimization boundary

A raw copy of the original material changes only `adjust_normal` to a static helper taking
`SurfaceInteraction si`, `uint hints` and the normal, and expands its policy query to the same
expression. Its conditional and calculation remain. Both material entries change from six NVVM
exponentials to zero, while NVRTC remains zero. NVVM entry stack size changes from 784 bytes to
zero; registers change from 67 to 52 for evaluation and 86 to 62 for sampling, with zero spills
reported in these assemblies. These are source comparisons; material runtime remains unassessed.

The original Slang IR is valid. `prepare` loads a whole `Graph` SSA snapshot and passes it by value
to `adjust`; the helper extracts only hints and direction. The load-narrowing pass requires extract
users, so the call prevents narrowing. Its immutable-buffer argument specialization excludes this
mutable local graph. The narrowed helper instead receives field values loaded at the call site;
its branch remains in both emitted CUDA and direct LLVM IR. Final NVVM PTX eliminates constant work.
The responsible internal libNVVM pass has not been identified.

The CUDA source route additionally runs `transformParamsToConstRef`, producing a temporary graph
snapshot and passing its address. Direct NVVM skips that pass. An explicit source model of the
same snapshot-plus-constref idea fails accepted NVVM compilation with E52018, `compact parameter-group
vector element extraction`. The failure also occurs in a minimal `__constref Payload` helper reading
a float3 field, while the float4 version compiles. The readonly helper field is mistaken for compact
parameter-group vector storage. This is a separate capability blocker; no compiler repair is included.

Full source probes, emitted intermediates, command logs, hashes, resources, GPU outcomes and retained
failures are under ignored `build/nvvm-receiver-snapshot267/`. The slice's
[report](../../report.slice-267-receiver-snapshot.md) and structured validation record own compiler
acceptance; the measurements above retain their accepted262 source-counterfactual identity.
