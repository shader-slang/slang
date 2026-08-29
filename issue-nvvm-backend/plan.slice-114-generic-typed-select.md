# Add generic typed value selection

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM lowers canonical Slang `select` instructions through the existing
typed value-operation interface. Scalar and two- through four-lane Boolean, selected integer,
Float16, and Float32 values share one descriptor contract. The existing
`tests/compute/logic-no-short-circuit-evaluation.slang` fixture gains direct runtime and PTX lanes
only when its unchanged `30, 31, 32, 33` result passes.

## Progress

- [x] (2026-08-29) Probed the existing fixture after Slice 113 and recorded its first stop as a
  same-lane `select(Vec<Bool,2>, Vec<Bool,2>, Vec<Bool,2>)` after both source alternatives have
  already executed.
- [x] (2026-08-29) Added the generic semantic family, provider emission, direct-emitter mapping,
  and fake-provider observation under one forward-only ABI revision.
- [x] (2026-08-29) Added positive scalar/vector and negative descriptor builder coverage plus focused
  direct-emitter coverage of the canonical Boolean-vector shape.
- [x] (2026-08-29) Added a focused file-backed runtime/PTX fixture after the existing corpus fixture
  exposed a separate initialized module-global boundary; assembled its PTX, passed the complete
  NVVM prefix 382/382, updated durable status, formatted, and self-reviewed the slice.

## Surprises and Discoveries

- The fixture's vector `&&` and `||` already reduce to established Boolean and integer operation
  families. Only the vector conditional remains as `select`; the calls that mutate the global
  result occur before it, so LLVM selection cannot accidentally introduce short-circuiting.
- LLVM's select shape already matches the semantic descriptor exactly: a scalar Boolean selects
  scalar alternatives, while a Boolean vector selects same-lane vector alternatives. No broadcast,
  branch synthesis, or source-operator callback is needed.
- After select was admitted, the existing fixture stopped at its mutable module-scope `static int`.
  The provider's current generic global declaration initializes internal storage with `undef`, so
  simply admitting that pointer would not preserve the source initializer. The slice therefore
  uses a focused file-backed select fixture and records initialized device globals as the next
  independent ABI/storage boundary.

## Decision Log

- Decision: add one generic typed select family rather than a Boolean-vector-specific callback.
  Rationale: the operation is determined completely by result and operand descriptors. Requiring
  an exact Boolean selector lane count and exact alternative/result equality scales across all
  selected first-class value kinds without enumerating overload combinations.
  Date/author: 2026-08-29, Codex.
- Decision: do not permit scalar-condition vector selection in this slice.
  Rationale: LLVM requires vector conditions for element-wise vector select, and the observed
  canonical Slang IR already materializes that shape. Admitting an unobserved broadcast spelling
  would create a second representation rather than consume the producer's invariant.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Implementation is complete. ABI revision 20 adds only the semantic select ID; the existing generic
operation callback carries all types and values. The real builder emits scalar-Half and vector
Boolean/integer selects in both assembly dialects. The fake emitter records the exact Boolean-vector
descriptor, and its operation recorder now honors the API's existing three-operand maximum rather
than its historical two-operand test-only allocation.

The focused runtime/PTX fixture produces `0, 0, 1, 0`. CUDA 12.9.86 `ptxas -arch=sm_70` accepts its
1,193-byte PTX and emits a 3,048-byte cubin. Release provider/compiler/unit builds pass, and the
complete NVVM prefix passes 382/382. The motivating corpus fixture advances to E52017 `device scalar
pointer`; no tentative directive was added because its initialized global remains unsupported.

The representation self-review retains no AST/IR equivalence, fallback, source-syntax recognition,
branch reconstruction, or serializer rewrite. The only new validity rule consumes the canonical
three-operand select shape. The emitter shares that rule with capability preflight; the provider
performs exact LLVM type and insertion-point checks before `CreateSelect`; and the fake-only
three-slot record matches the descriptor's pre-existing maximum. The device-global producer was
audited and deliberately left unsupported because using `undef` would violate its initializer.

## Context and Current Pipeline

Slice 113 demonstrated that specialized source constructs disappear before direct preflight. The
next corpus stop is ordinary canonical value selection. `_resolveNVVMValueOperation` already maps
IR operations to descriptors, the semantic catalog validates bounded typed families, and the real
provider emits their LLVM instructions. This slice extends those same three boundaries instead of
adding another emitter/provider method.

## Scope and Non-Goals

In scope are scalar and fixed two- through four-lane selection for Boolean, signed/unsigned selected
integer widths, Float16, and Float32 values; exact selector lane matching; exact alternative/result
type matching; focused malformed-descriptor rejection; and the existing non-short-circuit fixture.

Out of scope are aggregate, pointer, resource, matrix, Float64, BFloat16, FP8, scalable-vector, or
scalar-broadcast select; changing logical-operator legalization; and converting control flow to
select in the direct emitter.

## Architecture and Invariants

- Slang IR remains the source of truth for whether an expression is control flow or eager value
  selection. The emitter maps only `kIROp_Select` and never reconstructs it from branches.
- A valid select has exactly three operands: a selected Boolean condition, then two alternatives
  exactly equal to the result descriptor. All four descriptors have the same lane count.
- The semantic catalog is the sole validity predicate used by facade capability queries, fake
  recording, and the LLVM provider.
- Preflight validates every operand's dominance and availability before provider mutation.

## Interfaces and Dependencies

Forward-only builder ABI revision 20 adds one semantic operation ID but no callback or table field.
The existing generic value-operation method carries its three typed operands. LLVM 14
`IRBuilder::CreateSelect` supplies the implementation and the existing NVVM IR 2.0 text serializer
must preserve its LLVM 7-compatible spelling.

CUDA 12.9 libNVVM, `ptxas`, the Release provider, and the local CUDA runtime provide external
evidence.

## Milestones

1. Extend the operation ID, semantic resolver, real/fake providers, and direct IR mapping.
2. Exercise scalar and vector selected kinds in the real builder, reject mismatched selectors and
   alternatives, and observe the exact vector-Boolean descriptor through the fake emitter.
3. Add direct runtime/PTX lanes to a focused file-backed fixture, record the original corpus
   fixture's next independent stop, then build, test, assemble, document, format, self-review, and
   commit the slice with this plan.

## Validation and Acceptance

Acceptance requires Release provider/compiler/unit builds; focused ABI, builder, and emitter tests;
exact direct runtime and PTX lanes for a file-backed canonical select fixture; a recorded next stop
for the original corpus fixture; CUDA 12.9 `ptxas -arch=sm_70`; the full
`slang-unit-test-tool/nvvm` prefix; pinned clang-format; and `git diff --check`.

The self-review inventories the operation mapping, family validity rule, provider emission, fake
value classification, and test directive. For each retained change, verify that it consumes the
canonical three-operand IR shape and does not infer source syntax or rebuild control flow.

## Failure and Recovery

If libNVVM rejects LLVM's ordinary select spelling or runtime behavior differs, preserve the IR/PTX
under ignored `build/slice114-*`, remove only the fixture promotion, and record the exact provider
boundary. Do not widen the serializer or scalarize vectors without evidence. Do not reset unrelated
work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep IR, PTX, cubin, and focused logs under ignored `build/slice114-*`. Distill the settled typed
select contract and runtime evidence into `docs/design/nvvm-backend.md` and
`docs/design/nvvm-backend-capability-ledger.md`, then commit this completed plan with implementation
as explicitly requested.
