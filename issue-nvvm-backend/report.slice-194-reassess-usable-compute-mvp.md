# Slice 194: Reassess the bounded usable-compute MVP

## Motivation

Slice 193 leaves four native-healthy frozen-v1 workloads outside direct O0/O3 correctness. Treating
every historical denominator gap as the next implementation requirement would make corpus score,
rather than representative compute usability, drive the backend again.

The four cases also look more general in census diagnostics than they are in source. For example,
`dynamic-dispatch-substandard-float` first reports helper result type `A`, but `A` contains
`FloatE4M3` and `FloatE5M2`, and its alternate implementation contains `BFloat16`. The remaining
marker case is not inert metadata:

```slang
__requirePrelude("#define MY_CUDA_INTRINSIC 100");
return __intrinsic_asm "(MY_CUDA_INTRINSIC)";
```

Removing that marker would silently change the following expression.

## Proposed solution

Audit the four exact first canonical shapes and classify their feature scope without changing any
historical row. Publish the current frozen, discovery, and proposed-v2 metrics separately; record
why each feature is deferred; and pivot subsequent slices to the open productionization contract.

This is an evidence-only reassessment. It admits no new IR shape, adds no fallback, and does not
freeze the already proposed corpus v2.

## Change summary

- Added a schema-versioned reassessment containing exact corpus metrics, gap identities, producers,
  diagnostics, dispositions, and ranked next work.
- Refreshed the unchanged proposed corpus-v2 union to 473/477 correct in both modes.
- Documented that the selected usable-compute feature gate is met while the historical frozen-v1
  score honestly remains 423/427.
- Selected provider discovery/deployment/cache enforcement as the next implementation boundary.
- Changed no compiler, provider, test, runner, manifest, directive, or provider ABI file.

## Concepts and vocabulary

**Historical denominator** is the frozen set used to compare progress across slices; its rows are
not reclassified when later scope decisions become more precise. **Feature promise** is the bounded
set the initial usable-compute backend claims to support. **Substandard floating point** here means
FP8 and BFloat16 families rather than ordinary Float16/32/64 operations. **Target prelude** is
arbitrary CUDA source injected before C-like target code, potentially consumed by later inline
assembly expressions.

## Process report

The exact Slice 193 frozen snapshot has 452 unique identities and 427 native-healthy MVP-tier
references. It is 423/423/423 at direct O0/O3/both. Discovery remains a separate 82-row corpus with
72 healthy references and is 72/72/72. There are no old-correct regressions, provider failures, or
runtime mismatches among these healthy sets.

Three of the four healthy frozen gaps stop in `_validateNVVMHelperTarget` while it validates a
post-specialization linked `IRFunc` signature. Their first types are `A`, `BFloat16`, and
`FloatE4M3`. This is the correct layer to reject them: these are genuine selected semantic types,
not alternate spellings of already supported types. `A` contains two FP8 values, so teaching helper
classification only to flatten that struct would merely move the first diagnostic into unsupported
FP8 arithmetic/conversion. The bounded MVP explicitly excludes FP8, and neither FP8 nor BFloat16 is
required by a selected representative release gate. They remain deterministic diagnostics until
such a workload justifies one reusable typed representation.

The fourth case starts at `core.meta.slang::__requirePrelude`, whose intrinsic opcode becomes
`IRRequirePrelude`. `CLikeSourceEmitter::emitPreModuleImpl` collects the string while emitting CUDA
source. The test's following GenericAsm refers to a macro defined only by that string. The shape is
canonical and carries live semantics; the producer is not malformed. Direct NVVM cannot support it
by deleting the marker, matching the fixture, or forwarding the text to LLVM. Faithful generic
support would require interpreting CUDA C++ preprocessing and expressions, which is outside the
typed provider architecture. A future standard-module operation can still become supported by
giving its producer a typed semantic, as prior slices did, without admitting arbitrary user text.

Frozen-v1 rows and tiers remain untouched. This is important: changing these four rows to
extensions would turn 423/427 into an artificial 423/423 and destroy the historical contract. The
reassessment instead records two simultaneous facts: the fixed score is 423/427, while all
remaining healthy gaps are outside the bounded feature promise after source-level audit.

The unchanged Slice 157 proposal contributes 50 unique, healthy, both-mode-correct discovery
identities with no frozen source overlap. Its SHA-256 remains
`8126F402CE9F45D706C24A51E189783528DB5B5797AA45035AA7FB2ACD08CE9E`. The proposed union is now 502
rows, 477 healthy references, and 473/473/473; it remains proposed-only because installing it still
requires explicit approval.

The self-review inventory contains one JSON artifact and documentation. There is no new helper,
fallback, special case, syntax reconstruction, operand walk, accepted IR shape, or production
behavior. The next work therefore shifts to the universal boundary every supported workload uses:
enforcing provider discovery, adjacent deployment, ABI matching, caching, and deterministic
failure as an executable production contract. Packaging/install automation, CUDA 13 plus physical
SM70/80/90 validation, and controlled kernel-runtime measurement follow that boundary.

Because no code, build input, test input, manifest, or runner changed, Slice 193's completed
build/runtime/census run is the behavioral evidence. Slice 194 validation reparsed both TSVs,
verified unique identities and all reported counts, checked the proposed-additions hash and join,
parsed the new JSON, and passed `git diff --check`.
