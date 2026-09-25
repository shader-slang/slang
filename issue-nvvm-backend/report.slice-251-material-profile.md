# Refresh material profiling and select the class metadata constructor

## Motivation

Slice246 improved AST subtype-predicate visibility, but its old profile cannot identify the next
optimization. Both intact tiled-brass entries compile through NVRTC O3 and NVVM O0/O3. Consider
this existing helper, used by both entries:

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

Generic calls such as `normalize` and `dot` require ordinary declaration and overload checking. The
full material contains many such calls; debugger snapshots below are not attributed solely to this
helper. Compilation is measurable independently of the unavailable runtime bindings, texture/LUT
inputs and expected-output contract. This research preserves the full shader and all six identities.

## Proposed solution

Refresh the reliable245 timing protocol on accepted250, then profile separately. Select one future
prototype: expose `SyntaxClassBase(ASTNodeType)` for inlining while preserving the existing generated
tag-to-metadata mapping and every cast/predicate. Implement no optimization in251.

SemanticChecking medians are484.74–488.73ms, with fresh wall1407.13–1601.66ms. Three completed
qualitative profiles repeatedly reach `NodeBase::getClass` and the out-of-line metadata constructor.
Allocation samples span unrelated consumers. The constructor is the strongest defensible bounded
candidate from these observations, not a uniquely dominant function or a predicted speedup.
Historical246 measurements are unpaired with this session; no new improvement is claimed.

## Change summary

- [Completed plan](plan.slice-251-material-profile.md): predeclared protocol, research decisions,
  exact scope and future acceptance/discard obligations.
- [Compact evidence](timing-evidence.slice-251.json): all six distributions, opposite-order rounds,
  phase relationships, exact output hashes, three profiles, source/binary/input preservation and
  immutable primary snapshots/raw index.
- [Design note](../docs/design/nvvm-material-compile-time.md): fresh findings, concrete canonical
  source trace and one proposed constructor experiment.
- STATUS: research handoff while implementation/full250, targeted233 and cadence0 stay authoritative.

No compiler, provider, runner, fixture, material, build configuration or API changes. Production
helper/fallback/special-case inventory is empty. Local scripts and raw outputs remain under
`build/nvvm-loop/slice-251-material-profile`; historical245/246/250 raw evidence is untouched.

## Concepts and vocabulary

- **Canonical node tag:** ASTBuilder initializes each node with the generated `T::kType`.
- **Class metadata mapping:** the generated `kAllSyntaxClasses` table maps that tag to the class's
  existing `SyntaxClassInfo`, including its range, name and create/destruct functions.
- **Inclusive timer:** elapsed time includes nested calls; differently named rows may overlap.
- **Qualitative snapshot:** a debugger interrupt observes one stack and perturbs execution; its
  frequency is not a CPU fraction or call count.

## Process report

The starting revision is accepted250 `5916b57fb1ed95c33b1f7b1730b099a05496add4`, branch
nvvm-backend. Native Ubuntu24.04, NVIDIA L4 SM89 driver580.126.09, targetSM80, CUDA12.9.2,
NVRTC12.9.86, isolated LLVM14.0.6 and providerABI41 match250. Native slang-build guidance and both
helper layers were inspected;203 overrides the old Debug paths with RelWithDebInfo. This is a Ninja
multi-config build with `-O2 -g -DNDEBUG`; the root cache's `CMAKE_BUILD_TYPE=Release` does not select
the active multi-config build. No rebuild occurred. `llvm-config-14` is absent from PATH; the initial
capture failure is retained, and the isolated LLVMConfig.cmake establishes14.0.6 instead.

The compiler library SHA256 remains
`ae6fe92965ed066a02ff9eab550093294b916c6b29f122f02e8a3f2ab8b74f60`; provider SHA256 remains
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`. All119 source/generated/test,
12 artifact and563 runtime-input hashes match250 before and after. Submodules and environment are
retained. The default sandbox's bwrap networking setup fails; approved native execution was used
without changing system configuration.

From repository root, the raw `run.sh` records:

```bash
source build/nvvm-loop/slice-203-env.sh
timeout --kill-after=30s 30m python3 build/nvvm-loop/slice-251-material-profile/measure.py
```

A replay must use a new raw directory. The adapted245 driver uses the existing compile-command
builder with exactly250's source, entry, compute stage, PTX target, SM80 capability, backend,
optimization and performance options. Only output paths change. Two serial rounds reverse identity
order, each with two warmups and nine measured fresh processes per identity. Independent assembly
then uses two warmups and nine samples per identity. Piped Popen.communicate measures through exit,
with log writes afterward,180-second child bounds and a30-minute suite bound. No competing owned
build/benchmark/GPU suite ran. There were no failed, timed-out, retried or excluded benchmark attempts.
Every PTX includes the requested entry/SM80 target and matches250; every cubin also matches250.

| Identity          | Wall median, ms | Inclusive IQR, ms |  Full range, ms | Round1 / round2 medians, ms | Semantic median, ms | Assembly median, ms |
| ----------------- | --------------: | ----------------: | --------------: | --------------------------: | ------------------: | ------------------: |
| eval / NVRTC O3   |         1430.76 |   1429.58–1435.86 | 1419.81–1452.96 |           1431.24 / 1430.60 |              484.74 |              174.89 |
| eval / NVVM O0    |         1407.13 |   1400.07–1414.66 | 1392.56–1454.32 |           1414.59 / 1404.17 |              486.85 |              701.22 |
| eval / NVVM O3    |         1499.71 |   1494.21–1505.71 | 1481.37–1528.94 |           1501.67 / 1495.86 |              486.99 |              209.22 |
| sample / NVRTC O3 |         1460.21 |   1454.91–1464.20 | 1447.37–1502.54 |           1455.88 / 1463.90 |              488.00 |              253.15 |
| sample / NVVM O0  |         1445.76 |   1439.16–1447.85 | 1429.40–1491.97 |           1442.01 / 1447.42 |              488.73 |              880.94 |
| sample / NVVM O3  |         1601.66 |   1597.13–1609.39 | 1590.37–1657.61 |           1599.84 / 1603.74 |              486.25 |              294.03 |

Semantic checking remains the largest localized front-end interval. `frontEndExecute` contains it,
parsing and IR generation; `SemanticChecking` wraps `checkAllTranslationUnits`, and `generateIR`
contains `generateIRForTranslationUnit`. Builtin loading206.78–209.92ms precedes `compileInner` and
includes AST/IR deserialization. IR generation162.42–163.61ms, specialization139.09–145.30ms and
cumulative simplification121.82–128.57ms are inclusive observations, not additive exclusive costs.
`generateOutput` exceeds semantic checking in some cells but contains322.88–346.06ms of linking and
optimization. Output-minus-link/optimize medians130.17–315.59ms remain unattributed output/backend
residuals. Lifecycle-minus-builtin/compile medians70.26–77.50ms are not isolated startup overhead.
Assembly is measured independently; its O0 cost does not justify changing the workload's options.

The separate perf probe still fails at perf_event_paranoid4; permissions are untouched. The corrected
245 GDB sampler takes seeded3–9ms interrupts from `checkAllTranslationUnits` entry to `generateIR`
entry, then drains pending interrupts through normal exit. Seeds251/252/253 collect89/91/93 stacks;
all three complete with exact250 PTX. None of their elapsed times enters the benchmark. The breakpoint
window includes error checks and component construction after the exact semantic timer.268 snapshots
contain the checker frame. Sixteen reach the100-frame cap, including five without that frame; no fully captured snapshot is outside the checker. This caveat and all
unmodified stacks remain in the evidence.

| Leaf                        | Seed251 | Seed252 | Seed253 |
| --------------------------- | ------: | ------: | ------: |
| NodeBase::getClass          |       6 |       6 |       4 |
| SyntaxClassBase constructor |       3 |       3 |       9 |
| isSubClassOf                |       1 |       5 |       3 |
| _int_malloc                 |       6 |      10 |       4 |

The former predicate is not consistently the dominant leaf; inline frames can still appear in GDB.
The class-metadata path is concentrated and repeated. Allocation leaves include Val operand storage,
diagnostic strings, solver dependency maps, substitution caches, serialization and capability-set
copies. Those observations do not isolate one principled allocator change, and this slice does not
investigate multiple independent optimizations.

For the material helper above, the parser's call-expression case creates an InvokeExpr through
ASTBuilder. `SemanticsExprVisitor::visitInvokeExpr` checks operands and reaches `ResolveInvoke` and
candidate selection. Fresh seed253 stack81 records `AddOverloadCandidates -> AddDeclRefOverloadCandidates
-> addOverloadCandidatesForCallToGeneric -> inferGenericArguments -> as<CallableDecl> ->
NodeBase::getClass -> SyntaxClass<NodeBase>(tag) -> SyntaxClassBase(tag)`. The cast in
`inferGenericArguments` examines the generic declaration's inner node. Seed251 stack71 also reaches
getClass when generic constraint collection classifies ordinary parameters. The selected raw stacks
retain full file/line evidence; they establish this compiler path, not exact attribution to `dot`.

`ASTBuilder::_initAndAdd` installs generated `T::kType` through `NodeBase::init` before use. The
constructor maps that canonical tag through the sole generated `kAllSyntaxClasses`, asserts bounds,
and stores the metadata pointer. Its optimized binary still has a separate function containing the
index, pointer load/store and return. Existing casts then use the already-inlined range predicate.
These are valid canonical nodes. No alternate semantic spelling, producer bug, syntax rebuilding,
custom equivalence, generic substitution repair or runtime fallback is indicated.

A future minimal prototype should expose that constructor body for inlining. Unlike246, this is
not a body-only relocation: the table is currently translation-unit static in `slang-ast-boilerplate.cpp`.
It needs one internal shared declaration, for example a private static member of SyntaxClassBase,
with one unchanged generated definition and compile-time agreement with `ASTNodeType::CountOf`.
Preserve pointer identity, table ordering, class factories/destructors, node initialization, exact
valid indexing and invalid-tag assertions, null/default metadata, all four cast overloads and strict
DeclRefBase restrictions. Do not duplicate the hierarchy, change public ABI or reconstruct syntax.
Direct-tag casts and dispatcher changes are larger interventions and remain deferred.

Before editing, the next slice must declare its paired optimized experiment. Require at least5%
lower pooled semantic median in every identity,2% lower sum of the six pooled wall medians and no
identity wall regression above2% in either order round. Use both identity/build order reversals,
two warmups and nine samples per build/cell/round, exact artifacts and all samples. Review phase
dispersion and binary/text size. These are future thresholds, not observed benefits.

Correctness gates must reuse the exhaustive702-tag/492804-pair proof,66 abstract classes and636
factory-created objects; extend it for exact metadata pointer/count/linkage agreement. Preserve
null/const casts, creation/destruction, serialized AST and invalid/default policies. Check invalid
-1/CountOf only with matching assertion-enabled objects, never optimized UB or mixed AST layouts.
Run full compiler units, non-NVVM semantic/serialization regressions, runtime smoke, relevant NVVM
units, toolkit and runner gates, all six exact materials and the full frozen/discovery checkpoint.
Shared AST metadata has broad impact. Reject a prototype that misses semantic/performance gates or
requires another hierarchy, public ABI change, cache or material-specific exception. Do not expand
into another optimization to rescue a failed experiment. No implementation/revert drill exists in251.

| Preservation gate         | Result                                                               |
| ------------------------- | -------------------------------------------------------------------- |
| Source/binary/input       | 119/12/563 exact250 before/after                                     |
| Compile attempts          | 132 passed:108 measured,24 warmups; exact6 identities and all PTX    |
| Assembly attempts         | 66 passed:54 measured,12 warmups; all cubins exact250                |
| Separate profile compiles | 3 passed with exact250 PTX;273 qualitative snapshots                 |
| Fresh runtime cells       | 0; no material dispatch or runtime claim                             |
| Inherited full250         | 1701 cells:1662 correct,39 unresolved;18 resolved histories retained |
| Inherited cadence         | Latest implementation/full250, targeted233, implementation cadence0  |

Unchanged-source research needs no new GPU suite. Existing texture and column-major wrong-output
histories, qualified FP8 controls and BF16 literal acceptance remain linked through validation250
with their original identities. Parent owns independent acceptance and local commit. No worker
commit, push, driver/system change or reboot occurred.

Independent parent acceptance passed all sample/command/output identity checks, recomputed every
reported distribution and profile count, and verified579 indexed artifacts and134 primary snapshots.
The four retained parent audit artifacts are referenced from timing251. The full250 runtime baseline
and cadence remain unchanged.

Parent review additionally found six `NodeBase` visualization conditions in `source/slang/slang.natvis`
that reference `kAllSyntaxClasses[astNodeType]`. Preserve that existing table name/scope during the
constructor experiment, or explicitly adapt and validate these debugger consumers. A private static
member remains an example, not a selected implementation. An additional parent snapshot preserves
this finding without changing the worker's closed raw index.
