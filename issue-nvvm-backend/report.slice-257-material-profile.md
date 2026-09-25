# Profile current material compilation and qualify a getter experiment

Status: research acceptance passed. Production sources and artifacts remain accepted256. The local
fallback uses separate statistical, source and identity audits; no fresh independent-agent review
is claimed. No system setting, driver, runtime support or public API changed.

## Motivation

The previous material optimization exposed the AST class-metadata constructor for inlining. Its old
profile cannot identify the next optimization. Both entries of the intact tiled-brass material still
compile through NVRTC O3 and NVVM O0/O3. Consider this existing material helper:

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

Calls such as normalize and dot require ordinary generic/overload checking. The shader has many
such calls; the sampled stacks are not attributed solely to this helper. Material compilation can
be measured without the still-unavailable bindings, textures/LUTs, inputs and runtime output oracle.
Research257 preserves the shader and all six compile identities.

## Proposed solution

Select one future prototype: expose the existing `getCurrentASTBuilder` body for inlining while
retaining the one shared, constant-initialized thread-local pointer. Keep the setter, scope handling,
resolution epochs, cache behavior and platform TLS model unchanged. No optimization is implemented
in this research slice, and no speedup is inferred from the profile.

Semantic checking remains about 406–410 ms per fresh compile. Ten of 218 qualitative snapshots
traverse the current-builder getter across all three seeds. The optimized getter is a separate
32-byte function that calls `__tls_get_addr`; `Val::resolve` calls it before inspecting its cache.
This supports a small, measurable visibility experiment. It is not evidence of a uniquely dominant
hotspot or a guarantee that removing the wrapper call will meet the performance gate.

## Change summary

The completed plan declares the protocol before sampling. `timing-evidence.slice-257.json` records
all six distributions, every attempt, profiles, unchanged identities, the selected source trace
and future gates. The material compile-time design document retains the finding; STATUS names the
next bounded prototype while full256 remains authoritative. Raw evidence is under
`build/nvvm-loop/slice-257-material-profile`.

There are no compiler, provider, runner, fixture, manifest or build-configuration changes. The
production helper/fallback/special-case inventory is empty. Local scripts, primary snapshots,
assembly listings and measurement/profile outputs stay in ignored build directories.

## Concepts and vocabulary

**Active builder** is the ASTBuilder installed for the current thread and checking scope. It can
differ from the builder that originally allocated a reflected type. **Resolution epoch** determines
whether a cached resolved Val can be reused in the active builder's session. **TLS** supplies a
separate pointer slot per thread. **Inclusive timers** include nested work; debugger stack snapshots
perturb execution and do not estimate CPU fractions or call counts.

## Process report

The base is accepted256 `387930f7651d9ee1f04ba8e563740e8d49b0c76b` on nvvm-backend, using the matching
RelWithDebInfo build, CUDA12.9.2/NVRTC12.9.86, LLVM14/providerABI41 and L4SM89 driver580.126.09,
targetingSM80. The compiler remains
`1fc2311e9e0c332f2bff54d65dedb1a225218f740042a2c23c5210e76245c85f`; provider remains
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`. No competing owned
build/test/profile process was present before timing.

The definitive251/245 protocol runs two opposite identity orders, two warmups and nine measured
fresh processes per identity per round. Piped Popen.communicate measures process creation through
exit; log writes occur afterward. Separate assembly uses two warmups and nine samples per identity.
All 132 compiles and 66 assemblies complete with exact256 PTX/cubin outputs. The 108 measured
compiles, 54 measured assemblies and 24/12 warmups are all retained, with no retries or exclusions.
An independent audit reparses phase rows and recomputes medians, inclusive quartiles and extrema.

| Entry / backend   | Wall median (ms) | SemanticChecking median (ms) | Separate assembly median (ms) |
| ----------------- | ---------------: | ---------------------------: | ----------------------------: |
| eval / NVRTC O3   |          1339.16 |                       410.36 |                        172.29 |
| eval / NVVM O0    |          1305.41 |                       405.72 |                        701.66 |
| eval / NVVM O3    |          1408.33 |                       409.77 |                        209.64 |
| sample / NVRTC O3 |          1366.83 |                       409.50 |                        252.73 |
| sample / NVVM O0  |          1342.24 |                       408.18 |                        872.65 |
| sample / NVVM O3  |          1508.12 |                       408.53 |                        293.93 |

These are fresh observations. Historical251/252 timing is unpaired with this session, so no new
improvement is claimed. SemanticChecking/checkAllTranslationUnits are aliases; frontEndExecute and
generateOutput include other phases. Downstream and lifecycle residuals remain unattributed.
Assembly costs are measured separately and do not justify changing the workload's options.

The fresh perf cycles probe fails under the unchanged perf_event_paranoid4 restriction. The existing
GDB sampler instead interrupts the checkAllTranslationUnits-to-generateIR window at seeded3–9 ms
intervals, then drains pending interrupts through normal process exit. Seeds257/258/259 produce
71/73/74 stacks and exact accepted PTX. Debugger elapsed times never enter the benchmark. Of 218
stacks, 183 contain the checker frame; 52 hit the100-frame limit, including all35 that omit the checker.
No fully captured stack is outside checking. The window still includes nearby checking-context work,
and truncation/interrupt bias limits interpretation.

The selected getter path appears4/5/1 times. For example, seed257 stack11 records
`TryUnifyTypes -> as<ConcreteTypePack> -> Type::getCanonicalType -> Val::resolve -> getCurrentASTBuilder`.
Seed258 stack27 reaches the same getter and `__tls_get_addr` while flattening a type pack for
unification. Seed259 stack65 reaches it through subtype-witness checking. All ten full sampled stacks
are retained. Repeated inline subtype-check frames and heterogeneous lookup/allocation samples do not
justify a new hierarchy or cache in this slice.

`checkTranslationUnit` installs its linkage builder with `SLANG_AST_BUILDER_RAII` before constructing
the semantic visitor. `Type::getCanonicalType` and `Val::equals` call `Val::resolve`. Resolve obtains
the current thread's builder, handles the no-context case, and compares `m_resolvedValEpoch` against
`ASTBuilder::getEpoch` before returning a cached value. On a miss it updates the epoch before calling
resolveImpl, preserving recursion behavior and the existing core-module assertion.

The getter simply returns the sole `thread_local ASTBuilder* gCurrentASTBuilder = nullptr`.
`SetASTBuilderContextRAII` saves and restores the previous pointer. Session initialization installs
its root builder; destruction clears it only when that root is still active. The optimized binary
records the getter, setter and pointer as local symbols; none appears in dynamic exports. The
current getter assembly visibly calls the TLS accessor and loads the pointer; the caller assembly
visibly calls the getter. The first objdump query combined demangling with a mangled selector and
returned empty sections. That attempt is retained; the corrected query asserts both calls. No timing
or profiling batch was repeated for this command correction.

This is valid canonical state, not a malformed AST/Val representation. The active builder is the
semantic source of truth: `Type::m_astBuilderForReflection` is explicitly reserved for reflection and
must not replace it during checking. A cached Val does not permit skipping the active-context lookup;
its validity depends on that context's epoch. There is no producer repair, alternate value spelling,
custom equivalence relation or syntax reconstruction indicated by this trace.

A future prototype may place the getter body in the internal header, with one shared TLS declaration
and definition. C++20 constinit can make the existing null initialization explicit if needed to avoid
introducing a cross-translation-unit TLS initialization guard. This is not permission to select an
initial-exec/local-exec TLS model, assume early library loading, duplicate TLS per translation unit,
add a builder/epoch cache or change getter/setter lifetime policy. Do not bundle resolver, substitution,
class hierarchy or hash-table work into the experiment.

Before editing, declare a paired optimized baseline/candidate experiment with both workload/build
order reversals, two warmups and nine measured samples per cell/build/round, retaining every result.
Promotion requires at least5% lower pooled semantic median in every identity,2% lower sum of pooled
wall medians, and no per-identity wall regression above2% in either round, with exact PTX/cubins.
Missed gates require discarding the prototype, not lowering thresholds or adding another optimization.

Correctness must prove initial null state, thread isolation, one storage identity across translation
units, nested and explicit-null scope restoration, current Session lifecycle and unchanged resolution
cache-hit/cache-miss/epoch/no-context behavior. Test dynamic loading with existing and newly created
threads, preserving the supported loader model. Use matching Debug and optimized objects. Existing
parallelGenericEntryPointCompile coverage, full units, generic/overload/conformance/serialization/
reflection neighbors, runtime/toolkit/contracts and full frozen/discovery/material acceptance are
required for shared AST-context implementation. No implementation or revert drill occurs in257.

All133 baseline source,12 artifact and565 runtime-input hashes remain exact256;141 immutable primary
snapshots include eight additional inspection dependencies. Full256's1707 cells/1668 correct/
39 unresolved and18 resolved histories are inherited, not freshly executed. Full256/targeted233/
cadence0 and the rolling252/254/256 implementation history remain unchanged. This study adds no
material runtime claim and makes no changes to system profiling permissions.

Closure verifies597 indexed raw artifacts,141 source snapshots and41 final compact references.
