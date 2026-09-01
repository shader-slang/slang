# Legalize fixed direct-NVVM value intrinsics by semantic identity

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, every operation in the fixed typed value-operation catalog carries semantic
identity from its standard-module producer. NVVM legalization replaces those tagged CUDA
`IRGenericAsm` terminators with one typed `IRNVVMIntrinsic`; the CUDA spelling is not copied. The
catalog contains provider types and semantics only, with no producer text.

The initial inventory proved that recipe, resource, and atomic GenericAsm families need richer
typed payloads than a value-operation ID. They remain on their existing exact recognizers in this
bounded slice rather than introducing a universal escape opcode. Frozen and discovery correctness
must not regress.

## Progress

- [x] (2026-09-01) Converted the architecture review's semantic-intrinsic phase into this ExecPlan.
- [x] (2026-09-01) Inventoried fixed catalog, recipe, resource, atomic, and compound families and
  identified the stable producer boundary.
- [x] (2026-09-01) Proved an optional intrinsic-assembly semantic tag survives specialization while
  ordinary CUDA emission retains its established text.
- [x] (2026-09-01) Migrated all 72 fixed catalog rows across 105 standard-module producer cases and
  removed the catalog's CUDA text field and fallback recognizer.
- [x] (2026-09-01) Replayed both corpora, recorded the remaining text-owned families, and
  documented the unchanged coverage and provider ABI.

## Surprises and Discoveries

Final linked-IR dumps showed that target specialization removes declaration-level target-intrinsic
decorations and retains only the selected GenericAsm body. A statement-owned semantic decoration
does survive that boundary. The legalizer can therefore consume it without consulting function
names, source locations, or CUDA text.

A single value-operation ID is sufficient for catalog intrinsics because the complete specialized
function signature supplies the exact overload. It is insufficient for texture components,
surface shapes, atomic memory roles, and multi-step wave recipes. Those families must gain their
own typed producer payloads in later bounded work; packing arbitrary data into this opcode would
recreate GenericAsm under another name.

## Decision Log

- Decision: prefer existing core Slang IR operations over a generic NVVM escape instruction.
  Rationale: arithmetic, casts, atomics, image operations, and many wave operations already have
  semantic IR forms that all optimization and validation passes understand.
  Date/author: 2026-09-01, Codex.
- Decision: allow a small typed NVVM-specific operation only for a concrete semantic that cannot be
  expressed correctly through existing IR.
  Rationale: the provider boundary is target-specific, but a universal escape opcode would recreate
  the current unstructured text problem with an enum payload.
  Date/author: 2026-09-01, Codex.
- Decision: migrate the fixed value-operation catalog atomically and stop before richer families.
  Rationale: the prototype disproved the assumption that one semantic payload correctly represents
  every accepted family. This still removes the broadest shared text table while preserving one
  representation per migrated operation and avoiding a universal escape instruction.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Implementation and broad replay are complete. The selected representation is
an optional producer tag lowered as `IRNVVMSemanticDecoration`, then consumed by the NVVM legalizer
to create `IRNVVMIntrinsic(semantic-id)`. All 72 fixed catalog rows are text-free. Scalar Half
promotion remains a typed recipe keyed by the same semantic ID and complete helper signature.

Frozen corpus v1 remains 418/418/418 O0/O3/both over 427 healthy references, with no old-correct
loss. Discovery remains 72/72/72 over 72 healthy references. The selected prefix passes 437/437
and the permanent NVVM category passes 92/92. Provider ABI revision 34 is unchanged.

## Context and Current Pipeline

`StmtLoweringVisitor::visitIntrinsicAsmStmt` currently produces `IRGenericAsm` from an assembly
string and lowered arguments. The direct emitter later recovers meaning through exact spelling and
signature checks beginning with `_findNVVMGenericAsmSemantic` and the surface, texture, scalar,
atomic, compound-wave, masked-wave, and aggregate-wave resolvers in
`source/slang/slang-emit-nvvm.cpp`. Preflight resolves them once to collect requirements, validates
their operands again, and emission resolves them a third time.

The shared `slang-nvvm-semantic-catalog.h` correctly centralizes typed provider descriptors, but its
`genericAsm` spellings couple provider semantics to CUDA source syntax. The intended boundary is a
semantic descriptor with no producer text.

## Scope and Non-Goals

In scope are producer-side semantic identity, the Slice-181 NVVM legalizer, every fixed
value-operation catalog family, the smallest justified typed NVVM operation representation,
semantic-catalog separation, and focused direct/fake/real-provider coverage.

Out of scope are richer texture/surface/atomic/compound-wave semantic payloads, support for
currently unsupported device clock, advanced reconvergence, or half2 atomic semantics merely
because they are GenericAsm; source/fixture-name matching; arbitrary CUDA parsing; and provider ABI
revision without a proven operation that the generic interface cannot express.

## Architecture and Invariants

- Semantic identity originates at a declaration/IR producer and survives linking, specialization,
  cloning, and inlining without relying on a printed name or source location.
- NVVM legalization rewrites semantic operations before preflight.
- Ordinary recipes become ordinary IR control flow and scalar/vector instructions, allowing normal
  simplification and dominance validation.
- Provider descriptors remain typed and exact; the semantic catalog contains no CUDA source text.
- A true target operation has one typed representation and one emitter mapping.
- Migrated catalog operations never consult GenericAsm text; untagged recipe/resource families
  retain their exact established representation until they gain an adequate typed payload.
- The typed provider catalog has no producer spelling and one source of truth per overload.

## Interfaces and Dependencies

Likely files include the canonical intrinsic producer in `source/slang/slang-lower-to-ir.cpp`, IR
instruction/decorations only if the prototype proves they are required, standard modules carrying
the semantic declaration, `source/slang/slang-ir-nvvm-legalize.*`,
`source/compiler-core/slang-nvvm-semantic-catalog.h`, `source/slang/slang-emit-nvvm.cpp`, and the
NVVM unit-test files.

The current provider ABI is revision 34. Existing generic builder operations must be preferred.
Changing an IR instruction definition requires regeneration through the repository's established
IR generation build path; do not edit generated stable-name output without its source definition.

## Milestones

1. Build a complete inventory table: exact producer declaration/function, selected GenericAsm,
   specialized signature, current resolver, equivalent core IROp or irreducible target semantic,
   provider descriptor, and tests. This table is the deletion checklist.
2. Prototype semantic identity with one representative family that survives specialization. Emit
   it as an existing core IROp, dump pre/post-link and pre/post-NVVM-legalization IR, and prove the
   emitter does not read assembly text. Promote only if identity is stable and no alternate
   representation remains; otherwise delete the prototype and record why.
3. Migrate ordinary scalar/libdevice, conversion, fixed wave, execution-register, and barrier
   catalog families. Retain Half promotion as a typed recipe selected by semantic ID and signature.
4. Remove the catalog spelling field and its GenericAsm fallback. Confirm richer recipe/resource
   families remain explicit follow-up inventory rather than silently sharing the value opcode.
5. Replay both corpora, document the before/after inventory, and commit with subject `slice 182`.

## Validation and Acceptance

The prototype must retain IR dumps demonstrating identity before and after specialization and must
have a negative near-match test. After promotion, run outside the sandbox:

```powershell
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release
cmake.exe --build build --config Release --target slang-unit-test slangc slang-test
$env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Release/bin/slang-test.exe -category nvvm
python.exe issue-nvvm-backend/run-compute-census.py --output build/nvvm-census/slice182 --workload-ids-from issue-nvvm-backend/census.slice-179.tsv --jobs 8
python.exe issue-nvvm-backend/summarize-compute-census.py --input build/nvvm-census/slice182/results.tsv --table issue-nvvm-backend/census.slice-182.tsv --clusters issue-nvvm-backend/census.slice-182-clusters.json
python.exe issue-nvvm-backend/run-compute-discovery.py --frozen-v1 issue-nvvm-backend/census.slice-146.tsv --output build/nvvm-discovery/slice182 --jobs 8
python.exe issue-nvvm-backend/summarize-compute-discovery.py --input build/nvvm-discovery/slice182/results.tsv --selection build/nvvm-discovery/slice182/selected-workloads.tsv --frozen-v1-clusters issue-nvvm-backend/census.slice-182-clusters.json --table issue-nvvm-backend/discovery-census.slice-182.tsv --clusters issue-nvvm-backend/discovery-census.slice-182-clusters.json
```

Also run focused semantic-legalization, fake-recording, real-provider PTX, and representative
runtime tests before the broad gates.

Replay frozen v1 and discovery separately. Acceptance requires unchanged denominators, no old-
correct regressions, no assembly-text lookup for migrated catalog operations, no `genericAsm` field
in the typed provider catalog, deterministic diagnostics for unsupported residual GenericAsm,
successful O0/O3 PTXAS on representative scalar/wave/atomic/resource workloads, changed-line
clang-format 17, and `git diff --check`.

## Failure and Recovery

If no stable semantic identity survives specialization, discard the prototype and fix the
producer/copying boundary rather than falling back to text or names. If one operation cannot be
expressed in core IR, isolate it, prove the semantic gap, and add one typed target representation;
do not generalize prematurely. Keep migrations family-atomic so a failed family can be reverted
without restoring two accepted representations for completed families.

## Artifacts and Hand-Off

Keep identity traces, pre/post-legalization IR, PTX, cubins, and timing samples below
`build/nvvm-census/slice182-*`. Commit the completed plan, implementation, generated IR-definition
updates, both corpus artifacts, durable design/capability updates, and five-part report. Hand Slice
183 a text-free fixed semantic catalog plus the inventory of richer GenericAsm and type/ABI shims.
