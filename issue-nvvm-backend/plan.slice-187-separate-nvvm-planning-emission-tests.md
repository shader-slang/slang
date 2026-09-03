# Slice 187: Separate NVVM planning, emission, and test-support responsibilities

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds.

## Purpose and Observable Result

Turn the ownership boundaries established by Slices 184-186 into economical source and test
boundaries. The module emitter consumes plans; canonical planning/classification and reusable
recipe implementation live behind named internal interfaces; non-template test-support
implementation no longer forces every NVVM test translation unit to compile a giant header.

## Progress

- [x] 2026-09-03: Measured dependencies and selected boundaries that do not duplicate representations.
- [x] 2026-09-03: Extracted production responsibilities without changing semantics.
- [x] 2026-09-03: Audited test-support ownership and rejected a state-sharing extraction.
- [x] 2026-09-03: Built, tested, ran corpus validation, and compared source-size/rebuild evidence.
- [x] 2026-09-03: Completed self-review and durable documentation.

## Surprises and Discoveries

- The 16,072-line test-support header deliberately gives every decomposed test translation unit
  independent anonymous fake-provider state. Tests directly inspect that state, so moving ordinary
  definitions wholesale to one `.cpp` file would change fixture semantics.
- The production emitter fell from 16,232 to 16,161 lines and its entry header from 242 to 24 lines.
  The dedicated plan files contain 346 lines; the value is ownership clarity, not net LOC removal.

## Decision Log

- 2026-09-03, Codex: Split only along demonstrated ownership and dependency seams. Do not create
  files merely to reduce the line count, and do not duplicate private classifier types.
- 2026-09-03, Codex: Extract the immutable plan schema and checked source index as one coherent
  production boundary. Keep fake-provider state per translation unit until tests own explicit
  fixture contexts; line-count reduction does not justify shared mutable state.

## Outcomes and Retrospective

The plan schema and checked source index now have one named production owner. Emission initializes
one index before provider mutation and uses typed family lookups. The test-support header remains
intentionally header-only with its isolation contract documented. Build, 437/437 selected tests,
92/92 category tests, frozen 418/418/418, and discovery 72/72/72 pass with no classification change.

## Context and Current Pipeline

`slang-emit-nvvm.cpp` currently combines canonical classifiers, capability planning, operand
validation, and provider emission. The NVVM tests are split by role, but
`unit-test-nvvm-support.h` contains substantial shared implementation. After plan migration,
semantic classification no longer needs to occur in the emission walk, enabling a real boundary.

## Scope and Non-Goals

No feature support, new IR shapes, provider ABI changes, diagnostic weakening, corpus
reclassification, or broad rewrite. Preserve semantic test cases even if their LOC remains high.

## Architecture and Invariants

There remains one source of truth for every classifier and representation. File-local helpers are
extracted only when their dependency closure is coherent. Test declarations/templates remain in
the header; ordinary definitions move to one support translation unit registered in CMake.

## Interfaces and Dependencies

Internal headers may be added under `source/slang/` and `tools/slang-unit-test/`. Update the owning
CMake source list only as needed. No public API or provider ABI change.

## Milestones

1. Build a dependency inventory and select the smallest coherent production extraction.
2. Extract implementation and preserve anonymous/internal linkage where possible.
3. Extract non-template test support and table-drive demonstrably repeated assertions.
4. Validate behavior and record before/after structural measurements.

## Validation and Acceptance

Build/tests run outside the sandbox. Require the full NVVM unit/category gates and both corpora.
Source searches must show a single definition for each moved helper and no new fallback. A clean
incremental rebuild must link all NVVM test translation units.

## Failure and Recovery

Because the slice is semantic-preserving movement, failures should localize to missing includes,
linkage, or CMake registration. Move only one coherent boundary at a time and keep the tree
buildable between milestones.

## Artifacts and Hand-Off

Record structural measurements and validation in the Slice 187 report/design ledger. Keep this
active plan uncommitted.
