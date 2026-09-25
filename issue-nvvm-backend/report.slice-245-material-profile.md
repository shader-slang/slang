# Slice245: profile material compilation and select the AST cast boundary

## Motivation

The intact tiled-brass material passes six compile/assembly cells but has no runtime bindings,
texture/LUT inputs or output oracle. Rolling241/242/244 addressed correctness instead of the complex
corpus. Compile-time work is independently measurable now. Single-sample checkpoint timings could
identify a broad phase, but could not justify choosing an optimization or claiming a speedup.

For example, `make_surface_interaction` in the material normalizes a direction and calls generic
`dot`; both entry points compile the surrounding material's overloaded/generic functions. The full
helper and producer-to-consumer explanation appear in the
[design note](../docs/design/nvvm-material-compile-time.md). No source reduction or runtime oracle
substitution was used.

## Proposed solution

Measure all six unchanged identities with two opposite-order fresh-process rounds, each using2
warmups and9 samples per identity. Retain nested phase distributions and independently measure
assembly. Use separate qualitative debugger snapshots to narrow the dominant semantic-checking
interval. Select the canonical NodeBase subtype-test path for a future prototype; implement no
optimization in this research slice.

The measured SemanticChecking medians are651.5–654.9ms. `SyntaxClassBase::isSubClassOf` is the most
frequent leaf in all three completed semantic-only debugger runs. Existing NodeBase tags already
encode the class; the current cast path converts that tag through a class-info table before testing
the same generated range. The next experiment should compare inlining with removing that roundtrip
while retaining one canonical predicate and hierarchy. No speedup is established yet.

## Change summary

- The [completed plan](plan.slice-245-material-profile.md) records scope, protocol correction,
  discoveries and acceptance obligations.
- [Compact timing evidence](timing-evidence.slice-245.json) records distributions, all six artifact
  hashes, exact identity checks, raw evidence references and inherited runtime status.
- The [design note](../docs/design/nvvm-material-compile-time.md) records timer ownership, source
  traces and the bounded next hypothesis with explicit semantic/performance gates.
- STATUS records the accepted research handoff. Accepted implementation/full checkpoint244, targeted233
  and implementation cadence0 remain authoritative.

No compiler, provider, runner, material, runtime fixture or build configuration changed. Production
helper/fallback/special-case inventory is empty. Local research scripts and source snapshots remain
under ignored `build/nvvm-loop/slice-245-material-profile`; historical evidence is immutable.

## Concepts and vocabulary

- **Inclusive timer:** elapsed time includes nested scopes; totals with different names can overlap.
- **Canonical node tag:** ASTBuilder's existing `ASTNodeType` value on every initialized NodeBase.
- **Generated class interval:** Fiddle's `firstTag/tagCount` describes the same existing AST hierarchy.
- **Residual:** a subtraction of documented enclosing intervals; it does not identify the unmeasured
  functions inside the remaining interval.

## Process report

The starting branch is nvvm-backend at8d53504112617efda0e3446f7b3117bcc2f77fd2. Native Ubuntu24.04,
RelWithDebInfo, providerABI40, CUDA12.9.2/NVRTC12.9.86 and L4SM89 driver580.126.09 targetSM80 match244.
Compiler SHA256 `b9e87811c263f131cf1372b307bd60acdcc51993aaab2af91a0cfd03462d10a1`;
provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`.
Both inspected environment helper layers were read before sourcing; executable and builder paths
select the optimized configuration explicitly. No build or system change occurred.

The raw `run.sh` and `measure.py` preserve exact reproduction. From the repository root:

```bash
source build/nvvm-loop/slice-203-env.sh
timeout --kill-after=30s 30m python3 build/nvvm-loop/slice-245-material-profile/measure.py
```

A replay must use a new raw directory, preserving this evidence. The driver imports the existing
`compile_command`, with exactly the accepted source/entry/stage/target/capability/optimization/
backend/performance options; only output paths differ. Each child has a180-second bound. It uses
piped `Popen.communicate` and `perf_counter` through process exit, writing captured logs afterward.
All measurements are serial with no competing builds/benchmarks. Warmups remain recorded.

An initial file-redirection `subprocess.run(timeout=...)` pilot had polling completion waits. Source
inspection caught the measurement-method error; the owned process was stopped, its43 recorded
completed attempts and one interrupted unrecorded attempt were retained in `excluded-polling-pilot`,
and every pilot observation was excluded irrespective of value. The definitive run has no failed,
timed-out or excluded attempts. No timing-based retries or outlier removal occurred.

| Identity          | Wall median, ms | Inclusive IQR, ms |  Full range, ms | Round1 / round2 medians, ms |
| ----------------- | --------------: | ----------------: | --------------: | --------------------------: |
| eval / NVRTC O3   |         1624.80 |   1616.52–1629.37 | 1603.40–1643.42 |           1617.18 / 1629.28 |
| eval / NVVM O0    |         1596.77 |   1592.28–1613.68 | 1580.07–1670.71 |           1595.45 / 1598.25 |
| eval / NVVM O3    |         1691.05 |   1690.07–1702.48 | 1676.36–1742.16 |           1690.23 / 1691.32 |
| sample / NVRTC O3 |         1647.47 |   1642.27–1652.54 | 1634.34–1721.39 |           1642.44 / 1647.86 |
| sample / NVVM O0  |         1640.20 |   1632.79–1652.00 | 1629.13–1678.57 |           1639.44 / 1640.97 |
| sample / NVVM O3  |         1792.80 |   1789.31–1798.32 | 1783.81–1828.95 |           1789.92 / 1797.43 |

Semantic checking contributes36.4–40.9% of per-identity wall time. Its `checkAllTranslationUnits`
child is essentially the same interval, just as `compileInner` wraps `endToEndActions`. IR generation
is184.4–185.1ms; builtin loading206.1–210.3ms; specialization138.3–144.5ms; cumulative
simplification121.8–128.7ms. The design note identifies the exact nesting and multiple invocations.
These totals are not additive.

The median output-minus-link/optimize residual is129.6–316.2ms, containing downstream and other
unmeasured output work. The fresh lifecycle residual after builtins/compileInner is68.9–76.5ms,
containing CLI handling, profiler output, teardown and process overhead. Neither is isolated
provider or process startup time. Direct independent assembly medians range178.2–882.2ms. O0
assembly costs more for these artifacts; changing shader optimization settings would change the
workload and is not the selected hypothesis. Historical214 shared-session results remain inherited;
this research neither remeasures them nor reads aggregate profiler totals as per-request phases.

Local `perf` cannot run at perf_event_paranoid4. No permission or system configuration was changed.
Separate seeded GDB interrupts between `checkAllTranslationUnits` and `generateIR` collect121,118
and119 stacks in three completed eval/NVVM O3 compilations, whose PTX exactly matches244.
The class-test leaf appears19,22 and22 times; `getClass` appears6,6 and8 times. These are qualitative
observations, not CPU fractions. An auxiliary seed247 collected117 semantic stacks but its compile
stopped on a pending SIGINT after the endpoint; its raw log/stacks remain visible and outside the
completed-run summary. The final local sampler drains pending interrupts through exit; independent
seed248 completes. No debugger timing contributes to the benchmark distributions.

One observed call chain goes from declaration/body checking through `ResolveInvoke`, overload
candidate constraints and `GenericArgumentSolver::solve` to `TryJoinTypes`, `as<DeclRefType>` and
`dynamicCast`, then `NodeBase::getClass`/`SyntaxClassBase::isSubClassOf`. The valid input is an
ASTBuilder-created node whose canonical `astNodeType` was initialized by `_initAndAdd`. Generated
`kType`, `firstTag/tagCount` and `kAllSyntaxClasses` already agree on the hierarchy. The constructor
maps a tag to metadata; the existing constant-time subtype check reads the tag back to compare the
range. This is legitimate canonical input, not a malformed shape requiring producer repair.
No syntax reconstruction, custom semantic equivalence or new hierarchy is warranted.

The next prototype must preserve all registered tag/class pairs, including abstract classes, null
pointers, const overloads, invalid/default policy, `DeclRefBase` cast restrictions and serialized AST.
It must compare reusing/inlining the original operation against a direct canonical-tag path, never
maintaining a second classification. Relevant non-NVVM semantic tests and a full frozen/discovery
checkpoint are required because AST casting affects the whole front end. Proposed future gates
are5% lower semantic median in each identity,2% reduction in the sum of the six per-identity fresh wall medians (18 samples each) and no
identity wall regression above2% in either order round, with exact artifacts. These are gates for a
future experiment, not an estimated speedup. Reject the prototype if evidence does not support it.

| Preservation gate                    | Result                                                                   |
| ------------------------------------ | ------------------------------------------------------------------------ |
| Source/binary/runtime-input identity | 43/12/561 exact244 before and after                                      |
| Primary inventory                    | Exact6 identities; no duplicates, omissions or extra cells               |
| Compile attempts                     | 132 passed:108 measured,24 warmups; all PTX exact244; entry/SM80 checked |
| Assembly attempts                    | 66 passed:54 measured,12 warmups; all cubins exact244                    |
| Material runtime                     | Not executed; application contract still absent                          |
| Inherited full244                    | 1695 cells:1654 correct,41 unresolved;16 resolved histories retained     |
| Inherited cadence                    | Latest targeted233; implementation slices since full244:0                |

This unchanged-source research needs no new GPU suite or full-corpus replay. No correctness outcome,
failure history, support status or source oracle changed. Parent owns independent acceptance and
the local commit; no worker commit, push, driver change or reboot occurred.

Independent parent acceptance recomputes all sample inventories and distributions, including residuals,
checks every PTX/cubin against244, verifies43/12/561 current identities,36 compact references,528 raw
indexed artifacts and15 source snapshots, and validates the qualitative-run counts and exclusions.
Four parent audit artifacts are retained in the raw root and linked by the compact evidence.
