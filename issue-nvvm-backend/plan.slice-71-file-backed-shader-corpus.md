# Establish a file-backed direct-NVVM shader corpus

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, successful direct-NVVM compilation is exercised by ordinary `.slang` files under
`tests/cuda`, not only by source strings embedded in NVVM unit tests. The files form a small
capability ladder over raw scalar code, core CUDA execution/shared memory, and mixed numeric types.
The existing conventional-compute corpus is sampled through the same direct route, its first
unsupported canonical IR shapes are recorded, and Slice 72 has one evidence-backed target shader.

## Progress

- [x] (2026-08-28) Confirmed that all successful end-to-end direct-NVVM shaders are currently C++
  string fixtures while the only file-backed direct-NVVM shader is a negative test.
- [x] (2026-08-28) Probed `tests/cuda/compile-to-cuda.slang`; direct NVVM stops at the canonical
  `entry-point parameter` boundary before provider mutation.
- [x] (2026-08-28) Added a three-file positive `.slang` capability ladder using the ordinary
  `slang-test` pipeline.
- [x] (2026-08-28) Repaired the misleading negative coverage and recorded a four-shader
  existing-suite blocker census in the durable design and capability ledger.
- [x] (2026-08-28) Rebuilt Release `slang-test`/`slang-unit-test`, passed the focused 4/4 file set
  and full 332/332 NVVM prefix, ran formatting/whitespace checks, and completed the audit.

## Surprises and Discoveries

- The comment in `tests/cuda/nvvm-unsupported-ir.slang` still attributes rejection to a void
  barrier helper. Slice 66 supports that helper; the current conventional entry instead reaches
  the later missing-raw-`CUDAKernel` boundary. The generic E52017 check allowed the stale rationale
  to survive.
- Successful integration sources intentionally use raw `[CUDAKernel]` entry points and raw launch
  parameters. Most existing CUDA/compute suite shaders use `[numthreads]`, system semantics, and
  conventional global parameters, so file ownership and conventional ABI support are separate
  gates.
- Three conventional tests (`compile-to-cuda`, `cuda-layout`, and
  `wave-lane-index-multidim`) stop first at `entry-point parameter`. The no-entry-parameter
  `sampler-comparison-state-unused` shader proceeds farther and stops at `get_field_addr` in its
  conventional global-parameter/resource graph.
- Diagnostic annotations compare normalized messages and omit the `error[E52017]` prefix. The
  repaired negative test therefore asserts the complete shape-specific message rather than only
  the numeric code.
- `extras/formatting.sh` cannot run on this machine because its WSL environment lacks `gersemi`,
  `clang-format`, `prettier`, and `shfmt`. This slice changes only Markdown and `.slang` files;
  those files pass the repository parser/tests and `git diff --check`.

## Decision Log

- Decision: add positive file tests before broadening semantics.
  Rationale: the ordinary test runner must become the compatibility signal that prioritizes future
  coherent capability bundles; more embedded strings would not establish that feedback loop.
  Date/author: 2026-08-28, Codex.
- Decision: retain the existing embedded integration sources in this slice.
  Rationale: unit tests can run from packaged build trees where the source checkout is absent.
  Loading repository files would add an accidental runtime dependency; generating embedded copies
  is a separate build-system decision. The new files remain compact behavioral tests rather than a
  second exhaustive runtime matrix.
  Date/author: 2026-08-28, Codex.
- Decision: do not add expected-failure entries for the compatibility census.
  Rationale: unsupported shaders are planning evidence, not permanent accepted failures. Only
  passing positive tests enter the registered suite.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Three positive file-backed shaders now cover scalar program structure, CUDA execution/shared
memory, and mixed numeric/vector support through normal `slang-test` discovery and direct PTX
FileCheck. Together with the corrected negative file they pass 4/4. No compiler or builder API was
changed, and the post-build established NVVM prefix remains 332/332, including the real provider,
NVRTC differential, `ptxas`, and GPU runtime lanes.

The existing-suite sample establishes that file ownership was the immediate test-infrastructure
gap, while conventional compute entry/global-parameter ABI is the next semantic gap.
`tests/cuda/compile-to-cuda.slang` is the first Slice 72 acceptance target. The sampler probe shows
that resource-field addressing follows closely, so Slice 72 must audit the complete canonical
entry/global graph rather than special-case the first parameter diagnostic.

## Context and Current Pipeline

`slang-test` parses directives in a `.slang` file and invokes `slangc` with the file plus the
declared target options. `-target ptx -emit-cuda-via-nvvm` enters the same linked-IR validation,
builder, compatible NVVM IR assembly, libNVVM, and PTX path exercised by
`unit-test-nvvm-integration.cpp`. Today the positive integration tests call
`loadModuleFromSourceString` on constants in `unit-test-nvvm-support.h`, so source-file discovery,
directive routing, and FileCheck of direct PTX are not covered by them.

The first existing-suite probe is `tests/cuda/compile-to-cuda.slang`. Its conventional compute
entry produces a canonical entry-point parameter outside `isNVVMSupportedParameterType`, and
`_validateNVVMFunction` diagnoses `entry-point parameter`. That is an intentionally unsupported
producer shape, not malformed IR to patch in emission. Slice 72 must define the conventional CUDA
compute/global-parameter ABI upstream and then teach this consumer that exact canonical form.

## Scope and Non-Goals

In scope are a small positive file-backed shader ladder, precise negative diagnostics, a durable
corpus/blocker table in `docs/design/nvvm-backend.md`, and the current capability ledger. Out of
scope are new IR operations, conventional entry/global-parameter support, changes to render-test,
mass expected-failure lists, duplicating every embedded fixture, or claiming broad suite support.

## Architecture and Invariants

Every positive file uses only the accepted raw `[CUDAKernel]` contract and established canonical
IR. It compiles to PTX through the explicit direct method and checks stable semantic PTX evidence,
not optimizer-sensitive instruction sequences. Existing NVRTC directives remain unchanged.

The corpus census records the first consumer boundary reached by optimized linked IR. It must not
add emitter guards, alternate spellings, or source rewrites. A file becomes an enabled direct-NVVM
test only after it passes; unsupported files remain ordinary existing tests with no new directive.

## Interfaces and Dependencies

No public or builder interface changes are expected. Tests require the already-built
`slang-llvm-nvvm` provider and discoverable CUDA toolkit used by the established Release NVVM
prefix. File directives select `cuda_sm_7_0` explicitly to avoid toolkit-dependent architecture
defaults.

## Milestones

1. Complete: add raw scalar/control-flow, core execution/shared-memory, and mixed-numeric `.slang` positives
   with stable PTX FileCheck assertions.
2. Complete: make the negative test assert its real canonical rejection shape and correct its
   explanation.
3. Complete: probe a bounded set of existing CUDA/compute shaders and distill the first-blocker table plus
   the chosen Slice 72 acceptance shader into the durable design.
4. Complete: run focused `slang-test` paths, the complete Release NVVM unit prefix, formatting, vocabulary
   and representation audits, then commit the plan and implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires every new `.slang` test to
compile through direct NVVM and match its PTX evidence, the negative test to name the actual shape,
the full `slang-unit-test-tool/nvvm` prefix to remain green, NVRTC/default routes to remain
unchanged, `git diff --check` to pass, and no generated PTX or `external/slang-binaries/` content to
be staged.

## Failure and Recovery

If a proposed positive file exposes a new unsupported operation, simplify it to an already-proven
representative rather than widening the backend incidentally. Record the rejected candidate and
first diagnostic as Slice 72+ evidence. If direct file tests cannot locate the provider in the
normal test environment, fix test/provider discovery consistently with the unit suite rather than
embedding an absolute local path.

## Validation Evidence

- `cmake --build build --config Release --target slang-test slang-unit-test --parallel 8` completed
  successfully outside the sandbox.
- `build\Release\bin\slang-test.exe tests/cuda/nvvm-raw-scalar
  tests/cuda/nvvm-core-execution tests/cuda/nvvm-mixed-numeric
  tests/cuda/nvvm-unsupported-ir` passed 4/4 after the rebuild.
- `build\Release\bin\slang-test.exe -use-test-server -server-count 8
  slang-unit-test-tool/nvvm` passed 332/332 after the rebuild.
- Direct `slangc` probes recorded exact first stops for the four existing shaders in the durable
  table. No probe output was staged.
- `extras/formatting.sh` was attempted outside the sandbox and reported the missing WSL tools
  listed above. `git diff --check` is clean.

## Self-Review

This slice adds no production helper, fallback, guard, or semantic special case. The three positive
files use already-canonical accepted raw CUDA shapes. The negative test checks the existing exact
contract rather than masking it. The corpus table reports valid unsupported producer shapes but
does not add expected failures or teach emission to reconstruct conventional parameters.

The only duplication is the intentional distinction between compact file-level compositions and
packaging-safe embedded unit/runtime fixtures. No AST/IR/`Val` equivalence, syntax reconstruction,
operand-graph walk, target-specific producer repair, or silent default was introduced.

## Artifacts and Hand-Off

Retain the positive file list, focused/full test counts, exact corpus-probe diagnostics, formatting
evidence, and self-review here. Distill the durable capability ladder and Slice 72 target into
`docs/design/nvvm-backend.md`, then commit this completed plan with Slice 71.
