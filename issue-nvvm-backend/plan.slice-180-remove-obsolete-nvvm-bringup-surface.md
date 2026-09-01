# Remove obsolete direct-NVVM bring-up surface

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, the compiler and tests use the current generic provider construction and typed
operation interfaces directly. Integer-, floating-point-, call-, phi-, and return-specific facade
methods that only adapt to those generic interfaces are gone, along with tests and fake-provider
code whose only purpose was to preserve the earlier bring-up API.

This is a behavior-preserving cleanup. A developer can observe it by searching
`source/compiler-core/slang-nvvm-ir-builder.h`: there is one current operation path rather than
parallel generic and legacy convenience paths. Frozen corpus v1 remains exactly 452 workloads/427
healthy-MVP references, discovery remains separate, and every old-correct workload stays correct.

## Progress

- [x] (2026-09-01) Converted the architecture review's first phase into this bounded ExecPlan.
- [x] (2026-09-01) Inventoried every convenience facade, fake callback, and test-only helper, including all
  production call sites and the test that currently claims ownership.
- [x] (2026-09-01) Migrated retained production uses to the generic construction/value-operation interfaces.
- [x] (2026-09-01) Deleted obsolete facade methods and consolidated descriptor tests into data-driven cases.
- [x] (2026-09-01) Validated, replayed both corpora, and documented the measured reduction.

## Surprises and Discoveries

The old `emitIntegerBinary` method rejected multiply even though the generic descriptor path
correctly accepts it. Two facade tests therefore encoded the adapter's narrower policy rather than
provider behavior. Removing those assertions was part of removing the obsolete contract, while
the generic unknown-operation and exact descriptor tests remain.

The discovery runner verifies the historical frozen artifact (`census.slice-146.tsv`), not the
latest replay table. Passing a latest table correctly fails its immutability audit because progress
has raised the historical score. Discovery replay therefore uses the frozen artifact while frozen
workload selection uses the latest table.

## Decision Log

- Decision: do not revise provider ABI 34 merely to remove host-side convenience methods.
  Rationale: the C ABI already exposes generic construction plus typed value, atomic, surface, and
  texture operations; the redundant surface is in the C++ facade and tests.
  Date/author: 2026-09-01, Codex.
- Decision: file splitting or renaming is not an acceptance result by itself.
  Rationale: this slice must delete duplicate contracts and their tests, not move the same semantic
  branches to another file.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Removed 23 public convenience methods and 21 duplicate implementations. The facade header fell
from 470 to 332 counted lines, its implementation from 1,230 to 897, and the emitter from 15,663 to
15,620. Test files grew from 23,032 to 23,289 lines because concise descriptor scaffolding now
states operand/result types explicitly; that code is test-local and exercises the one production
contract instead of duplicating it.

Provider ABI 34 is unchanged. Frozen corpus v1 remains exactly 452/427 at 417/417/417 O0/O3/both,
with 431 all-row correct results per direct mode and no old-correct regression. Discovery remains
82/72 at 72/72/72. The focused tests pass after removing the two obsolete facade-policy assertions.

## Context and Current Pipeline

The direct emitter calls `NVVMIRBuilder`, which wraps the forward-only C provider ABI in
`source/compiler-core/slang-nvvm-ir-builder-api.h`. ABI 34 already has generic `emitPhi`,
`emitCall`, `emitValueReturn`, and `emitOperation` callbacks. The C++ facade still exposes older
methods such as `emitIntegerMultiply`, `emitIntegerBitAnd`, `emitIntegerEqual`,
`emitIntegerSignedLessThan`, `emitIntegerPhi`, `emitIntegerCall`, and `emitIntegerReturn`.

Most of these names occur only in the facade implementation and their unit tests. The few direct
emitter uses of integer-specific call/phi/return helpers can use the generic operations because
type lowering already supplies exact provider handles. Keeping both routes makes the fake provider
and builder tests prove two host APIs for one provider behavior.

## Scope and Non-Goals

In scope are the C++ `NVVMIRBuilder` facade, its implementation, direct-emitter call sites, the
shared fake/support layer, builder unit tests, and table-driven consolidation of exact descriptor
coverage. Remove only methods proven redundant by complete repository search and focused revert
tests.

Out of scope are provider ABI changes, new shader support, semantic `IRGenericAsm` legalization,
helper ABI representation changes, changes to NVRTC, compatibility adapters, fixture-name checks,
and broad movement of the 16K-line emitter merely to improve file size.

## Architecture and Invariants

- The C provider ABI remains the only cross-library contract and remains revision 34.
- One typed descriptor plus operand list identifies each value/atomic/resource operation.
- Generic construction owns calls, phis, returns, blocks, loads, stores, and pointer operations.
- The fake provider records and validates the same current callbacks; it does not expose a second
  semantic API solely for old unit tests.
- A method survives only if a production caller needs semantics not expressible through the current
  generic interface. Record that concrete gap before retaining it.
- Unsupported-shape diagnostics still occur before builder discovery or provider mutation.

## Interfaces and Dependencies

Primary files are `source/compiler-core/slang-nvvm-ir-builder.h`,
`source/compiler-core/slang-nvvm-ir-builder.cpp`, `source/slang/slang-emit-nvvm.cpp`,
`tools/slang-unit-test/unit-test-nvvm-builder.cpp`, and
`tools/slang-unit-test/unit-test-nvvm-support.h`. The semantic catalog remains the single descriptor
source of truth.

Use Windows-native `git.exe`, `cmake.exe`, and `python.exe`. The Release provider is under
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`. All CMake builds and tests run outside the
sandbox, with `SLANG_NVVM_BUILDER_PATH` set to that directory.

## Milestones

1. Produce a checked inventory of every facade method, categorizing it as production-generic,
   production-special, or test-only. For every special method, identify the callback semantics the
   generic ABI allegedly cannot express.
2. Migrate integer-specific phi/call/return production uses to `emitPhi`, `emitCall`, and
   `emitValueReturn`, retaining exact lowered types and diagnostics.
3. Remove facade-only arithmetic/comparison wrappers and replace per-method tests with descriptor-
   table tests of `emitValueOperation`. Remove matching fake helpers rather than leaving dead test
   entry points.
4. Run a self-review inventory for every retained helper. Revert one representative deletion to
   prove the generic path, and confirm no provider C callback or ABI metadata changed.
5. Regenerate frozen/discovery evidence, measure production and test surface before/after, update
   durable design documentation and the five-part report, then commit with subject `slice 180`.

## Validation and Acceptance

Build outside the sandbox:

```powershell
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release
cmake.exe --build build --config Release --target slang-unit-test slangc slang-test
```

Run the affected builder/emitter tests, then:

```powershell
$env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Release/bin/slang-test.exe -category nvvm
```

Replay frozen v1 and discovery separately:

```powershell
python.exe issue-nvvm-backend/run-compute-census.py --output build/nvvm-census/slice180 --workload-ids-from issue-nvvm-backend/census.slice-179.tsv --jobs 8
python.exe issue-nvvm-backend/summarize-compute-census.py --input build/nvvm-census/slice180/results.tsv --table issue-nvvm-backend/census.slice-180.tsv --clusters issue-nvvm-backend/census.slice-180-clusters.json
python.exe issue-nvvm-backend/run-compute-discovery.py --frozen-v1 issue-nvvm-backend/census.slice-179.tsv --output build/nvvm-discovery/slice180 --jobs 8
python.exe issue-nvvm-backend/summarize-compute-discovery.py --input build/nvvm-discovery/slice180/results.tsv --selection build/nvvm-discovery/slice180/selected-workloads.tsv --frozen-v1-clusters issue-nvvm-backend/census.slice-180-clusters.json --table issue-nvvm-backend/discovery-census.slice-180.tsv --clusters issue-nvvm-backend/discovery-census.slice-180-clusters.json
```

Summaries must report frozen and discovery denominators separately. Acceptance requires
417/417/417 or better over frozen v1's unchanged 427 healthy-MVP denominator, 72/72/72 or better
over the current 72 healthy discovery rows, no old-correct regression, unchanged provider ABI
metadata, changed-line clang-format 17, and `git diff --check`.

## Failure and Recovery

If a convenience method owns concrete behavior absent from the generic callback, stop deletion of
that method and record the exact caller, descriptor, and provider gap. Do not preserve neighboring
wrappers automatically. All edits are host-side and independently revertible; NVRTC and the C ABI
must remain untouched.

## Artifacts and Hand-Off

Keep transient logs below `build/nvvm-census/slice180-*`. Commit the completed plan with the
implementation, regenerated corpus TSV/JSON artifacts, durable architecture/capability updates,
and five-part report. The report must include the deleted/retained API inventory and before/after
production and test line counts. Hand Slice 181 one current generic facade with no compatibility
surface to preserve.
