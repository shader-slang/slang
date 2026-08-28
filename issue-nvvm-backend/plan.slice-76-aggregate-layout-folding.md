# Fold CUDA layout queries before direct NVVM emission

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM route folds CUDA `__sizeOf`, `__alignOf`, and canonical
`__offsetOf` calls to signed 32-bit IR constants before runtime-subset validation. Layout-only
aggregate construction, field extraction, and helper functions then disappear through ordinary
dead-code elimination instead of widening the provider API with aggregate runtime values.

The existing `tests/cuda/cuda-array-layout.slang` gains a direct lane. It compiles and runs through
both CUDA/NVRTC and direct libNVVM, producing `48, 0, 16, 20, 44, 4, 0, 0`.

## Progress

- [x] (2026-08-28) Captured the final linked IR and identified the aggregate result/helper failure.
- [x] (2026-08-28) Traced the aggregate values to compile-time-only layout-query arguments.
- [x] (2026-08-28) Folded the exact type/value size and alignment forms plus canonical field
  offsets.
- [x] (2026-08-28) Removed the obsolete helper-emission special case and added positive/negative
  fake coverage.
- [x] (2026-08-28) Added the direct runtime lane, validated real PTX, updated durable records,
  completed the self-review, and prepared Slice 76 for commit.

## Surprises and Discoveries

- `cuda-array-layout.slang` reaches direct preflight with a default constructor returning
  `StructWithArray`, four `get_field` instructions, and five aggregate-parameter helpers. None of
  those aggregates contributes a runtime value: each exists only because the CUDA prelude spells
  value-form layout queries as `GenericAsm` helpers.
- The Slice 74 implementation folded only type-form scalar/vector queries inside the helper
  terminator. That still declares and calls one runtime helper per query, and it forces helper
  signatures through the runtime scalar subset.
- The shared CUDA layout rules already compute nested struct, array, matrix, and half layouts, and
  cache field offsets by layout-rule identity. Reusing them keeps layout policy out of the emitter.

## Decision Log

- Decision: fold layout queries at their call sites before direct runtime validation.
  Rationale: the result is compile-time metadata. The aggregate argument graph is an artifact of
  the prelude spelling, not a request for an aggregate NVVM runtime ABI.
  Date/author: 2026-08-28, Codex.
- Decision: recognize only exact CUDA prelude `GenericAsm` strings and canonical signatures.
  Rationale: this is a bounded lowering of known language intrinsics, not a general evaluator for
  arbitrary inline assembly.
  Date/author: 2026-08-28, Codex.
- Decision: accept an offset only when argument one is a direct field extract of argument zero and
  its key identifies a field in that exact struct type.
  Rationale: IR field identity and the shared CUDA layout are existing sources of truth. Walking
  arbitrary value graphs or inventing structural equivalence would hide malformed input.
  Date/author: 2026-08-28, Codex.
- Decision: fold the earlier type-only scalar/vector family through the same call-site pass and
  delete its helper-emission path.
  Rationale: one source of truth is simpler and ensures all layout-only helpers are gone before
  provider discovery.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

All exact CUDA layout-query forms now share one call-site fold. The earlier scalar/vector type-form
fixture emits only the kernel and six constant stores instead of seven functions, six helper calls,
and six helper returns. The aggregate fixture covers nested struct, fixed array, half matrix,
type/value size, type/value alignment, and four field offsets; the fake provider likewise sees one
function, no query calls or returns, and nine constant stores.

`cuda-array-layout.slang` passes both registered CUDA/NVRTC and direct libNVVM lanes with
`48, 0, 16, 20, 44, 4, 0, 0`. Direct PTX contains `SLANG_globalParams[16]`, six literal stores of
`48, 0, 16, 20, 44, 4`, and no helper definition. CUDA 12.9 `ptxas` accepts the module for `sm_70`.
A query whose base is `left` but whose field extract is `right.value` stops with E52017 before
builder discovery.

The full Release build and isolated provider build pass, as does the complete NVVM prefix at
338/338. Builder ABI revision 3 is unchanged. The result confirms that aggregate layout metadata
does not require aggregate runtime lowering and removes the temporary Slice 74 helper topology
rather than expanding it.

Self-review inventory: `_getNVVMCUDALayoutQuery` survives because it recognizes only the three
exact prelude assembly families, exact parameter counts, one i32-result block, and no executable
body beyond the terminator. `_getNVVMCUDALayoutQueryValue` survives because it delegates policy to
the shared CUDA layout; its only structural rule uses the existing exact aggregate value and
`IRStructKey` sources of truth. `foldNVVMCompileTimeLayoutQueries` survives at the post-link direct
preparation boundary because CUDA source emission still needs the original `GenericAsm`, while
direct NVVM needs compile-time constants before runtime validation.

The input-shape audit found no alternate AST/IR representation: the specialized prelude itself
produces aggregate parameters and field extracts for compile-time queries. Those shapes are valid
before this target lowering but are not runtime values. Removing the fold reproduces the original
`helper function result type` failure on `cuda-array-layout.slang`; retaining helper-terminal
folding alone leaves the aggregate signatures in the direct call graph. The mixed-base negative
proves the offset helper does not walk arbitrary operand graphs or apply a custom equivalence
relation. No syntax is reconstructed, no default is returned for an impossible shape, and no
emitter-local size or offset table was added.

## Context and Current Pipeline

Consider this source from the CUDA suite:

```slang
StructWithArray s;
outputBuffer[0] = __sizeOf(s);
outputBuffer[1] = __offsetOf(s, s.a);
```

After specialization, the first query calls a helper taking `StructWithArray` whose terminator is
`GenericAsm("sizeof($T0)")`. The second passes the same aggregate value plus
`get_field(s, a)` to a helper whose terminator is the CUDA pointer-difference spelling. The current
direct call-closure walk sees the aggregate helper signatures first and rejects them as runtime
types.

The new preparation pass recognizes the exact helper definition at each call. For size/alignment
it asks `getSizeAndAlignment(..., IRTypeLayoutRules::getCUDA(), queriedType, ...)`. For offset it
resolves the field key in the exact struct and asks `getOffset` with those same rules. It replaces
the call uses with an i32 literal and invokes normal module DCE. Direct validation therefore sees
ordinary integer stores and no aggregate query closure.

## Scope and Non-Goals

In scope are type- and value-form `__sizeOf`/`__alignOf`, canonical `__offsetOf`, nested CUDA-layout
structs/arrays/matrices/scalars, call-site folding, DCE, the existing CUDA array-layout shader,
fake-provider observation, and deterministic malformed-offset rejection.

Out of scope are runtime aggregate parameters/results, general aggregate construction or field
extraction in the provider, arbitrary `GenericAsm`, non-CUDA layout rules, pointer-difference
evaluation, layout values outside positive signed i32 size/alignment or nonnegative signed i32
offsets, and noncanonical offset operands.

## Architecture and Invariants

The CUDA IR layout rule remains the sole source of truth for sizes, alignments, and field offsets.
The folding pass recognizes a helper only when it has one block, an i32 result, an exact parameter
count/type relation, and an exact prelude assembly spelling.

A size/alignment query obtains its type either from the explicit type operand of a zero-parameter
helper or from the one helper parameter. An offset query has two parameters and two arguments;
argument one must be `field_extract(argument zero, key)`, argument zero must have the exact struct
type, and `key` must name one of that struct's fields. Any recognizable query that violates those
rules fails with E52017 before builder discovery.

After successful replacements, standard DCE owns removal of dead constructors, field extracts,
query helpers, and their types. Runtime direct-NVVM validation and emission remain free of those
query-only aggregate values and operations.

## Interfaces and Dependencies

Add one preparation entry point beside direct preflight/emission and call it immediately after
`linkAndOptimizeIR`. Reuse `IRBuilder`, `IRFieldExtract`, `IRStructField`, `getSizeAndAlignment`,
`getOffset`, and `eliminateDeadCode`.

The builder ABI revision, provider implementation, libNVVM API, and public Slang API remain
unchanged.

## Milestones

1. Classify exact type/value size/alignment helpers and exact offset helpers.
2. Resolve each call to a CUDA layout value, replace it with i32, and eliminate dead IR.
3. Remove terminator-time type-query emission and update its fake test to observe direct constants.
4. Add aggregate size/offset fake coverage, malformed-offset coverage, and the CUDA array-layout
   direct comparison lane.
5. Format, build, run the complete NVVM prefix and focused runtime lanes, inspect/assemble real
   PTX, update durable design records, self-review, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- the fake provider sees only the entry function for both old type queries and the aggregate query
  graph, with query results arriving as integer constants rather than helper calls;
- a noncanonical offset query stops with E52017 before provider discovery;
- `cuda-array-layout.slang` passes its CUDA/NVRTC and direct libNVVM comparison lanes with
  `48, 0, 16, 20, 44, 4, 0, 0`;
- direct PTX contains the expected constants/stores and no layout-query helper definitions;
- CUDA 12.9 `ptxas` accepts the direct module for `sm_70`;
- the Release host build, standalone provider build, and complete NVVM test prefix pass;
- formatting and `git diff --check` pass; and
- `external/slang-binaries/` and generated build artifacts remain unstaged.

## Failure and Recovery

If an expected query is not recognized, inspect its final linked helper/call shape and compare it
to the prelude before widening the classifier. Do not add aggregate provider operations. If an
offset cannot be tied to one exact field key and base value, reject it rather than searching for an
equivalent aggregate expression.

All changes are isolated to direct-NVVM preparation and tests. Reverting the preparation call and
restoring the Slice 74 helper emission returns the prior scalar/vector-only boundary.

## Artifacts and Hand-Off

Keep dumped linked IR, emitted PTX, `ptxas` output, and runtime logs under ignored `build/` paths.
Distill the compile-time/runtime boundary, accepted query shapes, validation evidence, and next
corpus stop into `docs/design/nvvm-backend.md` and the capability ledger. Complete this plan's
progress, outcomes, and self-review before committing it with Slice 76.
