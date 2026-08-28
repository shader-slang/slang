# Slice 68: Expand numeric and type breadth

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the prototype is no longer hardcoded to Bool/signed-i32/unsigned-i32/float32.
It supports a prioritized coherent bundle of scalar widths, signed/unsigned semantics, conversions,
and fixed vectors sufficient for representative mixed-type kernels, with explicit policy and
differential runtime evidence.

## Progress

- [x] (2026-08-28) Scheduled type breadth after V4 and shared/type representation work so new types
  exercise generic descriptors instead of multiplying callbacks and tests.
- [x] (2026-08-28) Inventoried canonical numeric types/ops/conversions and chose the exact bounded
  bundle using source prevalence, workload value, LLVM/NVVM support, and policy risk.
- [x] (2026-08-28) Extended legalization, V4 signatures, provider operations, and descriptors by
  family without adding a callback, feature bit, facade method, or queried-interface version.
- [x] (2026-08-28) Added one representative mixed scalar/vector workload plus mixed-sign, i24,
  vector-operation, pointer-comparison, resource/shared/atomic, and old-provider boundaries.
- [x] (2026-08-28) Validated Release 436/436, Debug preservation 8/8, compatible assembly, CUDA
  12.9 `ptxas -v`, RTX/NVRTC runtime parity, formatting, ledger/hash, and the final input-shape
  audit.

## Surprises and Discoveries

- A source/test token inventory found `int8_t` 472 times, `uint8_t` 967, `int16_t` 518,
  `uint16_t` 653, `int64_t` 823, and `uint64_t` 1,272. Fixed-vector spellings are also common:
  `int2` 532, `int3` 534, `int4` 201, `uint2` 620, `uint3` 1,927, and `uint4` 684. This supported
  taking all ordinary integer widths together while keeping the vector proof bounded.
- `half` (1,561 token hits) and `double` (841) are prevalent too, but their numerical modes,
  libdevice overloads, ABI surface, and compatibility rules make them policy families rather than
  safe consequences of LLVM type construction. They remain explicit follow-up work.
- Final linked IR preserves exact scalar and pointer widths. Source conversions produce canonical
  `kIROp_IntCast`, `kIROp_CastIntToFloat`, and `kIROp_CastFloatToInt`; the vector producer is exact
  `Ptr(Vector(Int, 2)) -> load -> add -> store`. No operand-graph recovery or source reconstruction
  is necessary.
- V4's existing `{kind, bitWidth, laneCount}` descriptor is sufficient. The exact catalog can keep
  precedence for every legacy adapter while a shared resolver accepts bounded parameterized
  families. Only the three genuinely distinct conversion operation IDs were needed.
- The frozen integer-constant provider callback accepts a signed `int64_t` bit carrier. Narrow
  unsigned literals above their signed maximum therefore need conversion to the equivalent signed
  bit pattern before that callback; changing the old provider contract broke its existing invalid
  input test and was rejected.
- CUDA launch parameter widths agree exactly between NVVM and NVRTC: eight 64-bit pointers, two
  8-bit scalars, two 16-bit scalars, two 64-bit scalars, and one 32-bit float. Natural scalar
  alignments are 1/2/4/8 bytes and signed-i32x2 alignment is 8.
- The local LLVM dependency build contains Release libraries only. A Debug provider link therefore
  stops at missing `LLVMCore.lib`; the rebuilt Debug Slang host still passes the same fake-provider
  preservation class used by prior slices, while all real-provider/compatible/PTX/GPU evidence is
  carried by Release.

## Decision Log

- Decision: the initial candidate bundle is 8/16/32/64-bit signed and unsigned integer values,
  selected integer conversions, float32/integer conversions, and fixed vectors of already-supported
  scalar elements; float64/float16/bfloat/fp8 are audit-gated rather than assumed.
  Rationale: this attacks the current integer hardcoding while separating low-precision/libdevice
  policy from ordinary typed IR mechanics.
  Date/author: 2026-08-28, Codex. Revisit after prevalence and canonical-IR audit.

- Decision: one slice may contain several type families only when a representative kernel composes
  them and each family shares the V4 descriptor/validation architecture.
  Rationale: the goal is faster prototype progress without merging unrelated policy risks.
  Date/author: 2026-08-28, Codex.

- Decision: freeze the scalar bundle as signed/unsigned 8/16/32/64 integers for raw ABI, constants,
  scalar helpers, SSA/phis, naturally aligned device memory, established wrapping arithmetic and
  comparisons, explicit integer conversions, and float32/integer conversions.
  Rationale: these shapes share one canonical classifier, type cache, typed descriptor, and
  provider family implementation, and the mixed workload composes all four widths.
  Date/author: 2026-08-28, Codex.

- Decision: accept exactly signed-i32x2 device load/add/store as the initial fixed-vector proof.
  Rationale: it exercises the lane-count dimension and real vector memory/arithmetic without
  silently claiming arbitrary vectors, vector ABI, constructors, helpers, comparisons, or phis.
  Date/author: 2026-08-28, Codex.

- Decision: retain exact-catalog precedence and add bounded V4 family resolution after it.
  Rationale: legacy V3 adapters and their exact overload identities remain unchanged, while new
  widths add descriptors instead of API surface. A static-only V4 provider predictably rejects the
  family before module creation.
  Date/author: 2026-08-28, Codex.

- Decision: do not generalize resource, fixed-array, shared-memory, or atomic type policy as a side
  effect of ordinary numeric values and device memory.
  Rationale: those representations have independent ABI and semantic contracts; LLVM's ability to
  form a wider operation is not evidence for those contracts.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Complete. The backend now accepts the exact selected scalar bundle and a bounded signed-i32x2
proof. Three conversion operation IDs are the only ABI additions; there are no new callbacks,
features, facade wrappers, queried-interface versions, or whole-signature enums. The type-lowering
context remains the sole provider-type cache, and signedness remains semantic above LLVM's
signless integer types.

The representative kernel combines narrow wrapping/bitwise arithmetic, signed and unsigned
comparison branches, 64-bit arithmetic, integer width/signedness changes, float32/integer
conversion, and two vector lanes. Direct NVVM and NVRTC agree on the fifteen-parameter launch ABI
and all results on the RTX 5090. Normal and compatible provider assembly demonstrate `add i8`,
signed/unsigned `icmp`, `sext`, `zext`, `sitofp`, `fptoui`, and `add <2 x i32>`. CUDA 12.9
`ptxas -v` accepts representative direct `sm_70` PTX with 32 registers, no barriers, 444 bytes
constant memory, no stack, and no spills.

Five registered family-level tests add 468 physical lines across the five measured test/support
files, from 29,707 to 30,175. Release passes 436/436 with registered-name SHA-256
`38cc59e5a3488f84cdb4e5c26cc11f3afbb59e10dcae97036ab64c7e7148054d`; removing the five names
reproduces Slice 67's 431-name hash exactly. Debug preservation passes 8/8.

Explicit deferrals are float64, half/bfloat/fp8 and their libdevice/numerical policy; arbitrary
vectors and matrices; vector construction/constants/comparisons/helper or by-value entry ABI;
integer shifts/division/remainder and saturation/overflow variants; and all non-i32
resource/shared/atomic breadth.

## Context and Current Pipeline

The compiler's current legalization recognizes a deliberately narrow Bool/i32/u32/f32 subset,
with signedness carried semantically above LLVM's signless integer types. Entry parameters, helper
functions, constants, SSA, arithmetic/comparisons, memory, waves, and resources have targeted
hardcoded cases. V4 typed signatures and Slice 67 type/storage work provide the general extension
points this slice must exercise.

## Scope and Non-Goals

In scope are the audit-selected scalar widths/signedness, exact constants, arithmetic/comparisons
already settled for those types, explicit canonical conversions, selected fixed vectors, ABI and
memory operations needed by representative workloads, provider/facade/fake family tests, design,
ledger, and this plan.

Out of scope by default are matrices, arbitrary vector widths, implicit language conversion policy,
overflow/saturation variants, division/remainder/shift semantics not already settled, exhaustive
transcendental math, low-precision formats without an audited policy, and performance claims.

## Architecture and Invariants

One legalization context maps canonical Slang numeric/vector types to one provider type handle per
module. V4 type descriptors carry kind, bit width, and lane count; operations consume typed
signatures. Signedness remains semantic where LLVM is signless. Conversions are explicit operation
semantics selected from canonical IR, not inferred from result use or reconstructed syntax.

Family descriptors parameterize width/kind/lane dimensions. Provider validators check exact
physical types, supported widths/counts, ownership, and availability before mutation. ABI layout is
proved separately from arithmetic correctness. Unsupported type/signature pairs fail predictably.

## Interfaces and Dependencies

Modify centralized NVVM type legalization/cache, V4 typed descriptors and operation support,
emitter catalogs, provider, fake/family tests, representative integration/runtime cases, design,
ledger, and this plan. Add a new queried V4 interface version only if the existing descriptor is
demonstrably insufficient; never append V3 or expose LLVM types.

## Milestones

1. Measure source/test prevalence and trace canonical IR for candidate widths, conversions, and
   vectors; freeze the exact bundle and non-goals.
2. Generalize legalization/type caching and provider type construction with exact invalid tests.
3. Add operation/conversion families through typed V4 rows, not callback/feature proliferation.
4. Add ABI, memory, helper, scalar/vector composition, PTX, `ptxas`, and runtime evidence.
5. Run all regressions, measure marginal growth, document policy/deferrals, audit, and commit.

## Validation and Acceptance

Run focused per-family positive/adjacent-negative tests, V4 signature negotiation and V3 fallback,
full Release NVVM prefix, Debug preservation, compatible assembly, CUDA 12.9 `ptxas -v`, and
NVVM/NVRTC RTX workloads covering signed/unsigned extremes, widening/narrowing boundaries,
float/integer cases, and multiple vector lanes outside the sandbox.

Accept if the chosen bundle is stated exactly; no type is claimed from LLVM permissiveness alone;
one cache owns provider type identity; signedness/conversion semantics remain explicit; ABI and
runtime agree with NVRTC; new families add descriptor rows and shared test data rather than API
fields/bespoke bodies; unsupported pairs diagnose honestly; all regressions, formatting, and audit
pass.

## Self-Review and Input-Shape Audit

Final inventory:

- `isNVVMSupportedIntegerScalarType`, `asNVVMSupportedSignedI32x2Type`, the numeric-value/pointer
  compositions, and the alignment helper survive. They classify exact canonical `IRBasicType`,
  `IRVectorType`, and `IRPtrTypeBase` producers at the centralized type-lowering boundary; they do
  not use builtin names or create a second IR/type representation.
- `resolveV4Family` and `NVVMResolvedValueOperation` survive. The linked-IR instruction itself
  supplies the canonical result/operand types and operation. The stack-owned wrapper only keeps the
  descriptor's operand pointer valid; it does not perform structural matching or rediscover
  context. Exact catalog rows still win before any dynamic rule.
- The provider semantic-type map and family switch survive. They validate the already-negotiated
  descriptor against exact LLVM value types and availability before mutation. The width switch is
  the stated 8/16/32/64 policy; the only lane switch is the stated signed-i32x2 proof.
- Exact-width literal classification and narrow-unsigned bit normalization survive. `IRIntLit` is
  the semantic source of truth, and normalization preserves its bits through the frozen signed
  carrier callback. Removing it regresses existing UInt mask/constant evidence; widening the old
  provider input contract regresses its invalid-input test.
- Natural alignment survives at the memory-emission boundary because the canonical pointee/value
  type determines CUDA ABI alignment. Resource/shared/atomic alignments remain their existing
  exact rules.
- The named `kNoRequiredLegacyFeature` test sentinel survives only in common integration setup. It
  records that parameterized V4 families deliberately have no synthetic V3 feature, while the real
  provider and complete operation descriptors still perform capability preflight.

No new fallback reconstructs syntax, walks arbitrary operand graphs, silently chooses a width,
accepts magic type names, or handles a malformed alternate producer shape. The unsupported matrix
was narrowed only where the new family now owns the behavior; pointer comparisons and independent
resource/shared/atomic boundaries continue to fail before provider mutation.

## Failure and Recovery

If a candidate type depends on unresolved language or libdevice policy, defer that family while
retaining the coherent audited bundle; do not mask it with LLVM casts. If vector ABI differs from
NVRTC, stop and audit upstream layout. Families and catalog rows remain independently removable.
Never stage `external/slang-binaries/` or generated runtime artifacts.

## Artifacts and Hand-Off

Retained above and in the durable design/ledger: prevalence inventory, exact bundle/deferrals,
canonical IR traces, zero-callback descriptor cost, the single module-local type cache, normal and
compatible IR evidence, fifteen-field NVVM/NVRTC ABI comparison, CUDA 12.9 `ptxas -v` resource
data, RTX boundary results, 436-name count/hash, and final self-review. Commit this completed plan
with Slice 68.
