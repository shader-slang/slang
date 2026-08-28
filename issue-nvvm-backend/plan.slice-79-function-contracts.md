# Preserve direct NVVM function contracts

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM emission preserves the externally observable contract of every
reachable function. Entry points and `[CudaDeviceExport]` helpers have external linkage, ordinary
helpers have internal linkage, and `[noinline]` on an ordinary device helper becomes LLVM's
`noinline` function attribute. A CUDA device export uses the exact source-level export name carried
by its canonical IR decoration.

The existing `tests/cuda/noinline.slang` gains a direct PTX lane that distinguishes an internal
no-inline helper, an ordinary internal helper that may be inlined, the externally visible exported
helper, and the visible entry point. CUDA `ptxas` must accept the resulting module.

## Progress

- [x] (2026-08-28) Rechecked the Slice 78 probe and traced the missing semantics through function
  collection, declaration, LLVM serialization, and CUDA-source emission.
- [x] (2026-08-28) Added one generic linkage enum and independent function-attribute flags to
  exact builder ABI revision 5, the facade, the real provider, and the fake provider.
- [x] (2026-08-28) Mapped canonical entry-point, device-export, and no-inline decorations in the
  direct emitter.
- [x] (2026-08-28) Added builder, fake-boundary, libNVVM, PTX, `ptxas`, and file-backed shader
  coverage.
- [x] (2026-08-28) Formatted, built, ran focused and complete validation, updated durable
  documents, and completed the self-review. The slice is ready to commit.

## Surprises and Discoveries

- Slice 78's direct `noinline.slang` probe retained every helper because the generated module had
  not optimized them away. Presence of a `.func` was therefore not evidence that
  `IRNoInlineDecoration` crossed the direct boundary.
- `slang-llvm-nvvm` currently creates every function with LLVM external linkage. Consequently,
  ordinary implementation helpers appear as `.visible .func`, unlike internal CUDA device
  helpers.
- Lowering records the exact source-level CUDA export name as operand zero of
  `IRCudaDeviceExportDecoration`. The existing direct name helper instead uses the ordinary Slang
  mangled name for every non-entry function.
- LLVM's NVPTX regression corpus directly exercises `noinline` functions and distinguishes
  internal `.func` from externally visible `.visible .func`; these are generic LLVM function
  properties rather than CUDA-specific builder operations.
- A static `SIMPLE` PTX test without an explicit output can request entry-point output and reach
  the CUDA/NVRTC path even when its command line includes `-emit-cuda-via-nvvm`. Adding `-o -`
  selects whole-target stdout output, which is the direct libNVVM path. Auditing the existing
  static lanes found five false-positive registrations; all now request whole-target output and
  pass against actual direct PTX.

## Decision Log

- Decision: replace the global-specific linkage enum with one `SlangNVVMLinkage`, and add
  independent `SlangNVVMFunctionFlags` to the existing function-declaration operation.
  Rationale: linkage and optimization constraints belong to the function definition and are
  independent dimensions. A shared enum is the single generic contract for function and global
  linkage, while extensible flags avoid callbacks named after Slang attributes. No old ABI adapter
  is retained.
  Revisit only if a supported LLVM/libNVVM property cannot be represented at declaration time.
  Date/author: 2026-08-28, Codex.
- Decision: take a CUDA device export's symbol from its canonical decoration operand, while all
  ordinary helpers retain `getMangledName` and the selected entry retains its entry-point name.
  Rationale: lowering is the producer of the requested external symbol. Reconstructing it from a
  name hint or source syntax would create a second source of truth.
  Date/author: 2026-08-28, Codex.
- Decision: ignore a no-inline decoration on the selected entry point but preserve it on ordinary
  helpers, matching `CUDASourceEmitter::emitFunctionPreambleImpl`.
  Rationale: a kernel is a call-graph root and has no caller into which it can be inlined. The
  source backend already defines the attribute as an ordinary-device-function contract.
  Date/author: 2026-08-28, Codex.
- Decision: make every static direct-PTX `SIMPLE` lane request whole-target output with `-o -`.
  Rationale: the direct backend emits one linked target module, whereas the default entry-point
  output request can take a different CUDA path. The directive must select the artifact whose
  contents its checks claim to validate.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Exact builder ABI revision 5 now preserves internal/external linkage and the independent no-inline
property. Both normal LLVM 14 assembly and LLVM 7-compatible text distinguish internal no-inline,
internal plain, external no-inline, and external kernel functions. The fake direct boundary sees
the exact same contracts and the source-level `exportedFunc` symbol from the canonical export
decoration.

`tests/cuda/noinline.slang` passes all three CUDA-source, ordinary PTX, and direct-libNVVM lanes.
Its direct PTX contains internal `.func` definitions for `helperFunc` and `plainHelper`, a
`.visible .func exportedFunc`, and `.visible .entry computeMain`. CUDA 12.9 `ptxas -arch=sm_70`
accepts that PTX. The audit of existing static direct lanes added explicit whole-target output to
five files and adjusted two order-dependent checks to describe the direct module without relying
on helper placement.

The Release host targets and standalone provider build pass. The complete NVVM unit-test prefix
passes 342/342, and the combined unit/file-backed run passes 354/354. Generated PTX and cubin
artifacts remain under ignored `build/` paths.

Self-review inventory: the exact export-name branch survives because the lowering decoration is
the canonical source of truth, and release assertions enforce its producer contract. The
entry/export linkage classification and no-inline mapping survive because they transfer canonical
IR properties directly at function declaration. There are no new graph walks, source-syntax
reconstruction paths, compatibility fallbacks, or target-named provider operations. A proposed
second function-linkage enum was rejected in favor of the single linkage contract now shared by
functions and globals. The test-harness `-o -` changes survive because they select the actual
whole-target artifact rather than compensating for emitted IR.

## Context and Current Pipeline

Consider the existing source:

```slang
[noinline]
int helperFunc(int x) { return x + 1; }

[CudaDeviceExport]
[noinline]
int exportedFunc(int x) { return x + 3; }

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = helperFunc(42) + exportedFunc(1);
}
```

`slang-lower-to-ir.cpp` attaches `IRNoInlineDecoration` and records `exportedFunc` on
`IRCudaDeviceExportDecoration`. Direct preflight in `slang-emit-nvvm.cpp` visits the finite call
closure rooted at `computeMain`, `_getNVVMFunctionName` selects symbols, and the declaration loop
calls `NVVMIRBuilder::declareFunction`. The provider currently creates every LLVM function with
`ExternalLinkage` and no attributes, so both IR decorations are discarded before the LLVM 7-era
text reaches libNVVM.

This slice fixes that consumer boundary. It does not walk uses, recover source modifiers, or infer
visibility from call count. The canonical function and its decorations supply the complete
contract at declaration time.

## Scope and Non-Goals

In scope are internal/external function linkage, the no-inline semantic flag, exact CUDA device
export names, selected-entry behavior, unknown-enum/unknown-bit rejection, LLVM 14 and LLVM 7-era
text serialization, libNVVM compilation, and the existing file-backed no-inline shader.

Out of scope are arbitrary external declarations, indirect calls, address-taken functions,
recursive calls, host and unselected CUDA kernels, inline hints, calling conventions, function
parameter attributes, richer helper types, and general keep-alive/linker policy. Those remain
their existing deterministic boundaries.

## Architecture and Invariants

The linked Slang IR is the source of truth. The selected entry is externally linked and marked as
an NVVM kernel. A reachable helper is externally linked only when it carries the exact CUDA device
export decoration; every other helper is internal. A no-inline flag is emitted only for a
non-entry helper with `IRNoInlineDecoration`.

Symbol selection follows the same classification. Entry points use the entry-point decoration's
name, CUDA device exports use the decoration's string operand, and ordinary helpers use their
canonical Slang mangled name. Empty or duplicate names fail semantic preflight before provider
discovery.

The builder validates linkage and all flag bits before mutating a module. The LLVM provider maps
the linkage enum directly to LLVM internal/external linkage and maps `NO_INLINE` to LLVM's
`NoInline` attribute. The generic ABI does not mention CUDA, Slang decorations, or PTX syntax.

## Interfaces and Dependencies

Revise `source/compiler-core/slang-nvvm-ir-builder-api.h` to exact ABI revision 5. Add
`SlangNVVMLinkage`, internal/external constants, `SlangNVVMFunctionFlags`, none/no-inline
constants, and both arguments to `SlangNVVMBuilderConstructionAPI::declareFunction`. Update the
C++ facade and every call site explicitly; there is no compatibility overload.

Update `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` and the fake provider in
`tools/slang-unit-test/unit-test-nvvm-support.h`. Update `source/slang/slang-emit-nvvm.cpp` to derive
one declaration contract from canonical IR. LLVM 14 supplies `GlobalValue` linkage and
`Attribute::NoInline`; the established LLVM 7-compatible textual writer and libNVVM compiler need
no new target-specific operation.

## Milestones

1. Revise the exact ABI/facade and update existing builder clients with explicit external linkage
   and no flags.
2. Make the real provider validate and apply internal/external linkage plus no-inline, with normal
   and NVVM IR 2.0 assembly tests and invalid-input coverage.
3. Record the contract in the fake provider and map entry/helper/export decorations and symbols in
   the direct emitter.
4. Add fake direct-source coverage proving declaration order, exact names, linkage, and attributes.
5. Register a direct `noinline.slang` lane, inspect LLVM assembly and PTX, run `ptxas`, execute the
   full NVVM regression set, update durable documents, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- the builder API rejects unknown linkage values and flag bits before module mutation;
- LLVM assembly contains `define internal ... noinline` for an ordinary constrained helper,
  ordinary internal linkage without the attribute for a plain helper, and an externally linked
  no-inline exported helper;
- LLVM 7-era text verifies and compiles through libNVVM;
- the fake direct provider observes internal/no-inline for `helperFunc`, internal/none for
  `plainHelper`, external/no-inline plus exact `exportedFunc` for the device export, and
  external/none for `computeMain`;
- direct PTX distinguishes `.func` from `.visible .func`, contains `.visible .entry computeMain`,
  and CUDA 12.9 `ptxas -arch=sm_70` accepts it;
- the Release host build, standalone provider build, focused tests, file-backed shader, and
  complete NVVM prefix pass;
- clang-format, `git diff --check`, and repository status checks pass; and
- `external/slang-binaries/` and generated artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects `noinline` or internal linkage in the compatible text, retain the generic
builder contract only long enough to isolate whether the text bridge changed the attribute or
whether libNVVM rejects the valid LLVM spelling. Do not fake no-inline by retaining every function
or by rewriting PTX.

If a device export decoration is absent from the final linked helper, inspect the lowering/linking
producer and fix preservation there. Do not infer an export from source names or other decorations.
All ABI, provider, emitter, test, and documentation changes are one forward-only slice and can be
reverted together.

## Artifacts and Hand-Off

Keep linked IR, LLVM 14 assembly, LLVM 7-era assembly, generated CUDA, direct PTX, and `ptxas`
evidence under ignored `build/` paths. Distill the function declaration contract, exact export
symbol rule, file-backed registration, remaining function boundaries, and final validation into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md`. Complete the
living sections and self-review inventory before committing this plan with Slice 79.
