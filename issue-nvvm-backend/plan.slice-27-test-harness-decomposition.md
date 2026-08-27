# Slice 27: Decompose the NVVM test harness without changing behavior

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires each
completed slice plan to ship with its implementation, so this plan will be committed with Slice 27.

## Purpose and Observable Result

After this slice, the approximately 26,000-line
`tools/slang-unit-test/unit-test-nvvm-compiler.cpp` no longer combines the fake LLVM builder, fake
libNVVM library, provider ABI tests, direct-emitter topology tests, real PTX comparisons, `ptxas`
checks, CUDA runtime checks, and downstream-compiler tests in one translation unit. The same 188
`slang-unit-test-tool/nvvm` test names pass with the same skip behavior and the production backend
is byte-for-byte unchanged.

This is a structural prerequisite for the V3 provider and type-lowering slices. It deliberately
does not table-drive or weaken existing assertions yet; moving and deduplicating behavior in the
same diff would make failures difficult to attribute.

## Progress

- [x] (2026-08-27) Audited the current monolith, its 188 registered NVVM tests, fake-provider
  region, shared compile/PTX/`ptxas`/runtime helpers, and downstream-compiler tail.
- [x] (2026-08-27) Chose behavior-preserving test decomposition as the first cleanup boundary.
- [x] (2026-08-27) Captured the exact baseline test-name set, focused 188/188 result, file sizes, and established
  preservation result before moving code.
- [x] (2026-08-27) Extracted shared support and fake-provider seams, then moved tests into focused translation
  units without changing assertions or test names.
- [x] (2026-08-27) Rebuilt and proved exact test-name-set preservation, focused behavior, preservation, and clean
  production diffs; update the design/ledger and complete this plan.

## Surprises and Discoveries

- Observation: the monolith contains several already-distinct seams.
  Evidence: fake builder storage begins near the top of the file; builder and libNVVM loaders,
  compile helpers, PTX classification, `ptxas`, and CUDA driver helpers precede separately grouped
  unit tests. `tools/CMakeLists.txt` creates `slang-unit-test` through `slang_add_target`, so new
  files in `tools/slang-unit-test` participate in the same module without a new public target.
  Consequence: this slice can be a test-only move with shared internal support and no production
  build-system contract change beyond any source discovery adjustment proven necessary by CMake.

- Observation: the accumulated fixtures deliberately lived in one anonymous namespace.
  Evidence: moving the tests to separate translation units while retaining a header-only
  `unit-test-nvvm-support.h` preserves internal linkage and gives each test owner an independent
  fake state. Release and Debug builds compile all four owners without exporting test-support
  symbols, and the focused suite remains 188/188.
  Consequence: Slice 27 does not invent a cross-translation-unit fake ABI merely to move code.
  Slice 30 may replace the large internal fixture with recorded generic operations after V3 exists.

- Observation: the source-level registered name set is exactly stable.
  Evidence: the sorted 188-name list hashes to
  `c197159202001f39765394b2399146398d0c4534803864b3ea44cc694827ac78` before and after the
  split, with 188 unique names in both states.
  Consequence: no capability-ledger key or focused-test selector changes in this slice.

## Decision Log

- Decision: split the test seams before changing the provider ABI or type lowering.
  Rationale: subsequent cleanup then touches the builder and emitter test owners independently;
  keeping the first slice behavior-preserving gives a reliable attribution boundary.
  Date/author: 2026-08-27, Codex.
  Revisit when: the test module cannot share internal helper symbols across translation units
  without changing production linkage or test registration.

- Decision: preserve every existing registered test name and assertion in this slice.
  Rationale: the capability ledger and accumulated slice evidence refer to those names, and test
  consolidation belongs to Slice 30 after the generic provider surface exists.
  Date/author: 2026-08-27, Codex.
  Revisit when: two names are proven aliases of the same registered function rather than distinct
  evidence; record any exception explicitly before changing it.

## Outcomes and Retrospective

Slice 27 replaces the 25,941-line, 1,146,356-byte monolith with four focused registered-test
owners plus one shared internal implementation header. `unit-test-nvvm-builder.cpp` owns 66 tests
in 9,672 lines; `unit-test-nvvm-emitter.cpp` owns 44 tests in 3,718 lines;
`unit-test-nvvm-integration.cpp` owns 52 tests in 3,917 lines; and the reduced
`unit-test-nvvm-compiler.cpp` owns 26 tests in 1,173 lines. The 7,474-line support header contains
no registered test. `slang_add_target` discovered the new sources through its existing configured
glob after CMake regenerated; no build file or production source changed.

The sorted test-name hash remains exact, the final Release focused run passes 188/188, and the
Debug preservation matrix passes 10/10. The split intentionally preserves the large historical
fake implementation rather than redesigning it before V3. Slice 28 can now change builder ABI
tests without mixing downstream/runtime test moves, and Slice 30 has an explicit support owner to
replace with recorded generic operations.

## Context and Current Pipeline

`unit-test-nvvm-compiler.cpp` currently owns both test infrastructure and all consumers of that
infrastructure. Its fake `SlangNVVMBuilderAPI_V2` records LLVM-like operations for direct-emitter
topology assertions. A separate fake libNVVM API tests downstream discovery and failure behavior.
Real helpers compile Slang through direct NVVM and NVRTC, classify PTX, invoke matching-toolkit
`ptxas`, and launch kernels through the CUDA driver. The registered tests then cover provider
negotiation, invalid operations, direct topology, capability gating, real differential PTX,
assembler acceptance, runtime equality, LLVM coexistence, loader behavior, and libdevice policy.

These are intentional layers, but their shared anonymous namespace and static storage make the
physical file the only ownership boundary. Slice 27 turns those conceptual layers into source
boundaries while keeping their observable behavior identical.

## Scope and Non-Goals

In scope are internal test-support headers/sources, moving existing tests, narrowly deduplicating
include lists and environment/tool discovery needed for the move, and documenting the new owner of
each fixture.

Out of scope are production source changes, a V3 provider, callback or capability changes, new
type support, new backend behavior, rewriting assertions, combining test names, parameterized
scalar cases, changing skip policy, and performance claims.

## Architecture and Invariants

Use focused internal files with one-way dependencies. The implementation retained the fixture
definitions as a header-only anonymous namespace so each owner keeps private state and no test ABI
is exported:

```text
unit-test-nvvm-support.h        private fixtures/compile/PTX/ptxas/runtime helpers
        |
        +-> unit-test-nvvm-builder.cpp         provider ABI/invalid-operation tests
        +-> unit-test-nvvm-emitter.cpp         fake direct-emitter topology/gating tests
        +-> unit-test-nvvm-integration.cpp     real PTX/ptxas/runtime tests
        +-> unit-test-nvvm-compiler.cpp        downstream compiler/loader/libdevice tests
```

Exact filenames may be adjusted if an existing repository convention provides a better match, but
the dependency direction must remain support-to-test. Shared support must not register tests or
own mutable singleton state that is unrelated to the fake APIs. Fake state resets must remain
explicit per test. No production header may include test support.

The name set obtained from `SLANG_UNIT_TEST(nvvm...)` before the move is the source of truth. A
test may move but must not silently disappear, duplicate, or change its environment gate.

## Interfaces and Dependencies

All new interfaces are private to the `slang-unit-test` module. Prefer small structs and functions
in namespace `Slang` or a nested NVVM-test namespace; do not expose them through `include/` or the
Slang DLL. Keep CUDA and platform headers in the smallest support source that needs them so builder
ABI tests do not acquire unnecessary runtime dependencies.

The production provider and host APIs, provider DLL export allowlist, NVVM artifact contract, and
NVRTC route remain unchanged. The build remains the existing Windows-native CMake build.

## Milestones

1. Record `git grep`/`rg` output for the 188 exact test names, line counts, focused Release result,
   and Debug preservation 10/10. Add a temporary comparison script or local text artifact if
   useful, but do not commit generated name lists.
2. Extract environment, artifact-compilation, PTX classification, matching-root `ptxas`, and CUDA
   runtime helpers into `nvvm-test-support.h/.cpp`. Preserve failure and skip semantics exactly.
3. Extract fake LLVM-builder and fake-libNVVM implementations into `nvvm-test-fakes.h/.cpp`. Give
   observation/reset APIs explicit ownership; do not redesign per-operation state in this slice.
4. Move provider/invalid-operation tests to `unit-test-nvvm-builder.cpp`, direct fake-emitter tests
   to `unit-test-nvvm-emitter.cpp`, and real differential/assembler/runtime tests to
   `unit-test-nvvm-integration.cpp`. Leave downstream compiler, loader, artifact, and libdevice
   tests in the reduced `unit-test-nvvm-compiler.cpp`.
5. Compare the registered name set before and after, run focused and preservation tests, inspect
   the test DLL and production diff, format changed files, and update durable documentation.

## Validation and Acceptance

Build `slang-unit-test` and `slang-test` Release outside the sandbox using Windows-native CMake.
Run `build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm`; all 188 established names must be
present and pass or skip exactly as before. Compare a sorted pre/post list of registered names.

Build the Debug targets outside the sandbox and run the established preservation matrix: parser
1/1, routing/hash 2/2, unsupported boundary 1/1, sampler 3/3, CUDA compile/pass-through 2/2, and
runtime dispatch 1/1. Run `git diff --check` and verify that no file outside test infrastructure,
this plan, and durable NVVM design/ledger documentation changed. Acceptance requires no production
binary/API diff and no new environmental requirement for tests that previously ran without CUDA.

## Failure and Recovery

The extraction is safe to perform one seam at a time. If a moved group fails to link, restore its
definitions to the original test source with `apply_patch`, identify the missing private interface,
and retry without broadening production visibility. If static initialization changes test order or
state, make reset/fixture lifetime explicit rather than relying on file order. Do not combine or
delete a failing test to make the name comparison pass.

Do not delete or stage the user's `external/slang-binaries/` directory. Temporary name manifests
and generated outputs must be removed before the completed plan and implementation are committed.

## Artifacts and Hand-Off

The retained evidence is the pre/post name hash
`c197159202001f39765394b2399146398d0c4534803864b3ea44cc694827ac78`, focused 188/188,
preservation 10/10, and the per-file counts recorded above. CMake reported its expected configured
glob mismatch once, regenerated, and then compiled all four translation units. Update
`docs/design/nvvm-backend.md` with the settled ownership and the capability ledger with unchanged
evidence. Commit this completed plan with Slice 27; leave Slice 28's plan uncommitted until its
implementation is complete.
