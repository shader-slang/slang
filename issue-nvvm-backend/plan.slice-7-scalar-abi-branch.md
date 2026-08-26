# Preserve the scalar CUDA kernel ABI and lower an i32 branch through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 7 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, ordinary Slang CUDA kernels with signed 32-bit scalar and device-pointer
parameters can use the experimental direct-NVVM route. These three isolated sources define the
accepted boundary:

```slang
[CUDAKernel]
void writeScalar(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value;
}
```

```slang
[CUDAKernel]
void copyScalar(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
```

```slang
[CUDAKernel]
void chooseScalar(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    if (x < y)
        *destination = x + y;
    else
        *destination = x - y;
}
```

Selecting any one function with `-target ptx -emit-cuda-via-nvvm -stage compute -capability
cuda_sm_7_0` preserves the raw CUDA launch parameters instead of gathering them into a synthetic
constant buffer. `AddressSpace::Device` reaches linked IR as the canonical
`AddressSpace::UserPointer` spelling and lowers directly to `i32 addrspace(1)*`. The conditional
remains an SSA control-flow graph and lowers one-to-one to LLVM integer comparison/arithmetic and
branches. No CUDA-C++ local-variable or reference representation is introduced.

Real acceptance compares the same Slang source through NVRTC and direct NVVM. PTX must expose
parameter widths `[64, 32]`, `[64, 64]`, and `[64, 32, 32]` respectively, preserve the expected
global load/store semantics, and assemble with `ptxas`. When a CUDA driver and device are
available, an optional runtime differential launches both routes and observes identical results.

This slice is deliberately constant-free and phi-free. Loops, block parameters/phi nodes,
integer literals in executable code, calls, non-void returns, multiplication, other scalar types,
builtins, resources, and aggregates remain stable unsupported boundaries. Keeping those out makes
the first ABI/control-flow increment demonstrable without lowering canonical SSA back into
C-like temporaries.

## Progress

- [x] (2026-08-26 16:24Z) Re-read `.agent/PLANS.md`, the durable NVVM design and capability
  ledger, the completed Slice 6 hand-off, and the current shared PTX pipeline, NVVM emitter,
  V2 builder/provider ABI, and unit-test infrastructure.
- [x] (2026-08-26 16:24Z) Completed independent read-only audits of linked scalar IR, builder ABI
  evolution, launch semantics, and the smallest deterministic/real/runtime validation matrix.
- [x] (2026-08-26 16:24Z) Probed conventional compute and selected `[CUDAKernel]` functions before
  and after CUDA-source legalization and phi elimination; removed the temporary probe afterward.
- [x] (2026-08-26 16:24Z) Chose a bounded i32 parameter/load/store/conditional capability and
  created this ExecPlan before production implementation.
- [x] (2026-08-26 19:44Z) Preserved the canonical shared CUDA ABI and SSA shape at narrow
  direct-NVVM pipeline policy points, including selective retention of only the chosen CUDA kernel.
- [x] (2026-08-26 19:44Z) Appended and implemented the coherent V2 scalar-control-flow builder
  capability while preserving every prior minimum.
- [x] (2026-08-26 19:44Z) Replaced the empty-only validator/emitter with complete preflight
  legality, dominance checks, and one-to-one lowering for the accepted subset.
- [x] (2026-08-26 19:44Z) Added exact fake graphs, real NVRTC/NVVM differential PTX, `ptxas`, and
  CUDA-driver runtime coverage while retaining constants, phis, loops, and calls as deterministic
  next boundaries.
- [x] (2026-08-26 19:44Z) Built and tested outside the sandbox, formatted with clang-format 17,
  completed the helper/special-case audit, inspected the provider binary, and updated the durable
  design and capability ledger.

## Surprises and Discoveries

- Observation: direct PTX currently applies generic shader-parameter collection to CUDA launch
  parameters.
  Evidence: a direct probe of `writeScalar(int* destination, int value)` became a zero-parameter
  `Func(Void)` plus `global_param ConstantBuffer<EntryPointParams>` and body
  `get_field_addr`/`load` instructions. The same source at the final CUDA-source observation point
  remained `Func(Void, Ptr(Int, ..., UserPointer), Int)` with one store.
  Consequence: fix the shared pipeline producer so direct NVVM preserves raw entry parameters. Do
  not teach the emitter that the gathered constant buffer is an equivalent kernel ABI.

- Observation: a selected `[CUDAKernel]` definition is deleted on direct PTX before NVVM
  validation.
  Evidence: `removeTorchAndCUDAEntryPoints` strips its temporary keep-alive/HLSL-export state for
  PTX; dead-code elimination then removes the selected function, and Slice 6 reports E52017 on
  `entry-point definition`. CUDA source skips that pass and retains the definition.
  Consequence: preserve selected CUDA kernels on the direct route at the producer boundary. Never
  work around a stale/dead `LinkedIR.entryPoints` pointer in the validator.

- Observation: `lowerImmutableBufferLoadForCUDA` is selected by broad `isCUDATarget`, including
  direct PTX, and rewrites immutable pointer loads to the CUDA-source `CUDALDG` intrinsic.
  Evidence: the gathered direct `copyScalar` probe acquired `CUDA_LDG`, while the pre-rewrite load
  is already valid canonical memory IR for NVVM.
  Consequence: exclude direct NVVM from this NVRTC representation pass. A later memory-policy slice
  may deliberately choose NVVM metadata or intrinsics using differential evidence.

- Observation: the common late pipeline always eliminates block parameters into `IRVar`, loads,
  and stores, then calls `simplifyNonSSAIR` and C-like variable-scope correction.
  Evidence: before `eliminatePhis`, a branch merge uses block parameters and a loop uses `IRLoop`
  plus header parameters; afterward they are explicit stack-style temporaries. The durable design
  already says NVVM should keep SSA unless libNVVM evidence requires otherwise.
  Consequence: direct NVVM bypasses phi elimination and its non-SSA cleanup/scope-correction
  consumers. This slice accepts only blocks without parameters, but it preserves the principled
  representation needed by the next phi capability.

- Observation: a constant-free branch can exercise useful scalar control flow without a phi,
  executable literal, loop, or call.
  Evidence: storing `x + y` or `x - y` in the two arms produces `cmpLT`, `ifElse`, `add`/`sub`, two
  stores, zero-argument `unconditionalBranch` terminators, and `return_val(void_constant)`. A
  merged local instead creates a merge-block parameter, and the loop/call probe additionally
  requires constants, phis, non-void return, multiplication, and call ABI.
  Consequence: the arm-store form is the smallest coherent Slice 7 control-flow proof. The larger
  shapes remain explicit subsequent capability blocks.

- Observation: raw parameter layouts retain module-scope `IRStructKey` nodes, and
  `getIROpInfo(...).isHoistable()` does not classify them.
  Evidence: the CUDA-final parameter probe contains layout field keys even though no struct value
  or storage is emitted; rejecting the key makes a valid scalar parameter fail before its body is
  examined.
  Consequence: the legality boundary may admit `IRStructKey` specifically as layout identity, with
  a concrete producer/consumer comment and a test that fails if the allowance is removed. Prefer
  an existing general layout-metadata classifier if the implementation audit finds one.

- Observation: Slang's public `AddressSpace::Device` pointer syntax intentionally lowers to the IR
  enum value `AddressSpace::UserPointer` (`0x100000001`), not `AddressSpace::Global`.
  Evidence: `core.meta.slang` aliases `Device` to `UserPointer`, and both CUDA-final probes retain
  that operand. Slice 4's NVRTC/libNVVM differential established `i32 addrspace(1)*` as the kernel
  device-pointer ABI.
  Consequence: map only this canonical kernel-pointer spelling to NVVM global address space 1 in
  this slice. Do not reinterpret every Slang address-space enum or insert pointer/integer casts.

- Observation: the established CUDA emitter does not turn `[numthreads]` into a PTX launch
  constraint.
  Evidence: its entry-point attribute hook is empty, emitted CUDA has no `__launch_bounds__`, and
  NVRTC PTX for `[numthreads(2, 3, 4)]` contains no `.reqntid`. NVVM `reqntid*` annotations would
  impose an exact launch-dimension constraint absent from the established route.
  Consequence: keep the frozen `markFunctionAsKernel` behavior and do not emit `.reqntid` in this
  parity slice. Thread-group dimensions remain reflection/host-dispatch data.

- Observation: the file-test harness cannot require the independently optional LLVM 14 builder.
  Evidence: its test requirements know the PTX/NVRTC route but have no provider-availability bit;
  an unconditional positive direct-NVVM file test would vary by machine.
  Consequence: keep positive public-route coverage in injected unit tests, where the builder and
  libNVVM can be faked or explicitly preflighted. Retain the provider-independent barrier file as
  a stable later-feature rejection.

- Observation: removing every CUDA/Torch keep-alive pin and skipping the cleanup pass are both too
  broad for direct NVVM.
  Evidence: skipping cleanup retained an unselected `[CUDAKernel]` containing a barrier call; the
  emitter then rejected that second semantic global even though only `computeMain` was selected.
  Consequence: always run the normal cleanup, then restore `IRKeepAliveDecoration` only on the
  exact `LinkedIR.entryPoints` selections before DCE. The selected-kernel regression fails if this
  producer boundary is broadened again.

- Observation: LLVM context/module equality is not enough for function-local operands.
  Evidence: an argument or instruction from a second function in the same module passed the first
  ownership implementation, and a value produced in one conditional arm could be presented at a
  sibling insertion point.
  Consequence: both Slang preflight and the LLVM 14 provider enforce dominance and exact current-
  function ownership. Cross-module, cross-function, sibling, merge, same-block-order, and
  post-terminator rejection tests prove failure occurs before mutation.

- Observation: executable constants and phi-shaped source fail at canonical semantic edges that
  differ from their source spelling.
  Evidence: the final unsupported-source table reports E52017 on `integer_constant`, on `branch
  argument` for the selected-value merge, on `loop`, and on `call`, all before builder discovery.
  Consequence: Slice 8 starts from constants plus branch arguments/non-entry parameters rather than
  matching syntax such as an integer literal or local variable.

- Observation: downstream optimization does not preserve a literal subtraction instruction in
  the conditional PTX.
  Evidence: both direct NVVM and NVRTC optimized the source so stable PTX still contained signed
  comparison and addition but not necessarily an explicit `sub` mnemonic or branch.
  Consequence: differential assertions remain semantic: exact parameter widths, global memory,
  signed `.s32` comparison, assembler acceptance, and runtime results including
  `choose(-2, 1) == -1`.

## Decision Log

- Decision: accept signed i32 parameters/values, `Ptr<i32, UserPointer>` parameters, load/store,
  add/sub, signed less-than, conditional/unconditional branches, and void return.
  Rationale: `writeScalar`, `copyScalar`, and `chooseScalar` jointly prove the first CUDA parameter
  ABI, both memory directions, observable scalar arithmetic, and multi-block control flow while
  mapping each accepted semantic operation exactly once.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the next capability adds executable constants and block parameters/phis.

- Decision: preserve raw parameters and selected CUDA kernels through narrowly named direct-NVVM
  policy checks in `linkAndOptimizeIR`.
  Rationale: parameter gathering and CUDA-kernel removal create accidental non-CUDA or dead shapes.
  `TargetProgram::shouldEmitNVVMDirectly()` already identifies the representation split; using it
  at the owning passes fixes the producer without pretending the target is CUDA C++.
  Date/Author: 2026-08-26, Codex.
  Revisit when: target legalization is factored into explicit semantic and representation phases.

- Decision: do not opt direct NVVM into CUDA varying-parameter legalization, `undoParameterCopy`,
  `transformParamsToConstRef`, explicit C++ global context, immutable-load `CUDA_LDG`, or other
  CUDA-source transforms.
  Rationale: none is needed for the accepted raw scalar ABI, and each would create a C++ spelling
  the direct emitter would then have to reverse. Builtins, references, and immutable-load policy
  need their own NVVM contracts.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the corresponding feature slice names a canonical NVVM representation.

- Decision: retain SSA by bypassing `eliminatePhis`, `simplifyNonSSAIR`, and final C-like variable
  scope correction for direct NVVM.
  Rationale: LLVM and NVVM natively consume SSA. Even though this slice rejects block parameters,
  preserving them as the next honest unsupported shape prevents the consumer from depending on an
  accidental text-emitter representation.
  Date/Author: 2026-08-26, Codex.
  Revisit when: measured libNVVM behavior demonstrates a specific SSA restriction.

- Decision: append one all-or-nothing scalar-control-flow block to V2; do not add V3.
  Rationale: V2 explicitly permits append-only fields and this change does not alter existing
  function semantics, handles, pointer model, ownership, or getter negotiation. V3 is reserved for
  a breaking contract such as an opaque-pointer transition or table reset.
  Date/Author: 2026-08-26, Codex.
  Revisit when: an actual incompatible ABI requirement appears.

- Decision: add `emitIntegerBinary` with a fixed `ADD`/`SUB` enum,
  `emitIntegerSignedLessThan`, `emitBranch`, and `emitConditionalBranch` as the complete new V2
  prefix.
  Rationale: these four operations exactly cover the accepted branch. A generic binary operation
  avoids duplicate ownership/type checks for add and sub without publishing unimplemented
  arithmetic. Phi construction, constants, calls, and valued returns remain separately
  negotiable future fields.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the next scalar-control capability is designed.

- Decision: keep capability negotiation shape-dependent.
  Rationale: the frozen diagnostic prefix still suffices for the Slice 6 empty kernel, and the
  Slice 4 scalar-memory prefix suffices for straight-line write/copy. Only a branch requires the
  new complete prefix. Old valid providers therefore retain their published capabilities while a
  partial new prefix is rejected as malformed.
  Date/Author: 2026-08-26, Codex.
  Revisit when: provider packaging requires a deliberately stricter minimum.

- Decision: map `AddressSpace::UserPointer` kernel parameters to NVVM address space 1 and preserve
  access qualification in legality.
  Rationale: this is the canonical front-end spelling and the Slice 4 differential ABI. LLVM's
  typed pointer does not encode Slang `Read` versus `ReadWrite`, so the emitter must reject a store
  through a read-only parameter rather than losing that contract.
  Date/Author: 2026-08-26, Codex.
  Revisit when: Slice 8 establishes the complete address-space and memory-access matrix.

- Decision: do not emit NVVM required-thread-dimension annotations in this slice.
  Rationale: `.reqntid` is stronger than the current CUDA/NVRTC contract and is not required to
  launch these raw CUDA kernels. Any future choice to enforce exact dimensions is an orthogonal
  builder capability with its own compatibility and runtime tests.
  Date/Author: 2026-08-26, Codex.
  Revisit when: CUDA launch-shape policy is intentionally changed across both routes.

- Decision: validate the complete Slang IR subset before optional builder discovery and perform
  no semantic repair during emission.
  Rationale: unsupported float/vector/loop/call/phi shapes must report E52017 identically on every
  machine and create neither a builder module nor a libNVVM program. Builder capability mismatch
  is distinct and reaches E52016 only after the Slang shape is known to be supported.
  Date/Author: 2026-08-26, Codex.
  Revisit when: feature negotiation becomes a pre-link target capability.

## Outcomes and Retrospective

Slice 7 is complete. The direct route now preserves the raw `[CUDAKernel]` launch signature for
signed `i32` and device `Ptr<i32>` parameters, retains only the exact selected kernel, keeps SSA,
and lowers straight-line memory plus the constant-free `chooseScalar` CFG through the LLVM 14
provider. The append-only V2 control-flow prefix preserves the diagnostic and scalar-memory
prefixes; an exact old scalar provider still compiles copy/load/store, while a conditional program
requires the new complete prefix before module creation.

The final Release provider and Debug `slang-test` builds passed outside the sandbox. With
`SLANG_NVVM_BUILDER_PATH` set to the provider's `Release` directory, the complete NVVM unit prefix
passed 50/50 after formatting. It includes real LLVM verification/bitcode, CUDA 12.9 libNVVM,
NVRTC differential PTX, `ptxas` 12.9.86 acceptance for both routes at `sm_75`, and CUDA-driver
execution on an RTX 5090 (compute capability 12.0, driver 610.62). Both routes stored `37`, copied
`-17`, and produced `7`, `4`, `0`, and `-1`; the negative case distinguishes signed less-than from
unsigned comparison. Parameter widths matched as `[64, 32]`, `[64, 64]`, and `[64, 32, 32]`, and
both PTX routes retained the expected global load/store and signed-comparison semantics.

Established regressions passed after the final format/build: unsupported barrier 1/1, CUDA option
parse 1/1, emission routing/hash 2/2, default/explicit NVRTC sampler lanes 3/3, true NVRTC
pass-through 2/2, and the pre-existing CUDA runtime dispatch 1/1. The provider still exports only
`slang_getNVVMBuilderAPI_V1` and `slang_getNVVMBuilderAPI_V2`; `dumpbin /dependents` reports only
Windows system libraries and no process-visible LLVM DLL.

Pinned clang-format 17.0.6 changed-line formatting and new-header dry-run passed, as did
`git diff --check`; no generated PTX, cubin, or log artifact entered the diff. The final
helper/special-case inventory retains only producer-side pipeline policy, complete reject-only
preflight, layout-only `IRStructKey`, canonical `UserPointer -> AS1`, one-to-one maps, and strict
provider validation. The audit found no custom semantic equivalence, syntax reconstruction,
ignored edge operand, silent fallback, or downstream repair.

The honest hand-off is E52017 on `integer_constant`, `branch argument`, `loop`, and `call` before
builder discovery. Slice 8 will add signed-i32 constants, non-entry block parameters/branch
arguments as LLVM phis, and a finite loop. Direct calls/non-void helpers remain a separate Slice 9
ABI boundary.

## Context and Current Pipeline

Slice 6 calls `linkAndOptimizeIR` directly on the PTX `CodeGenContext`, validates one empty
zero-parameter compute entry, maps it through the LLVM 14 builder, and passes verified bitcode to
the existing NVVM downstream compiler. This is the correct orchestration boundary, but the shared
pipeline still assumes any PTX request will ultimately emit CUDA source in several places.

Consider `writeScalar`. The established CUDA-source path leaves this final semantic shape:

```text
func writeScalar : Func(Void, Ptr(Int, ReadWrite, UserPointer), Int)
{
block(destination, value):
    store(destination, value)
    return_val(void_constant)
}
```

Direct PTX currently runs `collectEntryPointUniformParams` and
`moveEntryPointUniformParamsToGlobalScope`, producing a synthetic `EntryPointParams` constant
buffer and a zero-parameter function. That shape is correct for targets whose host ABI supplies a
parameter block, but it is not the CUDA kernel-launch ABI. The producer must preserve the raw
function parameters before the NVVM validator sees them.

For `chooseScalar`, the desired target-independent CFG has one entry block, two arm blocks, and a
merge block:

```text
entry(destination, x, y):
    condition = cmpLT(x, y)
    ifElse(condition, trueBlock, falseBlock, mergeBlock)
trueBlock:
    sum = add(x, y)
    store(destination, sum)
    unconditionalBranch(mergeBlock)
falseBlock:
    difference = sub(x, y)
    store(destination, difference)
    unconditionalBranch(mergeBlock)
mergeBlock:
    return_val(void_constant)
```

`ifElse`'s `mergeBlock` operand records structured-control-flow information; LLVM conditional
branching needs only the true and false destinations. The arm terminators encode the actual CFG
edges into the merge. No executable constant or non-entry block parameter is present.

The existing V2 builder already owns i32 construction, typed address-space pointers, function
parameter lookup, aligned non-volatile loads/stores, structural blocks, void returns, kernel
annotation, and verified serialization. It cannot yet create arithmetic, comparisons, or branch
terminators. V2 is append-only and publishes coherent minimum sizes, so Slice 7 adds exactly one
new minimum after the Slice 4 scalar-memory prefix.

## Scope and Non-Goals

In scope:

- raw signed-i32 and device-i32-pointer CUDA kernel parameter ABI;
- selected conventional compute and `[CUDAKernel]` definition preservation;
- canonical `AddressSpace::UserPointer` to NVVM global address-space-1 mapping;
- aligned i32 load/store with `Read`/`ReadWrite` enforcement;
- signed i32 add, subtract, and less-than;
- parameterless non-entry blocks, `ifElse`, zero-argument unconditional branches, and void return;
- retention of SSA throughout the direct pipeline;
- one append-only V2 scalar-control-flow capability and strict partial-prefix rejection;
- full legality before builder/module creation and verifier diagnostics afterward;
- deterministic fake public-route tests, real same-source NVRTC/NVVM differential PTX,
  `ptxas`, and optional CUDA-driver runtime evidence;
- backward compatibility for Slice 6 empty and Slice 4 straight-line provider capabilities;
- default NVRTC, explicit NVRTC, true pass-through, cache-hash, loader, and diagnostics regressions;
  and
- durable design/ledger and working-log updates.

Non-goals:

- executable integer constants, multiplication, division, shifts, bitwise operations, casts, or
  unsigned comparisons;
- block parameters, phi nodes, branch arguments, loops, switches, break/continue, or recursion;
- direct calls, declarations/imports, function pointers, non-void functions, or valued returns;
- bool/i8/i16/i64/uint/half/float/double parameters or arithmetic;
- generic/local/shared/constant pointers, pointer arithmetic, GEP, address-space casts, allocas, or
  globals;
- system-value/varying parameters, thread/block IDs, barriers, atomics, or launch bounds;
- vectors, matrices, arrays, structs, resources, existentials, interfaces, autodiff, or libdevice;
- multiple selected entry points, OptiX, RDC/LTO, debug metadata, or source-level diagnostics;
- changing the default PTX route, public target enum, or provider export allowlist; and
- byte-for-byte PTX equality or register-count performance gates.

## Architecture and Invariants

`TargetProgram::shouldEmitNVVMDirectly()` remains the single effective representation query.
`linkAndOptimizeIR` owns canonical linked IR and must preserve the CUDA launch ABI without applying
CUDA-C++ representation transforms. `LinkedIR.entryPoints` remains the source of selected entry
identity and exact external name. The NVVM validator owns the complete accepted-subset proof. The
NVVM emitter only maps validated IR to opaque builder handles. `NVVMIRBuilder` owns ABI
negotiation/provider lifetime, the LLVM 14 module owns all returned handles, and
`NVVMDownstreamCompiler` remains the sole owner of architecture/options/libNVVM/PTX policy.

Pipeline policy uses the existing direct-NVVM query at each owning point:

1. Skip entry-uniform collection and movement for direct NVVM so raw launch parameters survive.
2. Run normal CUDA/Torch entry cleanup, then restore keep-alive only on exact selected entries.
3. Do not apply the CUDA-source immutable-load intrinsic rewrite.
4. Do not eliminate phis or run non-SSA simplification for direct NVVM.
5. Do not run final C-like variable-scope correction for direct NVVM.

These are producer-side representation choices, not a wholesale `PTX == CUDASource` condition.
Existing default/NVRTC targets keep their exact behavior.

The validator first inventories module globals, function signature/blocks, parameters, ordinary
instructions, terminators, operand types, branch destinations, and access qualifiers. It accepts
only the exact subset and records whether the program requires the diagnostic, scalar-memory, or
scalar-control-flow V2 minimum. It accepts layout-only `IRStructKey` only if the implementation
confirms the probed producer/consumer relationship; all other unrecognized semantic globals are
rejected. No builder load/module creation occurs until validation completes.

Lowering uses two passes over the selected function: declare the exact function and create every
LLVM block first, then select each block and emit its ordinary instructions/terminator. Maps from
canonical Slang IR types, values, and blocks to opaque builder handles are the only translation
state. The accepted mapping is:

| Slang linked IR | Builder/NVVM representation |
| --- | --- |
| `Void` | existing LLVM `void` |
| signed `Int` | existing signless LLVM `i32` |
| `Ptr(Int, Read/ReadWrite, UserPointer)` | existing `i32 addrspace(1)*` |
| entry-block `IRParam` | existing function parameter at the same ABI index |
| `load` / `store` | existing aligned non-volatile load/store, alignment 4 |
| signed `add` / `sub` | new integer binary op `ADD` / `SUB` |
| signed `cmpLT` | new LLVM `icmp slt`, returning `i1` |
| `ifElse` | new conditional branch to true/false blocks |
| zero-argument `unconditionalBranch` | new unconditional branch |
| `return_val(void_constant)` | existing `ret void` |

The structured `ifElse` merge operand is not an executable LLVM operand; actual successor edges
remain encoded by the arm branches. Branch arguments and destination block parameters are rejected
together, so no phi input is silently dropped. Stores through `Access::Read` are rejected even
though LLVM typed pointers do not carry that qualifier.

The new provider functions clear output handles first and fully validate before calling LLVM.
Binary operands must be same-context scalar integers of identical type and the enum must be
`ADD` or `SUB`. Signed less-than has the same operand rule and returns i1. Every branch requires a
live current unterminated block; destinations must be blocks in that same current function, and a
conditional value must be exactly i1. Failure inserts no instruction. No LLVM type/object,
allocator, exception, or ownership crosses the ABI.

V1 and both existing V2 minimum constants remain byte-for-byte frozen. A provider reporting the
old diagnostic or scalar minimum remains valid. A size strictly inside the new block is malformed;
at or above the new minimum every new function must be non-null. The host clamps future larger
tables to its known `sizeof`, exposes `supportsScalarControlFlow()`, sanitizes provider-written
outputs on failure, and includes the capability in builder identity/cache hashing.

`markFunctionAsKernel` remains unchanged. `[numthreads]` stays attached to linked metadata and
reflection but does not become `reqntid`; raw `[CUDAKernel]` sources retain arbitrary host launch
dimensions, matching CUDA source/NVRTC.

### Input-shape and special-case audit

Planned helpers/special cases:

- direct-NVVM pipeline-policy checks: survive as producer-side representation choices; each is
  backed by an exact before/after probe and default-NVRTC regression.
- supported-IR validator plus required-capability result: survives as the named legality boundary;
  it classifies but never repairs IR.
- `IRStructKey` metadata allowance: survives only if implementation reconfirms it is layout-only
  identity retained by raw parameter layout. Removing it must fail the scalar-parameter test.
- `UserPointer -> NVVM global AS1` mapping: survives as the established CUDA kernel ABI boundary,
  not a generic address-space fallback.
- selected-function/block/value maps: survive as one-to-one emission state and never search
  arbitrary operand graphs.
- builder enum dispatch: survives with only two published values and rejects every unknown value
  before mutation.

No custom IR/`Val` equivalence, syntax reconstruction, gathered-parameter unpacking, C-like
temporary reconstruction, arbitrary call-graph traversal, address-space guessing, placeholder
kernel, ignored branch argument, or silent default is permitted. During self-review, remove each
new helper/special case in turn where practical and run the smallest failing test to reconfirm its
owner.

## Interfaces and Dependencies

In `source/slang/slang-emit.cpp`, use `TargetProgram::shouldEmitNVVMDirectly()` to preserve raw
entry parameters and selected CUDA kernels, skip the CUDA-source immutable-load rewrite, retain
SSA, skip `simplifyNonSSAIR`, and skip final variable-scope correction. Do not change broad
`isCUDATarget` semantics for existing consumers.

In `source/compiler-core/slang-nvvm-ir-builder-api.h`, append the fixed-width binary-op enum and
four function-pointer types/fields after `emitStore`. Add
`SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE` through `emitConditionalBranch`. Keep V1,
the V2 getter, and existing minimum constants frozen. Update the strict-C probe in
`source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c` for field order/size visibility.

In `source/compiler-core/slang-nvvm-ir-builder.h/.cpp`, add strict coherent-prefix validation,
`supportsScalarControlFlow()`, wrappers for all four operations, output sanitation, and capability
identity. Existing scalar wrappers remain usable with old scalar-prefix providers.

In `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`, implement the four operations with LLVM 14
`CreateAdd`/`CreateSub`, `CreateICmpSLT`, `CreateBr`, and `CreateCondBr` after complete
same-module/context/current-function/unterminated-block validation. Populate only the existing V2
table; export lists remain unchanged.

In `source/slang/slang-emit-nvvm.cpp`, replace the empty-only validator/emitter with a named
supported-subset preflight and one-to-one type/value/block lowering. Rename the public internal
validator declaration in `slang-emit-nvvm.h` as appropriate. Reuse existing IR type/layout helpers
for i32 alignment if they express the exact invariant; otherwise keep the accepted i32 alignment
explicit and documented.

In `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`, extend the fake provider state/API, add
prefix/partial-prefix/operation/no-mutation tests, drive the three isolated ordinary-Slang sources
through the full fake route, and add real same-source NVRTC/NVVM/PTX/`ptxas` evidence. If runtime
coverage is implemented, factor the minimal dynamic CUDA-driver loader and RAII guards currently
private to `unit-test-coverage-cuda-runtime.cpp` into a small shared test utility and rerun that
existing coverage test; do not duplicate a second driver loader.

Update `tests/cuda/nvvm-unsupported-ir.slang` only to remove the stale Slice 6 empty-only wording;
the barrier should remain E52017 `call` before builder-module creation. Update
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md` with the frozen
Slice 7 boundary and measured evidence.

External real validation requires the independently built Release LLVM 14.0.6 provider, compatible
CUDA libNVVM/NVRTC, and optional matching `ptxas`; runtime additionally requires an NVIDIA driver
and device. Missing optional runtime prerequisites cause an explicit ignored test, while an
explicit broken builder path remains a failure.

## Milestones

1. Preserve canonical CUDA semantics without importing CUDA-C++ representation.
   Add narrow direct-NVVM policy conditions in `linkAndOptimizeIR`. A linked-IR/fake-route test
   proves raw parameter order/types and selected `[CUDAKernel]` survival. Regression probes prove
   default/explicit NVRTC still gather/legalize/emit exactly as before. Reconfirm that direct
   float/vector/loop/call shapes fail at their real canonical instruction rather than an accidental
   constant-buffer or deleted-entry shape.

2. Publish and harden one V2 scalar-control-flow capability.
   Append the four-operation block and new minimum, update host negotiation/wrappers/identity and
   the strict-C probe, and implement LLVM 14 operations. Tests cover old exact minima, partial
   prefix rejection, future larger tables, null functions, unknown enum, cross-module/type/block
   misuse, terminated blocks, failure-after-write sanitation, and no mutation on failure.

3. Lower the supported linked IR exactly once.
   Expand legality for i32/UserPointer params, load/store, add/sub/cmpLT, multi-block functions,
   if/else, branches, and void return. Declare types/function/parameters/blocks before bodies and
   emit verified bitcode. Fake tests assert the exact operation graph and capability checks;
   unsupported IR creates neither builder module nor libNVVM program.

4. Prove the public ABI and semantic PTX through both routes.
   Compile identical isolated `writeScalar`, `copyScalar`, and `chooseScalar` Slang sources through
   explicit NVRTC and NVVM. Normalize only stable semantics: entry names, parameter widths,
   global-memory instruction families, and arithmetic result behavior. Do not assert a literal PTX
   branch because either downstream optimizer may predicate/select. Require verifier success and
   matching-architecture `ptxas` acceptance for both routes when available.

5. Add runtime evidence without coupling production to a GPU.
   When the shared dynamic driver utility can be extracted narrowly, load each route's PTX, launch
   one thread, and compare write `37`, copy `-17`, and choose `(2,5)->7`, `(7,3)->4`,
   `(5,5)->0`. Ignore only absent driver/device/NVRTC/libNVVM prerequisites; never ignore an
   explicitly configured broken provider. If extraction proves materially larger than this slice,
   record the reason and keep runtime as a named follow-up rather than duplicating infrastructure.

6. Preserve established routes and hand off the next honest shape.
   Run empty-kernel, builder-diagnostic, loader/cache, option/routing/hash, NVRTC, pass-through,
   barrier, and CUDA runtime regressions. Inspect provider exports/dependencies, format, run
   `git diff --check`, perform the required helper/input-shape revert audit, update design/ledger,
   and record the first stable loop/phi/call failure for the next plan.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox, as required by repository instructions. Use
Windows-native tools from `C:\src\slang`.

Standalone provider and host builds:

```text
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release `
    --target slang-llvm-nvvm -- /m
cmake.exe --build --preset debug --target slang-test
```

Provider-independent fake and negative tests (exact names finalized during implementation):

```text
$env:SLANG_NVVM_BUILDER_PATH = $null
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarControlFlowAPI
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarControlOperations
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangScalarMemoryAndConditionalUseDirectPipeline
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangNegotiatesScalarControlFlowCapability
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
```

Real provider, differential PTX, assembly, and optional runtime:

```text
$env:SLANG_NVVM_BUILDER_PATH = `
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderBuildsScalarConditionalKernel
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangRealScalarDifferentialPTX
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangRealScalarPtxasAccepts
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangScalarRuntimeMatchesNVRTC
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Relevant established regressions:

```text
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangEmptyCompute
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmSlangBuilderIdentity
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection `
    tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection `
    slang-unit-test-tool/coverageCudaRuntimeDispatch
```

Acceptance requires:

- the three ordinary Slang sources crossing the public direct route with exact raw ABI types/order;
- fake proof that their exact builder graph and verified bitcode reach libNVVM;
- provider-independent E52017 before module creation for every named next-boundary shape;
- old V2 diagnostic/scalar providers retaining old capabilities and partial new prefixes failing;
- LLVM verifier success and preserved verifier diagnostics on failure;
- same-source NVRTC/NVVM PTX parameter widths and global-memory semantics;
- `ptxas` acceptance for both routes when present;
- equal runtime values on a machine with driver/device support, or a documented ignored result when
  those prerequisites are absent;
- unchanged empty-kernel, default/explicit NVRTC, true pass-through, option/hash/cache/loader, and
  barrier results;
- unchanged provider export allowlist and no process-visible LLVM DLL dependency;
- pinned clang-format success, `git diff --check`, and no generated PTX/cubin/log artifacts; and
- a completed self-review inventory showing no C++-shape repair, ignored edge operand, or silent
  fallback.

## Failure and Recovery

All production changes remain gated behind explicit `SLANG_EMIT_CUDA_VIA_NVVM`. Restoring the
five narrow pipeline policy sites, the four appended V2 fields, and the Slice 6 validator/emitter
returns cleanly to the prior boundary without changing default PTX. V1 and old V2 layouts are
immutable, so rebuilding an old provider remains a compatibility check rather than a recovery
operation.

Builder operations validate completely before mutation and module scope cleanup destroys every
LLVM object on all exits. Unsupported Slang IR fails before optional dependency use. An old but
valid provider remains usable for its prior capability; a program requiring the new block receives
the named builder incompatibility diagnostic and never reaches libNVVM. An explicit broken
`SLANG_NVVM_BUILDER_PATH` is never a skip or logical-name fallback. No direct failure retries
through NVRTC.

Keep generated CUDA/PTX/cubin files in test-owned temporary storage or ignored `build/` paths.
Do not remove `external/slang-binaries/`; it is unrelated untracked workspace state. Do not commit
this active ExecPlan.

## Artifacts and Hand-Off

Keep this plan current with exact commands, counts, diagnostics, rejected alternatives, provider
and CUDA tool versions, normalized ABI observations, optional runtime status, and the final
special-case/revert audit. Distill stable architecture into `docs/design/nvvm-backend.md`, test
status into `docs/design/nvvm-backend-capability-ledger.md`, and the implementation narrative into
the required five-part PR description.

The next bounded plan starts from the first canonical rejected shape: executable integer
constants plus non-entry block parameters/branch arguments (LLVM phis), then loops; direct calls
and non-void returns may join that plan only if their ABI stays independently demonstrable. It must
extend the builder explicitly and must not re-enable phi elimination or reconstruct C-like local
variables.
