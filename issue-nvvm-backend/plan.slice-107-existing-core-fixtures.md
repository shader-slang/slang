# Promote established core compute fixtures

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, six existing compute fixtures whose complete optimized IR already lies inside the
direct NVVM capability boundary run permanently through libNVVM: `simple.slang`,
`switch-stmt.slang`, `dot1.slang`, `entry-point-uniform-params.slang`,
`ieee754-mixed-type-nan-comparisons.slang`, and `logic-short-circuit-evaluation.slang`. Each fixture
keeps its existing expected output and gains a direct CUDA runtime lane. Representative direct PTX
checks prove the requested compiler route, and accepted PTX is assembled independently.

## Progress

- [x] (2026-08-29) Completed Slice 106 as `806c71ffa`, with 376/376 NVVM unit tests, 28/28 affected
  fixtures, focused surface negatives, direct runtime parity, and accepted PTX.
- [x] (2026-08-29) Probed a broad existing CUDA compute corpus and separated six already-supported
  core/value/control/ABI fixtures from distinct future capability buckets.
- [x] (2026-08-29) Added direct runtime and representative PTX coverage to the six selected
  existing fixtures.
- [x] (2026-08-29) Ran every exact new test, inspected output/PTX, and assembled every generated
  module.
- [x] (2026-08-29) Ran the complete 376-test NVVM unit prefix and the twelve exact affected tests.
- [x] (2026-08-29) Self-reviewed, updated durable design status and this plan, checked the diff, and
  prepared the slice commit.

## Surprises and Discoveries

- All six selected fixtures compile directly at `-O3` without production changes. Their PTX sizes
  are 674, 1,151, 477, 1,090, 1,253, and 953 bytes respectively. This means the highest-value next
  step is to turn latent capability into permanent existing-suite evidence before widening the
  emitter again.
- The same probe distinguished nearby unsupported buckets cleanly: aggregate helper parameters,
  unsigned and groupshared atomics, richer conventional-global fields, vector construction,
  `select`, and texture-query GenericAsm. Combining any one of those unrelated boundaries with this
  promotion slice would obscure whether failures came from existing capability or new lowering.
- `switch-stmt.slang` is especially useful existing-suite coverage for the descriptor-driven switch
  operation added in Slice 103. `entry-point-uniform-params.slang` composes global constant-buffer
  data, entry-point uniform structs/resources, launch semantics, and structured-buffer output.
- Broad filename prefixes also select automatically synthesized WebGPU tests on this machine. Dawn
  rejects their empty bind-group-layout entries before dispatch. The twelve exact new CUDA/PTX test
  IDs pass independently, proving the direct backend contract without treating the unrelated
  WebGPU environment failure as a libNVVM result.
- The first PTX check for `entry-point-uniform-params.slang` listed `ld.param` before
  `ld.global.nc`, while optimized PTX emits the global constant-buffer load first. Reordering the
  stable patterns fixed the test; no production change was involved.

## Decision Log

- Decision: make Slice 107 a test-promotion slice for all six already-supported fixtures.
  Rationale: the prototype needs breadth evidence from real suite shaders, not only new callbacks.
  These fixtures form one coherent gate: no new production capability is expected, and a runtime
  failure would identify a gap hidden by successful compilation.
  Date/author: 2026-08-29, Codex.
- Decision: keep the first unsupported buckets out of this slice and measure them for Slice 108.
  Rationale: aggregate ABI, atomics, resource layout, `select`, and texture queries have different
  producers and provider semantics. The next slice should select one of them based on fixture
  impact after this regression baseline is committed.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slices 72-106 established the conventional CUDA launch/global ABI, selected scalar/vector and
aggregate values, structured/byte-address buffers, control flow, sampled textures, and read-write
surfaces. Many synthetic and focused fixtures permanently exercise those features, but several
older compute-suite shaders still have only ordinary CUDA lanes even though direct compilation now
succeeds.

The six selected fixtures cover complementary compositions:

- `simple.slang`: launch index, integer-to-float conversion, and Float32 structured-buffer store;
- `switch-stmt.slang`: helper call, integer switch, merges, and signed integer buffer traffic;
- `dot1.slang`: one-lane vector normalization, float/integer arithmetic, and conversion;
- `entry-point-uniform-params.slang`: collected globals plus conventional entry uniform structs and
  a resource parameter;
- `ieee754-mixed-type-nan-comparisons.slang`: mixed integer/Float32 conversion and ordered/unordered
  comparison behavior;
- `logic-short-circuit-evaluation.slang`: helper side effects and short-circuit CFG.

## Scope and Non-Goals

In scope:

- direct CUDA runtime lanes using `-emit-cuda-via-nvvm`, `cuda_sm_7_0`, and optimized IR;
- representative direct PTX checks that distinguish the direct route;
- independent `ptxas` acceptance for every selected fixture;
- retained ordinary backend lanes and expected buffers unchanged;
- durable design status and exact validation evidence.

Out of scope:

- adding a production operation merely to admit another candidate;
- changing shader semantics or expected buffers;
- aggregate helper ABI, atomics, new resource/global field types, `select`, or texture queries;
- generic entry-point specialization and dynamic dispatch;
- the neighboring ordinary PTX surface-source failure recorded by Slice 106.

## Architecture and Invariants

- A direct runtime directive is valid only if the fixture compiles through the explicit direct
  option and produces its pre-existing expected output.
- No test is simplified, duplicated, or replaced with a synthetic source. Existing suite shaders
  remain the semantic source of truth.
- PTX checks should identify stable semantic instructions or kernel shape, not optimizer register
  numbers or formatting.
- Any production failure stops this promotion slice for diagnosis; it is not papered over with a
  fixture-specific special case.

## Interfaces and Dependencies

No ABI revision or provider interface change is planned. Modify only the six existing fixtures,
`docs/design/nvvm-backend.md`, and this plan unless validation exposes a principled defect in
already-admitted behavior.

## Validation and Acceptance

Acceptance requires all direct runtime outputs to match their existing checks, direct PTX from all
six fixtures, CUDA 12.9 `ptxas -arch=sm_70` acceptance for every module, all affected test prefixes,
the complete `slang-unit-test-tool/nvvm` prefix, pinned clang-format where applicable, and
`git diff --check`.

Record the exact affected counts, output values, PTX/cubin sizes, any runtime-only gap, the next
measured capability boundary, and self-review.

## Self-Review and Input-Shape Audit

Inventory every test-directive and PTX-check change. Confirm that each fixture's final IR uses only
already-admitted canonical shapes and that no production helper/fallback/special case was added. If
a fixture needs production code, identify its exact producer and decide whether to move it to the
next capability slice rather than weakening this regression gate.

## Failure and Recovery

If direct runtime, LLVM verification, libNVVM, or `ptxas` rejects a fixture that compiled during the
probe, preserve diagnostics under ignored `build/`, remove no existing coverage, and diagnose the
principled gap. Never reset unrelated work or stage `external/slang-binaries/`.

## Outcomes and Retrospective

All six selected fixtures now carry an optimized direct CUDA runtime directive and a direct PTX
directive. Their existing outputs pass unchanged: `simple.slang` writes Float32 values 0, 1, 2,
and 3; `switch-stmt.slang` preserves its eight-value switch comparison; `dot1.slang` writes 8;
`entry-point-uniform-params.slang` composes the values 1, 2, 3, and the thread index;
`ieee754-mixed-type-nan-comparisons.slang` preserves its 24 IEEE comparison results; and
`logic-short-circuit-evaluation.slang` writes four ones, eight zeroes, and four ones.

The twelve exact new tests pass 12/12. Their optimized PTX sizes are 674, 1,151, 477, 1,090,
1,253, and 953 bytes in the plan's fixture order. CUDA 12.9.86 `ptxas -arch=sm_70` accepts all six
and emits 2,792-, 3,240-, 2,664-, 2,920-, 3,048-, and 2,920-byte cubins. The complete Release NVVM
unit prefix remains 376/376. Broad fixture prefixes additionally trigger unrelated synthesized
WebGPU lanes that fail at Dawn bind-group-layout creation on this machine; those failures are
recorded rather than attributed to direct NVVM.

Self-review found only test directives and stable semantic PTX patterns. No production helper,
fallback, special case, builder operation, or ABI change was added. Removing any runtime directive
removes the corresponding real-shader execution proof; removing any PTX directive removes its
explicit direct compiler-route proof. Existing shader bodies and expected buffers remain untouched.

The corpus probe suggests the highest-impact next capability bucket is generalized aggregate
helper ABI: `array-param.slang`, `column-major.slang`, `func-param-legalize.slang`,
`mutating-and-inout.slang`, `structured-buffer-of-struct.slang`, `struct-in-generic.slang`, and
`typedef-member.slang` all first stop at helper parameter/result shapes. Slice 108 should measure
those final IR signatures together before choosing a bounded aggregate transport contract.
