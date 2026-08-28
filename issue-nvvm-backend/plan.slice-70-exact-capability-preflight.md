# Slice 70: Replace builder feature bits with exact capability preflight

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the compiler preflights the exact typed value operations required by linked IR.
The 49 `SLANG_NVVM_BUILDER_FEATURE_*` constants, synthesized feature words, legacy catalog adapters,
and per-feature facade methods are gone. Structural construction is guaranteed by the exact ABI.

## Progress

- [x] (2026-08-28) Established that current parameterized Slice 68 operations already bypass the
  feature bitset and preflight their exact descriptors.
- [x] (2026-08-28) Replaced feature collection/checking with an owned, deduplicated exact
  requirement representation populated from canonical linked IR.
- [x] (2026-08-28) Removed feature IDs, legacy catalog metadata/adapters, old operation enums, and
  feature-specific facade methods.
- [x] (2026-08-28) Consolidated capability coverage around complete descriptors and added a
  table-driven rejection-before-module test for representative descriptor families.
- [x] (2026-08-28) Built the provider and host unit-test target, ran focused coverage and the full
  332-test Release NVVM prefix, formatted the changed C/C++ files, and completed the audit.

## Surprises and Discoveries

- Slice 68's numeric families have no feature IDs; they already demonstrate the scalable design by
  resolving and checking complete typed descriptors before module creation.
- Structural feature bits describe callbacks that the new exact ABI requires unconditionally, so
  checking both duplicates the same contract at different abstraction levels.
- Consolidating the operation identifiers exposed an old `[8]` bound in fake-provider call-count
  storage. Current operation identifiers exceed that range, so the fake now sizes storage from
  `SLANG_NVVM_VALUE_OPERATION_COUNT` instead of encoding an obsolete assumption.
- Once the facade checks a complete descriptor before provider dispatch, an unsupported overload
  consistently returns `SLANG_E_NOT_AVAILABLE`; tests that expected a provider-specific failure
  now assert the public facade contract.
- `extras/formatting.sh` could not run its complete WSL toolchain because `gersemi`, WSL
  `clang-format`, `prettier`, and `shfmt` are unavailable on this machine. The pinned Windows
  clang-format 17 binary successfully formatted every changed C/C++ file, and `git diff --check`
  is clean.

## Decision Log

- Decision: value-operation support is keyed only by a complete operation descriptor.
  Rationale: operation, result type, and operand types are the capability boundary; bundled or
  per-overload integers lose information and require synchronized mappings.
  Date/author: 2026-08-28, Codex.
- Decision: exact-ABI structural callbacks are invariants, not semantic capabilities.
  Rationale: initialization validates the complete table before any emitter can run.
  Date/author: 2026-08-28, Codex.
- Decision: collect owned exact requirements during validation and deduplicate them before loading
  or mutating a provider module.
  Rationale: the canonical linked IR already supplies the operation and complete result/operand
  types; preserving that information avoids a second mapping and makes rejection atomic.
  Date/author: 2026-08-28, Codex.
- Decision: the direct backend emits LLVM 7-compatible NVVM IR 2.0 text only.
  Rationale: libNVVM consumes the compatible textual dialect established by the earlier experiment;
  retaining unused bitcode-format capability plumbing would create a false choice.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Slice 70 leaves one capability system: a complete typed value-operation descriptor. It removes all
49 builder feature constants, their synthesized words, legacy catalog adapters, old operation
enums, and feature-specific facade methods. Across the 13 implementation/test files, the change is
net -1,141 lines before this plan, despite adding exact requirement collection and table-driven
preflight coverage.

The table-driven preflight test covers signed-i32 multiply, float32 addition, wave lane index, and
parameterized signed-i8 addition. In each case the fake rejects only the requested descriptor and
the compiler returns before `createModule`, proving that unsupported semantics cannot partially
mutate provider state. The full Release NVVM prefix passes 332/332, including the real-provider,
PTX assembler, and available GPU coverage.

The final vocabulary audit found no builder feature constants/types, `LegacyFamily`, legacy catalog
adapter metadata, old operation enums, feature-specific facade support methods, or versioned V1-V4
builder code names. Uses of "legacy" that remain in the provider describe the required LLVM
7-compatible textual-dialect transformations, not builder ABI compatibility.

## Context and Current Pipeline

Linked-IR validation currently accumulates `NVVMIRFeatureSet`; after loading the builder, the host
compares it with feature words synthesized from current descriptor support. Parameterized numeric
families separately collect resolved descriptors and query each one. This is two parallel
capability systems for the same provider boundary.

## Scope and Non-Goals

In scope are linked-IR validation/preflight, semantic catalog metadata, facade capability methods,
provider and fake descriptor support, capability tests, durable documentation, and this plan.
Out of scope are new semantic families, looser provider support, and compatibility adapters.

## Architecture and Invariants

Canonical linked IR is the source of every requirement. Validation resolves each value-producing
instruction to a complete descriptor and deduplicates requirements without reconstructing types or
walking arbitrary operand graphs. Builder initialization guarantees the structural API. Every
required descriptor is checked before module creation; rejection performs no provider mutation.

## Interfaces and Dependencies

Remove `SlangNVVMBuilderFeature`, its bitset and constants, `legacyFeature`, `LegacyFamily`, legacy
operation codes, and `supportsFeature(s)`. Keep current `isOperationSupported` and `emitOperation`
as the single semantic operation contract. Make serialization-format support an exact foundation
invariant for this backend.

## Milestones

1. Complete: model and collect exact typed operation requirements from canonical linked IR.
2. Complete: remove feature and legacy adapter vocabulary from API, catalog, facade, emitter, and
   provider.
3. Complete: replace per-feature fakes/tests with descriptor-based and table-driven preflight tests.
4. Complete: rebuild, run focused and broad regressions, format, audit, and commit.

## Validation and Acceptance

Run focused descriptor positive/adjacent-negative tests, preflight rejection-before-module tests,
the Release NVVM prefix, compatible assembly, PTX/runtime tests available on this machine, and
formatting outside the sandbox where required. Accept if there are no builder feature constants or
legacy catalog adapters, every current workload still passes, and unsupported exact descriptors
fail before construction.

## Failure and Recovery

If a structural operation is not actually universal, give it a named semantic capability rather
than restoring a historical bit or nullable ABI field. If descriptor lifetimes complicate
collection, store an owned resolved descriptor shape instead of pointers into temporary arrays.
Never stage `external/slang-binaries/`.

## Validation Evidence

- `cmake --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --parallel 8`
  completed successfully; only the existing MSVC exception-flag override warnings remain.
- `cmake --build build --config Release --target slang-unit-test --parallel 8` completed
  successfully.
- Focused exact-support and preflight tests passed after copying the freshly built provider DLL
  into `build\Release\bin`.
- `build\Release\bin\slang-test.exe -use-test-server -server-count 8 slang-unit-test-tool/nvvm`
  passed 332/332 tests.
- The pinned Windows clang-format 17 executable formatted all changed C/C++ files, and
  `git diff --check` reports no whitespace errors.

## Self-Review

The new production helpers and special cases were inventoried before finalizing:

- `_requireValueOperation` survives. It copies and deduplicates exact descriptors produced from
  canonical linked IR; it neither defines a new IR equivalence nor repairs an alternative shape.
- The facade's exact-support guard survives. It enforces the provider boundary before dispatch and
  returns the documented unavailable result without changing AST or IR.
- The executable-work check survives. It replaces a feature-bit proxy with the direct fact needed
  by accepted CUDA IR and does not patch a malformed producer shape.
- `_rejectFakeNVVMBuilderValueOperation` is test-only instrumentation that rejects one owned exact
  descriptor so the no-mutation boundary can be verified.

No new helper reconstructs syntax from semantic state, walks arbitrary operand/substitution graphs,
or adds a custom equivalence over AST, IR, `Val`, `DeclRef`, or witnesses. The canonical linked IR
remains the source of truth, so no producer-side representation fix is indicated by this slice.

## Artifacts and Hand-Off

Retain the final feature/reference removal measurements, test consolidation numbers, validation
evidence, and self-review here. Commit this completed plan with Slice 70.
