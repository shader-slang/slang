# Isolate the masked-prefix KernelContext preflight boundary

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Fresh workers
remain unavailable at the app's agent-thread limit; parent local execution follows WORKFLOW fallback.

## Purpose and Observable Result

Explain why the two frozen masked-prefix min/max workloads reject a canonical helper context pointer,
and establish a minimal independently runnable reproduction before choosing a fix. Research only:
no production change or support claim based merely on advancing an unsupported diagnostic.

## Progress

- [x] 2026-09-25: Select on accepted full 223 base `e3e30bd7200016565c00086ee466f75c23ca1fd8`.
- [x] Read affected sources, earlier context slice 115, producer and current helper-pointer checks.
- [x] Dump final/producer IR for an original failing workload and identify exact pointee fields.
- [x] Create minimal source/control cases; compile all modes and execute admitted source/direct cases
      against independent expected per-invocation state, retaining exact unsupported diagnostics.
- [x] Audit input shape, record responsible boundary/next action, verify unchanged223 hashes.
- [x] Complete report/evidence/STATUS and local research commit.

## Surprises and Discoveries

Slice115 already admits the exact ThreadLocal pointer spelling, but only when the pointee satisfies
`asNVVMSupportedScalarStructType`. The broader resource/copyable struct algebra is not automatically
admitted for explicit context pointers. Source addressSpace1 means Slang ThreadLocal, not NVVM global
address space 1. The diagnostic alone cannot identify the actual rejected field or imply a bad pointer.

## Decision Log

2026-09-25: Prioritize two existing frozen preflight failures with a shared representation boundary.
Quad reconvergence and ordinary FP64 vector-by-value shuffle have independent contracts. Material
runtime needs its missing input/oracle contract. Inspect canonical producer/pointee before widening
any emitter checks; retain any next independent prefix semantic blocker without absorbing it.

## Outcomes and Retrospective

Accepted research on 2026-09-25. The original context has Bool plus three uint fields. Minimal
Boolean global state reproduces both direct preflight stops; integer-global and local Boolean-struct
controls pass all modes. Seven GPU executions produce 672 exact output words; two direct Boolean-
global cells reject before PTX. No production or registered corpus changes; full 223/cadence 0 retained.

## Context and Current Pipeline

`wave-multi-prefix-min.slang` and max use static per-invocation state across generic helper calls.
The direct pipeline already runs CUDA `moveGlobalVarInitializationToEntryPoints` and
`introduceExplicitGlobalContext`. The latter creates KernelContext fields, entry-local storage and
explicit ThreadLocal helper parameters. `_isSupportedNVVMHelperParameterType` calls the type-lowering
pointer classifiers; `asNVVMSupportedLocalResourceStructPointerType` admits explicit context only for
flat selected scalar structs. Need inspect the actual generated field types and producer invariants.

## Scope and Non-Goals

IR/source traces, minimal runtime-source/control probes and current artifact preservation. No
pointer-admission edit, hardcoded KernelContext-name rule, provider/ABI change, corpus addition,
Float64 prefix admission, reconvergence implementation or material runtime claim.

## Architecture and Invariants

Each invocation owns initialized context storage; helper calls pass that same state. ThreadLocal and
compact entry-local pointer spellings intentionally differ but preserve pointee semantics. Any later
fix must reuse canonical type/layout support and prove real read/write state across helper boundaries,
not reinterpret the context as a device-global object or accept arbitrary pointer decorations.

## Interfaces and Dependencies

Native Ubuntu L4 SM89 target80 CUDA12.9.2/NVRTC12.9.86 LLVM14 ABI36; source env203. No build expected.
Accepted compiler01e06def851b6228dea63d2bbb18cb4c3167ea89542d542623ea79e9d6f3258d and provider
ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372 remain unchanged.

## Milestones and Validation

1. Reuse the just-completed full 223 original workload/runtime evidence. Compile one original source
   with `-dump-ir-after introduceExplicitGlobalContext` and final IR dump into ignored raw output.
   Inspect exact KernelContext fields and function/call pointer types, retaining command/diagnostic.
2. Select the smallest matching source representation without unrelated wave operations. Add a control
   with admitted pointee shape. Compile NVRTC O3/NVVM O0/O3, run accepted cases using a bounded CUDA
   driver probe after smoke 4/4, compare independent per-lane expected values and input preservation.
3. Verify all 24tested sources, 12 artifacts and 550runtime inputs unchanged. Preserve full 223 results,
   51open failures/sixresolved histories and cadence 0; research does not advance cadence.

## Failure and Recovery

Stop on GPU/device loss without driver/reboot changes. Keep expected preflight failures distinct from
crashes/wrong output. If no matching runnable reproduction can be established, retain research rather
than shipping diagnostic advancement. No oracle relaxation, baseline reset, pointer-name exception
or push. One owned writer; bound commands and retain raw apparatus corrections.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-224-context` for IR/source/commands/results/PTX/cubins. Durable completed
plan, five-part report, compact semantic evidence and STATUS. Parent locally reviews the exact shape,
producer/consumer trace and independent runtime evidence before selecting a bounded implementation.

2026-09-25 final review: producer IR confirms canonical explicit ThreadLocal context parameter and
compact entry-local argument. Scalar-only pointee guard rejects Bool despite admitted local copyable
Boolean structs. All seven assembled runnable cells and smoke 4/4 pass. All 24 tested-source hashes,
12 artifacts and 550runtime inputs match 223. Next 225 uses existing copyable-value classification for
canonical context pointees, retaining qualifiers/layout and resource-bearing context exclusion;
require representative runtime regression and full checkpoint before acceptance.
