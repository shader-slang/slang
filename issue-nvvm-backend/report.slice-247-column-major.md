# Qualify the non-square column-major CUDA contract

## Motivation

The discovery cell `compute/non-square-column-major.slang#discovery-1` fails in all three modes.
Its relevant source is:

```slang
// Host bytes: [1, 0, 10, 0, 0, 1, 20, 0], each a Float32.
ConstantBuffer<float3x2> matrixBuffer;
RWStructuredBuffer<float> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3x2 M = matrixBuffer;
    float2 r = mul(float3(1, 2, 1), M);
    output[0] = r.x;
    output[1] = r.y;
}
```

The active graphics directive requests column-major layout and expects `11, 22`. Discovery replaces
its target with CUDA while preserving bytes and oracle. The original source disables CUDA and says
it ignores matrix layout. We needed to distinguish incorrect arithmetic from incompatible host
packing before proposing a compiler fix. This existing correctness investigation takes priority over
texture dimensions, FP8 admission and arbitrary RequirePrelude; none was investigated further.

## Proposed solution

Close this as a bounded research slice. CUDA already honors column-major layout for this
`ConstantBuffer<float3x2>` contract, using 12-byte columns. The graphics fixture supplies columns
16 bytes apart. Its original three failures remain failures; no oracle, input, identity, runner,
compiler or ABI change is justified by this evidence.

A useful next implementation gate, if this boundary is selected again, is a separate explicit CUDA
contract with six packed input floats and independent expected output, plus a full checkpoint for
any corpus/runner change. Preserve the old ID and historical mismatch. Adding general host-data
repacking to discovery would require an explicit target-layout-aware harness contract; rewriting
arbitrary TEST_INPUT bytes or inferring intent from source text is not acceptable. Broad claims
about every matrix/resource shape require separate evidence.

## Change summary

Only the completed plan, report, compact semantic evidence, STATUS and durable design/ledger
notes change. All controls, compiler traces, reflection, outputs and attempt logs remain under
`build/nvvm-loop/slice-247-before` and `slice-247-research`. No build or production edit occurred.

| Fresh check                                     | Result                                                            |
| ----------------------------------------------- | ----------------------------------------------------------------- |
| Runtime health                                  | 4/4 pass                                                          |
| Original discovery ID                           | 3/3 still runtime-mismatch, actual `11,1`, expected `11,22`       |
| Independent GPU controls                        | 18/18 pass; 144 output values checked                             |
| Original-source PTX/reflection/IR traces        | 6/6 compiles and SM80 assemblies pass                             |
| Original-source CUDA and HLSL source/reflection | 2/2 compiles pass                                                 |
| Matching final identity                         | 117 source/generated/test paths, 12 artifacts, 561 runtime inputs |

The controls use three byte payloads, both explicit major-layout flags, and NVRTC O3/NVVM O0/O3.
Each returns all six logical matrix elements and both multiply results. Expectations are generated
from integer byte offsets before execution, independently of either backend. Complete input/output
records and commands are retained. Results below hold in every mode:

| Input Float32 words               | Column-major multiply | Row-major multiply |
| --------------------------------- | --------------------- | ------------------ |
| Original `[1,0,10,0,0,1,20,0]`    | `11,1`                | `21,1`             |
| Packed intended `[1,0,10,0,1,20]` | `11,22`               | `22,20`            |
| Distinct `[2,3,5,7,11,13,17,19]`  | `13,42`               | `23,30`            |

The full246 preservation inventory remains 1695 cells: 1654 correct, 41 unresolved, 16 resolved
histories. Exactly three original cells are freshly replayed with equal five-field outcomes; 1692
cells are inherited. All 41 unresolved records and 16 resolved histories are retained exactly from246
with their original evidence status and references, explicitly historical here. No support unlock,
new registered cell, implementation-cadence increment or full checkpoint is claimed. Units,
semantic regressions, toolkit, runner and six material support cells remain inherited246. Material
runtime still needs bindings, textures/LUTs, input and output oracle; no kernel-performance claim.

## Concepts and vocabulary

- **Logical matrix:** `M[r][c]` and `mul` semantics, independent of buffer byte order.
- **Major-vector stride:** bytes between columns for column-major, rows for row-major. It is distinct
  from final object extent and the binding's pointer size.
- **PhysicalType wrapper:** the existing storage-lowering representation with a fixed major-vector
  array and explicit stride; it preserves target ABI before logical matrix legalization.
- **Adapted contract:** the discovery runner replaces target flags but retains source input/oracle.
  This intentionally exposes target differences; a failed adapted contract is not automatically a
  backend arithmetic bug.

## Process report

Helper/fallback inventory: no production helper, fallback or special case was added. Research-only
helpers are the byte-offset oracle, fixture generator, command driver, acceptance/index checker,
and a local copy of `complex-test-server.py` changing only its toolName from slangc to render-test.
They survive only as reproducible evidence. Every render control starts a separate existing test
server, executes one request, validates its response and output, and shuts down. This is not a
runner batching change. An initial absent standalone render-test launch and two local driver edit
errors are retained under `attempts/`; all occurred before GPU controls. No failed result is excluded
from the history, and no compiler source was rebuilt to repair these research scripts.

For column-major packing, CUDA offsets are `4*(3*c+r)`: columns `[1,0,10]` and `[0,0,1]`, yielding
`1+2*0+10=11` and `0+2*0+1=1`. Graphics offsets are `16*c+4*r`: columns `[1,0,10]` and `[0,1,20]`,
yielding `11,22`. Row-major CUDA offsets `4*(2*r+c)` give rows `[1,0]`, `[10,0]`, `[0,1]` and `21,1`.
The distinct-value control prevents sparse zeros from concealing a transpose or stride error; the
six-word control proves compact data gives the intended logical transform. Extra original words
are padding/data for the graphics contract, not values CUDA is required to read.

`slang-type-layout.cpp::_createTypeLayout` swaps major/minor counts for column-major and records
row/column strides. `CUDALayoutRulesImpl::GetVectorLayout` assigns Float3 size12/alignment4;
`GetMatrixLayout` uses the default array rule, giving column stride12 and total24. Float2 rows have
stride8, size24 and alignment8. Fresh CUDA/PTX reflection reports exactly those sizes/alignments.
Fresh HLSL reflection reports extent28/alignment16: two 12-byte columns separated by16, omitting the
last tail padding. The fixture supplies32 bytes. Reflection JSON's `elementStride:0` is not a matrix
stride report: `spReflectionTypeLayout_GetElementStride` handles arrays/vectors, not matrices. We
therefore use the canonical IR and actual load offsets for stride evidence.

`DefaultBufferElementTypeLoweringPolicy` deliberately chooses row-major as CUDA's native value
layout. `shouldLowerMatrixType` detects a requested column-major matrix and `lowerLeafLogicalType` creates
the physical column array. `createMatrixUnpackFunc` reads columns and constructs logical rows.
Generated CUDA contains `_MatrixStorage_float3x2_ColMajornatural` with `FixedArray<float3,2>` and
an explicit column-to-row construction. Thus a row-based CUDA `Matrix` value does not imply
row-based external storage. The original fixture comment and user-guide general statement about
ignored major flags are stale for this measured shape; this report does not generalize to all types.

For direct NVVM, `slang-emit.cpp` runs LLVM buffer-storage lowering before `legalizeMatrixTypes`.
Final IR explicitly contains `[PhysicalType]`, size24/alignment4 and
`Array(Vec(Float,3),2,12)`. `NVVMTypeLoweringContext::lowerType` reuses the wrapper's structured
storage representation; `_lowerArrayType` retains the storage role. Existing generic physical
storage support handles the compact vector3, so the old slice111 diagnostic is no longer current.
The NVVM emitter does not need a new matrix special case. NVRTC PTX loads offsets0,12,4,16,8,20 and
computes column sums; direct O3 loads0,4,8,12,16,20 and groups the first/last three. Both implement
the same proven byte contract. O0 runs the same oracle and its PTX also assembles.

Input-shape audit: the exact producer input is a valid column-major Float3x2 with CUDA natural
layout, intentionally represented by a physical compact array and logical unpack. This shape is
canonical, not an accidental alternative spelling. Its semantic source of truth is target layout;
no syntax reconstruction, custom equivalence, operand search, default-return guard or producer fix
is needed. The failing graphics oracle demonstrates a target-portability boundary, not ownership
by the NVVM consumer. A compiler change to force stride16 would contradict reflection and the
passing compact CUDA controls. There is no production revert drill because there is no production
change. The responsible next work, if desired, is explicit test-contract qualification, with the
original mismatch preserved and any broad layout-policy change separately designed and validated.

Research247 is independently accepted. Parent-audit.py/json verifies all144 output values and18
executions, three exact old outcomes, six reflection/assembly pairs,158 indexed artifacts,12 primary
source snapshots and166 compact references before its own two audit references. Source/binary/input
identities and41/16 histories match246 exactly. Latest implementation/full checkpoint stays246,
latest targeted acceptance233, implementation slices since full zero. Tested HEAD is
`739ead0d725f1075b4d382ec531ebe9070e60ea1`; ABI40, native Linux RelWithDebInfo, CUDA12.9 and SM80/L4
are unchanged. Exact source/binary hashes and all raw evidence indices are in the compact manifest.
