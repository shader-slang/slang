# Slice246: inline the existing AST subtype predicate

## Motivation

The intact tiled-brass material already passes six compile/assembly cells. Research245 measured
semantic checking at651–655ms and found repeated subtype checks in qualitative debugger stacks.
Consider this existing helper, compiled as part of both entry points:

```slang
SurfaceInteraction make_surface_interaction(float2 uv, float3 wi_ws)
{
    SurfaceInteraction si = {};
    si.uv = uv;
    si.wi_ws = normalize(wi_ws);
    si.normal_ws = float3(0.0, 0.0, 1.0);
    si.front_facing = dot(si.wi_ws, si.normal_ws) >= 0.0;
    si.shading_frame_ws = Frame::identity();
    return si;
}
```

Overload and generic checking repeatedly classifies AST nodes. The existing constant-time subtype
predicate lived out of line, preventing callers from optimizing its small range test. The measured
boundary warrants a bounded compiler optimization; it does not warrant changing the material,
generic solving or its canonical AST representation.

## Proposed solution

Move the exact `SyntaxClassBase::isSubClassOf` body into its existing class definition. Implicit
inlining exposes the existing null checks and unsigned interval operation to callers. The tag table,
metadata, casts and producer remain unchanged; no new production helper, hierarchy, cache or fallback.

The minimal candidate passes the predeclared paired performance gate: every semantic median falls
at least5%, the sum of six wall medians falls at least2%, and neither round has a cell wall regression
above2%. Actual reductions are18.43–25.56% semantic and9.43% aggregate wall. Five pooled wall medians
improve11.38–11.83%. Sample/NVVM O3 has substantial dispersion and its pooled wall median is0.066%
slower, although each round's median improves1.92% and9.85%. Do not claim every pooled wall cell improves.
The more invasive direct-tag prototype is deferred because exact relocation meets the agreed gate.

## Change summary

- `source/slang/slang-ast-support-types.h` contains the existing predicate body;
  `source/slang/slang-syntax.cpp` removes its out-of-line definition.
- `check-ast-subtype.py` and `ast-subtype-proof.cpp.in` provide reproducible exhaustive native-Linux
  proof without exporting compiler internals or changing unit-module linkage.
- The completed plan, compact timing/runtime evidence, full census summaries, design note and STATUS
  preserve measured claims, exact final identity, inherited controls and acceptance handoff.

## Concepts and vocabulary

- **Canonical node tag:** `ASTBuilder::_initAndAdd` installs generated `T::kType` through `NodeBase::init`.
- **Class interval:** Fiddle generates `firstTag/tagCount` from the existing C++ hierarchy. A subtype
  test compares unsigned distance to the target's interval size; it already runs in constant time.
- **Inclusive phase timer:** timings include nested scopes and aggregate calls. SemanticChecking and
  checkAllTranslationUnits describe the same interval and must not be added.
- **DeclRef cast restriction:** the existing Slang `IsBaseOf` trait is strict for identical types.
  `as<DeclRefBase>(DeclRefBase*)` remains deleted, while `as<DirectDeclRef>(DeclRefBase*)` is allowed.
  This overload policy is separate from reflexive class membership when casting a `NodeBase*`.

## Process report

The native Ubuntu checkout starts atc29b9b7158ab069141476761f5585c26d3cf7460, branch nvvm-backend.
Its production source is accepted244;245 changes only research documents. Before editing,43 source,
12 artifact and561 input hashes match244. Complete old and candidate bin/lib layouts preserve their
actual compiler libraries, built-in modules and providers. LD_DEBUG plus strace confirms each layout
loads its own copies. RelWithDebInfo, CUDA12.9.2/NVRTC12.9.86, providerABI40, L4SM89 driver580.126.09
and targetSM80 remain fixed. Builds use at most4 CPUs, unit servers2 and sequential GPU suites.

`checkAllTranslationUnits` checks declaration bodies through `ResolveInvoke`, overload candidate
constraints and `GenericArgumentSolver::solve`. A recorded245 path reaches `TryJoinTypes`,
`as<DeclRefType>`, `dynamicCast`, `NodeBase::getClass` and the range test. ASTBuilder already installs
the canonical tag before any cast. `SyntaxClassBase(ASTNodeType)` maps it through generated
`kAllSyntaxClasses`; the predicate reads `firstTag` and tests the target interval. This is valid,
intentional input. No accidental representation, syntax reconstruction, substitution or producer
repair is needed. Exposing the existing operation at this boundary is the responsible-layer change.

The production helper/fallback/special-case inventory is empty: one existing function moves unchanged.
Default/null class references still return false; null and const pointer casts retain their bodies;
DeclRef overload restrictions retain their declarations. The tag constructor's existing assertions
for-1 andCountOf remain byte-identical. Invalid tags are not executed in assertion-disabled builds,
and Debug AST layouts are never mixed with Release objects.

The exhaustive proof uses the registry only for702 class names.702 static assertions establish their
actual enum positions. Independent C++ `__is_base_of` computes all492804 source/target relationships,
including66 abstract classes. Both the original predicate and the candidate agree with every pair.
Typed `getSyntaxClass<T>().createInstance` calls construct636 genuine ASTBuilder objects and allow
normal C++ conversion to NodeBase, avoiding a void-pointer base-layout assumption. Every target checks
all four pointer cast overloads, null inputs, const return types and pointer roundtrips. Metadata/tag
consistency is established before any downcast. Serialization and real semantic checking receive
separate end-to-end regression coverage.

An initial durable-proof compilation incorrectly asserted that DeclRefBase self-as was allowed.
The compiler rejected that test expectation before execution. Source inspection confirmed the existing
strict trait; the corrected negative assertion preserves it. Failed generated source, command and
log remain under `after/proof-final`; corrected final proof is `after/proof-final-v2`. Production
code was not changed to satisfy the test. The full before compiler-unit suite passes1044 tests/14
skips, and before generic/overload/diagnostic/serialization coverage passes1052 tests/77 skips.

The timing driver uses the exact245 compile-command builder with only compiler/output paths changed.
It measures piped `Popen.communicate` through process exit and writes logs afterward. Two rounds reverse
both identity and build order; each identity/build/round has2 recorded warmups and9 measured attempts.
All264 compiles and24 separate assemblies pass; every PTX and cubin equals244/245 byte-for-byte.
There are no timing retries, exclusions or competing owned builds/benchmarks. Phase medians are not
summed. The final compiler source and artifacts are identical to this measured candidate.

| Identity            | Semantic median before → after, ms | Reduction | Wall median before → after, ms | Reduction |
| ------------------- | ---------------------------------: | --------: | -----------------------------: | --------: |
| eval / nvrtc / o3   |                    649.27 → 486.32 |    25.10% |              1614.64 → 1427.57 |    11.59% |
| eval / nvvm / o0    |                    650.10 → 485.86 |    25.26% |              1589.70 → 1401.71 |    11.83% |
| eval / nvvm / o3    |                    649.18 → 483.27 |    25.56% |              1687.24 → 1495.20 |    11.38% |
| sample / nvrtc / o3 |                    649.40 → 485.78 |    25.20% |              1644.28 → 1453.82 |    11.58% |
| sample / nvvm / o0  |                    648.59 → 485.09 |    25.21% |              1625.96 → 1436.04 |    11.68% |
| sample / nvvm / o3  |                    673.44 → 549.32 |    18.43% |              1869.63 → 1870.86 |    -0.07% |

Sample/NVVM O3 round1 varies across built-in loading, semantic checking, IR generation and output in
both builds. Its wall ranges are1923.90–2370.98ms before and1946.52–2541.17ms after; round2 ranges are
1776.16–1815.37ms and1597.76–1795.20ms. The retained samples establish variation, not its cause.
No host-contention attribution is made. All predeclared thresholds still pass without changing the
protocol or selecting favorable subsets; the pooled wall limitation remains part of the result.

The compiler library decreases from20927528 to20606872 bytes. Its ELF `.text` section decreases
from11695100 to11370284 bytes. These are size observations, not an instruction-count or kernel-speed
claim. The provider ABI and source are unchanged.

The final accepted-gate sequence uses one unchanged identity across 117 source/generated/test paths,
12 artifacts and all 561 original runtime inputs. Smoke runtime four passes first, followed by the
exhaustive proof. Full units pass 1045 with 13 ignored; all 1058 test identities match the original
1044-pass/14-ignore baseline and all original passes survive. The sole ignored-to-passed test is
`CapabilityGeneratorFailsOnError.internal`: the copied baseline layout initially lacked the sibling
build-time generator. Its existing `_getGeneratorPath` derives that path from the executable location.
Installing the exact unchanged generator in the copied layout makes the old compiler pass that one
test. The original full baseline and supplemental old-compiler result are both retained; this is not
an optimization-caused capability improvement. All 511 relevant NVVM/routing/reporter/math passes
and the existing Windows skip exactly match 244.

All 1129 semantic regression identities and outcomes match before: 1052 pass and 77 ignored across
generics, overloads, operator overloads, diagnostics and serialization. Toolkit 18 and runner contracts
six pass. Frozen 452 identities/1356 cells and discovery 113 identities/339 cells preserve every old
classification, return code, execution count, diagnostic and canonical shape: 1654 correct, 41 known
unresolved, all 16 resolved histories retained, no additions or deltas. All six final material support
compiles/assemblies pass and preserve exact 244/245 PTX/cubin bytes. No backend admission changes.

The compact validation records exact per-test inventories, baseline generator control, the failed
proof-construction attempt and corrected final proof. It distinguishes freshly executed gates from
historical 244 literal/helper-3548/dynamic-FP8/trace controls and older rawLLVM/BF controls; those
independent controls were not replayed in 246. All 528 indexed 245 artifacts, 15 primary snapshots and
four parent audits remain unchanged. Final source snapshots and all retained raw 246 evidence are
indexed separately.

Full246 is independently accepted, resetting the implementation cadence to zero. Historical rawLLVM/BF16/FP8 controls
retain original source/artifact identities; fresh registered corpus and compiler-unit results do not
relabel those independent controls. All245 artifacts, source snapshots and parent audit files remain
immutable. Material runtime still lacks bindings, textures/LUTs, inputs and an output oracle.
No worker commit, push, driver/system change or reboot occurs. Parent owns acceptance and local commit.

Parent review independently recomputes all paired statistics and gates, checks the isolated compiler
layouts and all264 PTX/24 cubin outputs, verifies the exact predicate-body relocation, and compares
all1695 old corpus outcomes and failure histories. It verifies1058 unit and1129 semantic identities,
117 source snapshots,117/12/561 live identities,2830 indexed raw artifacts and918 references including
historical245 before its six own references. The six parent audit artifacts are retained under
`build/nvvm-loop/slice-246-after` and linked by the validation manifest.
