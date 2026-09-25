# Discard the current AST builder getter visibility prototype

Status: research accepted; performance gate failed and the prototype is discarded. Production
source and tested artifacts are restored exactly to256. The local fallback uses separate statistical, source and identity audits,
without claiming a fresh independent-agent review. No candidate optimization is accepted.

## Motivation

Research257 observed the current-builder getter in ten qualitative semantic stacks. The intact
material contains generic and overloaded calls such as these:

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

All measurements use the unchanged full tiled-brass shader and its actual eval_buffer/sample_buffer
entries. Type canonicalization
reaches Val::resolve, which retrieves the active AST builder before testing its resolution cache.
The existing out-of-line getter calls the TLS accessor. The experiment asks whether visibility
alone removes enough overhead to justify changing that internal boundary.

## Proposed solution

Reject the prototype under the predeclared gates. Moving the existing getter into its internal
header, with an extern thread_local constinit declaration and the one constant-initialized TLS
definition, removes the wrapper call in optimized Val::resolve. However, pooled semantic medians
improve only 1.6512–2.4123% across six cells, below the required 5% in every cell. The sum of pooled
wall medians improves 0.66549%, below the required 2%. No round-specific regression exceeds 2%, but
that does not compensate for the failed improvement thresholds. Keep the accepted getter unchanged.

## Change summary

The completed plan, compact timing evidence, design finding and STATUS retain the bounded negative
result. Raw baseline/candidate layouts, source snapshots, commands, patch, timing, assembly, loader
checks and audits stay under build/nvvm-loop/slice-258-before and slice-258-prototype. Restoration
records stay under slice-258-restored. No production, provider, fixture, corpus or public API change
will be committed. The only production helper inventory entry is the existing getter's relocated
body, now reverted; the constinit annotation and shared extern declaration are also reverted.

## Concepts and vocabulary

**Active builder** is the thread's current semantic checking context, which can differ from the
builder that allocated a reflected type. A **resolution epoch** qualifies cached Val results for
that context. **TLS** supplies separate storage per thread. A **pooled median** uses all 18 measured
samples per cell/build across both rounds; round medians retain each 9-sample order separately.

## Process report

Base c1955155467403566171e4655cec3692b62ecbf6 is research257, with unchanged implementation256.
The native optimized compiler, CUDA12.9.2/NVRTC12.9.86, LLVM14/providerABI41 and L4SM89 driver580.126.09
remain the established environment; generated PTX targetsSM80. All 133 baseline source, 12 artifact
and 565 runtime-input identities match256. The two independent optimized layouts each retain 80
files, including compiler, provider and builtin module. Eighteen submodule pins remain unchanged.

The valid input is the active ASTBuilder installed by checkTranslationUnit through
SetASTBuilderContextRAII. Type::getCanonicalType and Val::equals call Val::resolve, which obtains
that builder and then handles no context, cached epoch matches and recursive resolution. This is
canonical semantic state. No alternative value spelling or malformed producer was found, and the
reflection allocation builder would be an invalid replacement. Setter, scope restoration, Session
lifecycle and all resolution logic remained byte-for-byte unchanged in the candidate.

The prototype changes only slang-ast-builder.h and .cpp. The header exposes the existing getter
body and declares the existing pointer with extern thread_local constinit; the .cpp keeps the one
definition with explicit constant initialization and removes the out-of-line body. The configured
C++20 optimized build succeeds. Symbol/disassembly inspection proves one local TLS symbol, exact
public export names, no introduced TLS initialization guard, and direct __tls_get_addr access from
Val::resolve instead of the wrapper call. The setter continues using the existing TLS model.
These observations qualify the intended code-generation mechanism, not its full semantics.

Before timing, strace and dynamic-loader diagnostics prove each isolated layout loads its own
compiler, builtin module and provider. Both control compiles preserve accepted PTX. Timed runs
then use six fixed identities, baseline/candidate order in round 0 and candidate/baseline order in
round 1, with the workload order also reversed. Each cell/build/round has 2 warmups and 9 measured
fresh processes. All 264 compiles (216 measured, 48 warmups) and 24 assembly checks pass and preserve
accepted256/257 PTX/cubin hashes. No sample is retried or excluded. Assembly is a support check,
not a downstream performance measurement.

The timer surrounds piped Popen.communicate through exit; log writes occur afterward. Independent
acceptance verifies the full ordered inventory, command options, logs, output hashes and unchanged
layouts, then recomputes medians and inclusive quartiles using interpolation independently of the
measurement driver's statistics library. The gates were declared before editing and remain fixed.

| Entry / backend   | Semantic median reduction | Wall median reduction | Round0 wall change | Round1 wall change |
| ----------------- | ------------------------: | --------------------: | -----------------: | -----------------: |
| eval / NVRTC O3   |                   2.4123% |               0.8963% |           -1.8883% |           -0.5712% |
| eval / NVVM O0    |                   2.1604% |               0.8833% |           -0.7002% |           -1.0743% |
| eval / NVVM O3    |                   1.6512% |               0.3491% |           -0.8786% |           +0.0042% |
| sample / NVRTC O3 |                   2.0624% |               0.5293% |           -0.5488% |           -0.4873% |
| sample / NVVM O0  |                   2.1757% |               0.6096% |           -0.0589% |           -0.2722% |
| sample / NVVM O3  |                   2.3933% |               0.7385% |           -0.5593% |           -0.9177% |

Baseline pooled semantic medians are 409.095–411.640 ms; candidate medians are 400.345–402.930 ms.
The result is a modest observed timing difference in this experiment, not an accepted production
speedup. Both improvement gates fail. The candidate is discarded without adding a cache, selecting
a stronger TLS model, changing resolution, lowering thresholds or collecting favorable retries.
Conditional Debug/thread/epoch/late-loader semantic proofs and full runtime suites were not run,
because promotion had already failed. Do not infer that the discarded candidate passed those gates.

The first candidate identity audit assumed every artifact belonged to the relative build layout;
absolute toolkit ptxas caused a ValueError. The rollback guard rejected the incomplete audit before
any source edit. Its surrounding shell lacked set -e and ran a no-op candidate build; that attempt
is retained separately. The corrected audit checks absolute toolkit identities in place and copied
build artifacts inside their layouts, then passes. No measured run was repeated. Actual rollback
uses a fail-fast command and restores both original source files exactly before rebuilding.

Full256's 1707 runtime cells/1668 correct/39 unresolved and18 resolved histories remain inherited.
No fresh GPU runtime cells or material kernel performance are claimed. Full256/targeted233/cadence0
and rolling252/254/256 are unchanged. Material bindings/textures/LUT/input/output contracts remain
unavailable. The failed hypothesis does not justify forcing another speculative material optimization.

The native restoration build reproduces the accepted compiler and test libraries exactly. Its
regenerated builtin cache differs in two timestamp-prefix bytes and eight payload bytes; the latter
are retained without attributing them to a particular serialization field. The cache format binds
its uint64 timestamp to the compiler library's file modification time, as documented by
BuiltinModuleCache and the build packaging target. After proving compiler bytes identical, restore
the immutable accepted cache and saved compiler file timestamp together. Do not rewrite cache
bytes to mask the mismatch. A fresh loader control proves the accepted cache is read unchanged and
produces exact accepted PTX. Its logs were generated under build/loader-check and archived under
slice-258-restored/loader-check. The reused driver's generic “Both isolated” message is not the
inventory: this restoration control has exactly one compile, separately from the two paired controls.

Final acceptance verifies all133 source/12 tested artifact/565 input hashes and18 submodule pins
exact256, with no production diff. The rebuilt-cache attempt, exact differences, restored metadata
and four primary cache-contract snapshots are retained. Independent compact-evidence acceptance
recomputes every recorded phase distribution, in addition to the complete ordered timing audit.
There are282 baseline/candidate primary snapshots plus4 restoration-contract snapshots.

Closure verifies1129 indexed artifacts,286 primary snapshots and39 final compact references.
