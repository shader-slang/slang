# Material compile-time measurement and next boundary

Slice245 profiles the unchanged tiled-brass material on the accepted244 RelWithDebInfo compiler.
It changes no compiler, provider, runner or material source. The material's two entries each pass
NVRTC O3 and NVVM O0/O3 compilation and SM80 assembly. Runtime bindings, textures/LUTs, inputs and
an output oracle are still missing; compilation evidence does not establish material runtime behavior.

See [timing evidence](../../issue-nvvm-backend/timing-evidence.slice-245.json), the
[report](../../issue-nvvm-backend/report.slice-245-material-profile.md) and
[plan](../../issue-nvvm-backend/plan.slice-245-material-profile.md).

## Measurement contract

The six identities use the existing `run-complex-corpus.py::compile_command` options: intact source,
selected entry, compute stage, PTX target, `cuda_sm_8_0`, selected optimization/backend and
`-report-perf-benchmark`. Two serial rounds run2 warmups followed by9 samples per identity; the
second round reverses identity order. Each compiler runs in a fresh process. Piped
`Popen.communicate` measures startup through exit without the timeout polling of file-redirection
`subprocess.run`. Logs are written outside that wall interval. Assembly is a separate serial study
with2 warmups and9 samples per identity. No competing builds or benchmarks run during measurements.

All132 PTX outputs and66 cubins exactly match accepted244's per-identity artifacts. The43 source,
12 binary/toolkit and561 runtime-input hashes match before and after. Warmups are retained. An
interrupted polling-method pilot is explicitly excluded in its entirety, independent of its values.

| Entry/backend     | Fresh wall median, ms | Inclusive IQR, ms | SemanticChecking median, ms | Independent assembly median, ms |
| ----------------- | --------------------: | ----------------: | --------------------------: | ------------------------------: |
| eval / NVRTC O3   |               1624.80 |   1616.52–1629.37 |                      651.51 |                          178.18 |
| eval / NVVM O0    |               1596.77 |   1592.28–1613.68 |                      652.00 |                          703.84 |
| eval / NVVM O3    |               1691.05 |   1690.07–1702.48 |                      653.09 |                          210.52 |
| sample / NVRTC O3 |               1647.47 |   1642.27–1652.54 |                      652.73 |                          253.37 |
| sample / NVVM O0  |               1640.20 |   1632.79–1652.00 |                      654.86 |                          882.24 |
| sample / NVVM O3  |               1792.80 |   1789.31–1798.32 |                      652.66 |                          296.63 |

The reverse-order round changes per-cell wall medians by at most0.75%. Exact ranges, both round
statistics, all named phase distributions and command arrays remain in the linked evidence.
These are unchanged-compiler observations, not an optimization comparison.

## What the timers mean

`PerformanceProfilerImpl::enterFunction/exitFunction` accumulates inclusive elapsed duration by name
in a thread-local profiler. It neither subtracts children nor resets at every compile request.
The scopes below follow the source, not an assumption that printed rows are disjoint stages:

- `compileInner` wraps `EndToEndCompileRequest::executeActions`; `endToEndActions` is its inner
  action scope. They describe almost the same interval for these invocations.
- `frontEndExecute` contains parsing, `SemanticChecking` and IR generation.
  `SemanticChecking` directly wraps `checkAllTranslationUnits`; those rows must not be added.
  `generateIR` contains `generateIRForTranslationUnit`.
- Fresh CLI built-in loading precedes `compileInner`. `loadBuiltinModule` contains serialized AST
  and IR reads. Its206–210ms medians do not add to its child timers.
- `generateOutput` includes `linkAndOptimizeIR` and the remaining backend/output work.
  Specialization and simplification are nested compiler work. `simplifyIR` aggregates multiple
  invocations, including calls from other passes; it is not an exclusive stage.

Semantic checking is651.5–654.9ms, about36.4–40.9% of each fresh-process wall interval. IR generation
is184.4–185.1ms, specialization138.3–144.5ms and cumulative simplification121.8–128.7ms. These
observations prioritize the front end; their durations must not be summed into a total.

Per-attempt `generateOutput - linkAndOptimizeIR` yields median residuals129.6–316.2ms. That includes
emission, provider/downstream work, diagnostics and artifact handling; it is not measured libNVVM
or NVRTC time. Wall minus `compileInner` minus built-in loading leaves68.9–76.5ms, which includes
argument handling, profiler printing, teardown and process overhead. It does not isolate process
startup or session construction. `ptxas` is measured directly and separately; its O0 cost is larger
here, but assembly is not the selected compiler optimization boundary.

[Slice214](../../issue-nvvm-backend/report.slice-214-complex-batching.md) already qualified bounded
shared-session reuse. Its measured batch savings remain historical, and its mandatory fresh
references made short complete runner invocations slower. This study does not repeat that work or
turn aggregate shared-session profiler rows into per-request phase medians.

## Selected next hypothesis: canonical AST subtype checks

Unprivileged `perf` is denied at perf_event_paranoid4. Separate GDB runs interrupt only the semantic
checking interval at seeded3–9ms delays. Three completed eval/NVVM O3 runs collect121,118 and119
stacks. `SyntaxClassBase::isSubClassOf` is the most frequent leaf in each, with19,22 and22 snapshots;
`NodeBase::getClass` contributes6,6 and8 leaves. All three completed outputs exactly match244.
These are qualitative wall-interrupt observations with debugger perturbation, not unbiased CPU
fractions, call counts, or a predicted speedup. One auxiliary trial stopped after checking because
of a pending debugger interrupt; its evidence is retained and excluded from the completed-run
summary. The corrected local cleanup drains interrupts through process exit.

Consider this existing material helper:

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

The material contains many overloaded/generic calls of this kind; the snapshots do not attribute
all sampled work to this helper. `checkAllTranslationUnits` calls `checkTranslationUnit`, which
creates a shared semantics context and calls `SemanticsDeclVisitorBase::checkModule`. Declaration
checking visits expressions and resolves calls through `ResolveInvoke`, overload candidate checking
and generic argument inference. A recorded stack reaches `TryCheckOverloadCandidateConstraints`,
`GenericArgumentSolver::solve`, `TryJoinTypes`, `as<DeclRefType>`/`dynamicCast`, then the class test.

`ASTBuilder::_initAndAdd` initializes `NodeBase::astNodeType` with generated `T::kType`. That tag is
already canonical. Today `as<T>(NodeBase*)` and `dynamicCast<T>` call `node->getClass()`, whose
out-of-line `SyntaxClassBase(ASTNodeType)` constructor looks up `kAllSyntaxClasses[tag]`.
`SyntaxClassBase::isSubClassOf` then reads the resulting `firstTag` and performs an unsigned range
comparison against the destination's generated `firstTag/tagCount`. Fiddle generates this metadata
from the existing hierarchy. The algorithm is already constant-time; the hypothesis concerns the
metadata roundtrip and call visibility, not replacing a hierarchy walk.

A bounded next prototype should compare minimal inlining of the existing predicate with a
NodeBase cast path that uses the canonical tag directly. Reuse one range operation and the existing
generated metadata as the sole hierarchy truth. Do not introduce another hierarchy table, a
material-specific shortcut, changed generic solving, syntax reconstruction or semantic caches.
Default/null `SyntaxClassBase` behavior is separate from a correctly initialized non-null NodeBase;
retain existing null-pointer casts and invalid-node policy. No malformed representation was found.

Before implementation, exhaustively compare every registered source tag against every target class,
including abstract classes, against the generated hierarchy and original predicate. Cover null
pointers, const overloads, invalid/default policy, existing `DeclRefBase` cast restrictions and AST
serialization. Run relevant non-NVVM overload/generic/constraint/diagnostic tests, compiler units,
all six byte-identical material outputs and a full frozen/discovery checkpoint: AST casts have broad
front-end impact. Do not claim a narrow NVVM-only regression domain.

For a future paired optimized-build experiment, a useful promotion gate is at least5% lower
SemanticChecking median in every identity and at least2% reduction in the sum of the six per-identity fresh wall
medians (18 measured samples per identity), with no identity regressing more than2% in either order round. Validate assembly and output
identity before timing acceptance. These are proposed future thresholds, not measured benefits.
Reject the prototype if savings are not robust or semantic preservation fails. No optimization was
implemented in245.

## Slice246: retain exact predicate inlining

The bounded prototype moves `SyntaxClassBase::isSubClassOf` from `slang-syntax.cpp` into its existing
class definition in `slang-ast-support-types.h`. Its body is unchanged. Generated hierarchy metadata,
tag-to-class lookup, ASTBuilder initialization and pointer cast overloads remain the only existing
path. Once this minimal variant passed the predeclared gate, the direct-tag alternative was deferred;
no second hierarchy, semantic cache, new production helper or producer repair was needed.

The [paired evidence246](../../issue-nvvm-backend/timing-evidence.slice-246.json) compares isolated,
verified before/after compiler layouts in two opposite identity/build orders. Each cell/build/round
has2 warmups and9 measured fresh processes. All264 compiles and24 support assemblies preserve exact
244/245 PTX and cubin bytes. Semantic medians improve18.43–25.56%; the sum of six wall medians improves
9.43%. Five pooled wall medians improve11.38–11.83%. Sample/NVVM O3 instead has a0.066% slower pooled
wall median despite per-round median improvements1.92% and9.85%; round1 has substantial variation
across multiple phases in both builds. Its cause is not established. Retain all samples and this
limitation; do not claim every pooled wall cell improves or infer a kernel speedup.

All three thresholds from245 pass unchanged. No outliers, retries or favorable subsets are removed.
The final compiler source and12 artifact identities remain the measured candidate through correctness
acceptance. Later refinements to the standalone proof script require only another proof invocation.
The compiler library decreases320656 bytes; its ELF `.text` section decreases324816 bytes. These are
size observations, not instruction-count evidence.

[The reproducible proof](../../issue-nvvm-backend/check-ast-subtype.py) links the configured native
Linux Ninja compiler objects into a separate executable, avoiding exported test APIs. Registry names
provide membership only;702 enum-position static assertions and the C++ compiler's independent
inheritance relation cover all492804 pairs, including66 abstract classes.636 typed factory-created
objects exercise all four cast overloads, nulls, const return types and pointer roundtrips. The original
predicate remains a compatibility reference. Default/null metadata and unchanged invalid-tag assertions
are separate contracts: never test invalid tags against optimized objects or mix Debug AST layouts
with Release objects. The existing strict `Slang::IsBaseOf` restriction also keeps DeclRefBase self-as
deleted; derived DeclRefBase casts retain their old policy.

Run the proof only after a matching compiler build, with a new output directory:

```bash
python3 issue-nvvm-backend/check-ast-subtype.py --output build/ast-subtype-proof
```

The [slice246 report](../../issue-nvvm-backend/report.slice-246-ast-subtype.md) and
[validation manifest](../../issue-nvvm-backend/runtime-validation.slice-246.json) record full shared-AST
acceptance, serialized-AST/semantic/compiler-unit coverage, exact preserved corpus outcomes, original
failure histories and inherited independent controls. Material runtime still requires its missing
application bindings, texture/LUT inputs and expected-output contract.

## Slice251: refresh the profile after predicate inlining

[Research251](../../issue-nvvm-backend/report.slice-251-material-profile.md) and its
[compact evidence](../../issue-nvvm-backend/timing-evidence.slice-251.json) measure accepted250 without
compiler changes. The same two opposite-order rounds retain132 compiles (108 measured,24 warmups),
followed by66 independent assemblies (54 measured,12 warmups). Every output matches accepted250;
119 source,12 artifact and563 runtime-input hashes remain exact. All samples survive without retries
or exclusions. Fresh wall medians are1407.13–1601.66ms and SemanticChecking484.74–488.73ms. Historical
246 is not a paired baseline for this session; no new speedup is established.

Semantic checking remains the largest localized front-end interval. Some inclusive `generateOutput`
medians are larger, but include322.88–346.06ms linking/optimization and130.17–315.59ms unattributed
output residual. Neither those rows nor other nested timers are additive. No material runtime or
kernel-speed claim follows from these measurements.

Fresh separate GDB seeds251/252/253 complete with exact PTX and89/91/93 snapshots. getClass leaves
are6/6/4; the SyntaxClassBase constructor3/3/9; the already-inlined predicate1/5/3. Allocation leaves
6/10/4 have heterogeneous consumers. These qualitative observations nominate a narrow repeated
metadata path, not a unique dominant function or CPU fraction. The breakpoint window runs from
checkAllTranslationUnits to generateIR, also spanning intervening component construction.268 stacks
contain the checker;16 hit the100-frame cap, five before that frame. No fully captured stack is
outside the checker. Debugger timings are excluded entirely; perf remains denied at paranoia4.

The concrete helper shown above exercises ordinary call checking. The parser creates InvokeExpr
through ASTBuilder; visitInvokeExpr and ResolveInvoke select candidates. Fresh seed253 stack81 reaches
`inferGenericArguments -> as<CallableDecl> -> NodeBase::getClass -> SyntaxClassBase(ASTNodeType)`
when classifying the generic declaration's inner node. These samples establish the compiler path,
not attribution of all work to that helper. `_initAndAdd -> NodeBase::init` already installed its
canonical generated tag. The constructor maps it to the existing class metadata, which consumers
use for subtype ranges and other reflection. No malformed representation or producer repair appears.

The bounded next hypothesis is making this existing constructor visible for inlining. Its table is
currently translation-unit static, so this needs one internal shared declaration and one generated
definition, with compile-time agreement against ASTNodeType::CountOf. A private static table member
is one possible arrangement. This is more than246's body-only relocation; linkage, pointer identity,
count/order and assertion policy must be proved. Preserve the same metadata factories/destructors,
node initialization, casts and predicate. Do not add a second hierarchy or public ABI change.
Direct-tag casts/dispatch changes and dispersed allocation work remain separate deferred candidates.

A future prototype must preserve exhaustive tag/class/cast behavior, metadata pointer identity and
serialization, and run shared-AST units/semantic regressions plus a full frozen/discovery checkpoint.
Its paired optimized before/after study must reverse identity and build order, retain all samples and
exact artifacts, lower every pooled semantic median at least5%, lower the sum of six pooled wall
medians at least2%, and avoid wall regressions above2% in either round. These are proposed gates,
not predicted savings. Discard on failed gates or unwanted semantic/ABI expansion; record a negative
result rather than bundling another optimization. No constructor prototype is implemented in251.

The existing `source/slang/slang.natvis` NodeBase visualization also names `kAllSyntaxClasses` in
six conditions. The linkage choice must preserve those lookups or explicitly adapt and validate
them; retaining the existing name/scope avoids an unnecessary debugger-consumer change.

## Slice252: expose the existing class metadata lookup

The bounded prototype keeps `Slang::kAllSyntaxClasses` as the only generated tag-to-metadata mapping
and exposes the existing `SyntaxClassBase(ASTNodeType)` body in its internal header. An incomplete
extern declaration allows the generated definition to deduce its true length; an independent static
assertion equates that length with CountOf. Both bounds remain, and every valid tag returns the same
class metadata object. The generated initializer, factories/destructors, predicate, casts and node
initialization remain unchanged. No direct-tag dispatcher, alternate hierarchy, cache or API change.

The name and namespace preserve existing NatVis references. The object symbol is GLOBAL HIDDEN,
and the final compiler library localizes it; exported names are unchanged. All702 exact typed
metadata pointers and492804 C++ inheritance pairs pass in matching before/candidate Debug and
optimized proofs, including636 real typed objects,66 abstract classes, all four cast overloads,
null/const/default behavior and strict DeclRefBase restrictions. Real Debug invalid tags-1/702
produce the specific constructor-bounds diagnostic and abort. Debug and optimized layouts are never
mixed. Registered nontrivial objects use the actual ASTBuilder destructor path.

Header insertion shifts only generated FIDDLE source-line macro names by exactly+7; metadata/enum
files remain byte-identical. An initial overly broad generated-byte audit stopped on this difference;
the exact expected transform passes a retained corrected audit without changing production code.

[Paired timing252](../../issue-nvvm-backend/timing-evidence.slice-252.json) retains264 compiles
(216 measured,48 warmups), opposite identity/build orders and24 independent support assemblies.
Every PTX/cubin equals250/251. Semantic medians fall16.86–18.89%; summed six wall medians fall7.35527%,
and all12 per-round wall comparisons improve. The predeclared5% semantic/2% aggregate wall/2% maximum
round-regression gates pass. No samples are excluded or retried. Before eval/NVRTC O3 has slower
round1 values across multiple phases, and candidate sample/NVVM O3 varies between rounds; causes
are not established. All distributions remain available. The ELF `.text` section shrinks67616bytes,
and the compiler file shrinks82416bytes. These are compile-time/size observations, not kernel claims.

The [report252](../../issue-nvvm-backend/report.slice-252-ast-class-lookup.md) retains concrete source
trace and exact proof/source differences. Full shared-AST/corpus checkpoint preserves all1701 outcomes (1662 correct,39 unresolved),
all18 resolved histories, exact1060 unit/1129 semantic outcomes and all required gates. Independent
parent acceptance verifies every timing statistic, corpus outcome, failure history and indexed artifact.
Compiler sources, measured artifacts and inputs remain fixed through acceptance;
only the proof-driver help example is corrected after timing, with its exact doc-only diff recorded.
Material runtime still lacks its binding, texture/LUT input and output contract. Any next optimization
needs fresh evidence on the accepted compiler; direct-tag casts and allocation changes remain separate.

## Research257: current-builder getter visibility

Fresh accepted256 measurements retain all132 PTX/66 cubin outputs. SemanticChecking medians are
405.72–410.36 ms and fresh-process wall medians1305.41–1508.12 ms; historical timings are unpaired,
so this is no new speedup claim. Three completed semantic-window debugger runs collect218 stacks.
The current-builder getter path appears4/5/1 times;52 stacks reach100 frames, including all35 that
omit the checker frame. Counts are qualitative observations, not CPU fractions.

The selected future experiment exposes only getCurrentASTBuilder for inlining. The current optimized
getter is a separate32-byte function calling __tls_get_addr and returning the one gCurrentASTBuilder
pointer. Val::resolve calls it before comparing its cached epoch with the active builder's session.
The active builder is canonical checking context; the reflection allocation builder is not a valid
substitute. Scope restoration, null handling, epoch invalidation, recursion and Session lifecycle
must remain exact. No new cache, TLS model, per-TU copy or lifetime policy is justified.

Keep one shared constant-initialized thread-local definition. If header exposure needs an explicit
C++20 constinit declaration to avoid introducing a dynamic TLS initialization guard, qualify that
in the prototype. Preserve dynamic-library loading with existing/new threads and hidden linkage.
Require paired reversed-order timings, exact outputs, thread/scope/epoch proofs in matching Debug
and optimized builds, parallel generic compilation and full shared-AST correctness acceptance.
Promotion gates remain5% semantic improvement per cell,2% summed wall improvement and no2% per-round
cell wall regression. Discard a failed prototype without adding independent optimizations.

See [plan257](../../issue-nvvm-backend/plan.slice-257-material-profile.md),
[report257](../../issue-nvvm-backend/report.slice-257-material-profile.md) and
[timing257](../../issue-nvvm-backend/timing-evidence.slice-257.json). No implementation, runtime
support change or material GPU performance claim is part of this research.
