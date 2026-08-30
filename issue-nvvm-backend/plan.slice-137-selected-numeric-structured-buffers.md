# Generalize numeric structured buffers and the physical Half helper ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, one compiler-owned resource-element contract admits every already-selected
numeric scalar and two-, three-, or four-lane vector as a canonical `StructuredBuffer<T>` or
`RWStructuredBuffer<T>` element when its CUDA external layout agrees with the provider
representation. This removes the older 32-bit-only resource gate for `half`, `double`, and their
vectors without adding per-type emission paths. Existing generic resource-view, pointer-offset,
load, store, aggregate, and call operations remain authoritative.

The measured resource widening exposes two further parts of the same numeric transport boundary.
Canonical Boolean-to-Half/Double conversions use the existing typed integer-to-float operation,
and scalar Half helper parameters/results cross LLVM calls as physical i16 bits while helper bodies
retain canonical Half values. This prevents libNVVM O3 from omitting the caller parameter store.
All three changes use existing generic revision-27 operations, so the isolated LLVM 14 provider and
builder ABI remain unchanged.

The fixed 452-workload census is the denominator. Every workload that becomes differentially
correct at both direct O0 and O3 is promoted beside its native CUDA lane. Workloads that merely
advance to a later unsupported canonical shape are reported under that new root cause and are not
counted as coverage gains.

## Progress

- [x] (2026-08-30) Committed Slice 136 as `17b63cb`; the fixed census records 276 direct-O0 and
  272 direct-O3 successes with no old-correct regression.
- [x] (2026-08-30) Repartitioned the leading helper and aggregate clusters by exact final IR type.
  The 28-row helper bucket is heterogeneous; the adjacent structured-buffer failures expose one
  coherent older policy gate shared by conventional half/double workloads.
- [x] (2026-08-30) Dumped final linked IR for `bugs/frexp-double.slang`. CUDA resource lowering
  produces `RWStructuredBuffer<Double>` in the synthesized `GlobalParams` block, a keyed
  `get_field_addr`, `rwstructuredBufferGetElementPtr`, and typed `Double` load. The first direct
  failure is the field address because `_isNVVMSupportedResourceElementType` still excludes scalar
  Double.
- [x] (2026-08-30) Added focused positive and negative coverage for selected half/double
  scalar/vector resource views and layout-incompatible neighbors.
- [x] (2026-08-30) Replaced the 32-bit resource-element subset with the existing selected-numeric
  value classifier and enforced the external layout invariant at every raw structured-buffer
  boundary.
- [x] (2026-08-30) Measured newly reachable conversion helpers: Boolean conversion was a missing
  shared-family row, while O3 PTX omitted every caller-side `st.param.b16` for direct Half helper
  parameters.
- [x] (2026-08-30) Legalized scalar Half helper parameters/results to an i16 physical call ABI,
  routed every ordinary and specialized helper return through one boundary, and retained canonical
  Half within function bodies.
- [x] (2026-08-30) Ran the affected census family at O0/O3, audited every transition, promoted only
  differential successes, then regenerated the fixed census and Pareto artifacts.
- [x] (2026-08-30) Formatted, built Release host/provider outside the sandbox, ran focused,
  promoted, selected regression, representative, PTX/SM70/80/90, and diff/self-review gates, then
  committed the slice.

## Surprises and Discoveries

- The current entry-point unit suite deliberately rejects `RWStructuredBuffer<double>` and
  `RWStructuredBuffer<double2>` even though Slice 132 generalized Double values and generic
  aggregate transport, and later slices generalized their arithmetic. Those negatives now encode
  a bring-up-era resource policy rather than a provider limitation.
- `getNVVMResourceValueAlignment`, `_getNVVMStructuredBufferLoad`, raw resource lowering, and
  pointer-offset/load/store emission are already type-generic. The remaining admission function
  duplicates a narrower list of integer, Float32, and 32-bit vector leaves.
- External structured-buffer memory is not an internal helper ABI. A broader type classifier is
  insufficient by itself: every newly admitted element must prove that CUDA size/alignment and the
  LLVM provider representation agree before module mutation.
- `conversion-to-half.slang` and `conversion-to-double.slang` became correct at O0 after resource
  and Boolean-conversion admission but mismatched at O3. Comparing emitted PTX showed that every
  direct Half helper call declared a 32-bit parameter slot, the callee loaded 16 bits, and O3
  omitted the caller's 16-bit store entirely. O0 emitted the store and ran correctly.
- The first helper-return implementation covered ordinary `IRReturn`, but the full census found
  two specialized GenericAsm helpers returning directly. Consolidating all non-void helper returns
  restored both old-correct identities and removed a producer-specific ABI drift.

## Decision Log

- Decision: bound Slice 137 to selected numeric structured-buffer elements rather than the entire
  28-row helper-signature cluster.
  Rationale: the helper rows span unrelated pointer, existential, resource, ref-parameter, and
  deferred scalar representations. The numeric resource family has one canonical producer and one
  generic representation used by ordinary compute kernels.
  Date/author: 2026-08-30, Codex.
- Decision: reuse `isNVVMSupportedNumericValueType` instead of enumerating half/double overloads.
  Rationale: it is already the source of truth for the scalar/vector value algebra that generic
  type lowering and memory operations can represent. A second resource-specific type table would
  drift again.
  Date/author: 2026-08-30, Codex.
- Decision: validate resource elements at the external storage boundary.
  Rationale: producer-side resource lowering supplies the exact element type and stride contract;
  direct emission may use its ordinary LLVM representation only when CUDA and provider layouts
  agree. Incompatible aggregates and Boolean storage must retain deterministic preflight failures.
  Date/author: 2026-08-30, Codex.
- Decision: keep provider ABI revision 27.
  Rationale: existing generic type, resource-view, aggregate extraction, pointer offset, load,
  store, call, and return operations fully express the measured IR. Classification and layout
  legality belong on the compiler side.
  Date/author: 2026-08-30, Codex.
- Decision: represent only scalar Half helper boundaries as physical i16 and bit-reinterpret at
  calls, entries, and returns.
  Rationale: the final linked helper signature is valid canonical Slang IR, but measured libNVVM O3
  PTX fails to transport direct Half arguments. Existing generic integer types and typed
  bit-reinterpret operations exactly express the target ABI without changing source IR or adding a
  provider callback.
  Date/author: 2026-08-30, Codex.
- Decision: route every non-void helper producer through `_emitNVVMFunctionValueReturn`.
  Rationale: ordinary IR returns, value GenericAsm, surface/texture helpers, and compound recipes
  all return values from canonical helper bodies. A single physical boundary prevents specialized
  producers from diverging from the declared helper type.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

Eleven existing conversion workloads are now correct at both direct O0 and O3 and receive 22
direct regression lanes. Three were previously preflight failures in both modes; eight integer
conversion fixtures were already correct at O0 but mismatched at O3 because each passes a Half
input through a noinline helper. No Slice-136 correct identity regresses.

The fixed 452-workload census reaches 279 correct at O0 and 283 at O3. Against 427 healthy native
MVP references, O0/O3/both correctness is 278/281/278, or 65.1%/65.8%/65.1%. Six former aggregate
field-address failures advance to their actual GenericAsm, `unmodified`, or conversion blockers;
the aggregate/pointer/layout cluster falls from 23 to 17. The three representative gates remain
correct and their direct O3 PTX assembles for SM70, SM80, and SM90 with CUDA 12.9.

Self-review keeps four principled changes: selected numeric resource admission uses the existing
numeric algebra; external resource layout is checked at each raw-buffer boundary; Boolean is a
valid canonical integer-to-float source; and physical Half transport is selected solely from final
helper signatures. There is no fixture dispatch, syntax reconstruction, compatibility fallback,
malformed-upstream repair, provider callback, or ABI revision.

## Context and Current Pipeline

Consider this existing corpus source:

```slang
RWStructuredBuffer<double> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    int exponent;
    frexp(inputBuffer[i], exponent);
    outputBuffer[i] = exponent;
}
```

After specialization, global-parameter collection, and linking, the CUDA path contains a
synthesized `GlobalParams` constant buffer with a keyed `RWStructuredBuffer<Double>` field. The
entry body uses `get_field_addr`, loads the resource view, produces an
`rwstructuredBufferGetElementPtr`, and loads a `Double`. Those are canonical operations produced by
ordinary resource and l-value lowering; no syntax recovery is needed.

`getNVVMSupportedRawBufferType` currently routes the element through
`_isNVVMSupportedResourceElementType`, whose scalar/vector leaves are limited to integers,
Float32, and 32-bit numeric vectors. This prevents `_getNVVMConventionalGlobalParams` from owning
the collected block and makes the first keyed field address fail. Once admitted,
`NVVMTypeLoweringContext::_lowerRawBufferType` creates the existing pointer/count view,
`_getNVVMStructuredBufferLoad` or `RWStructuredBufferGetElementPtr` validates typed access, and
`_emitNVVMModule` emits generic pointer offsets and aligned loads/stores.

## Scope and Non-Goals

In scope are selected Half/Float/Double and signed/unsigned integer scalar/vector structured-buffer
elements; raw and conventional resource-view classification; exact typed read-only loads and
read-write pointers/loads/stores; CUDA/provider layout comparison at entry, helper, and collected
global resource boundaries; canonical Boolean-to-floating conversion; scalar Half helper physical
parameter/result transport; focused fake and real coverage; promotion of newly correct corpus
workloads; and fixed-census/Pareto/representative metrics.

Out of scope are Boolean resource elements, matrices that do not already lower to an admitted
finite physical aggregate, arbitrary resource-containing recursive structs, compatibility
fallbacks, fixture-name checks, syntax reconstruction, atomics beyond the existing typed catalog,
new helper pointer families, existential/dynamic-dispatch transport, FP8/BFloat16, and provider ABI
revision 28.

## Architecture and Invariants

- Final linked resource types are canonical. Admission depends only on the exact structured-buffer
  type and its selected element, never a fixture, source intrinsic, or mangled name.
- `isNVVMSupportedNumericValueType` is the single scalar/vector leaf algebra. Resource structs and
  physical aggregate forms retain their existing recursive contracts.
- A newly admitted external element must have matching CUDA and provider size/alignment. Struct
  fields and arrays additionally preserve their existing recursive offset/stride checks.
- A resource view remains `{ element addrspace(1)*, i64 count }`; all pointer arithmetic is typed
  by the exact element representation, and memory alignment comes from the admitted value.
- Preflight, requirement collection, type lowering, pointer validation, and emission derive from
  the same raw-buffer classifier. Unsupported layouts fail before provider discovery or module
  creation.

## Interfaces and Dependencies

Production changes are in `source/slang/slang-emit-nvvm-type-lowering.cpp`, the shared semantic
catalog, and
`source/slang/slang-emit-nvvm.cpp`. Focused fake-provider coverage belongs in
`tools/slang-unit-test/unit-test-nvvm-{support,emitter}.h/.cpp`; existing old negatives should be
converted or narrowed rather than duplicated. No public header or `source/slang-llvm-nvvm` change
is required.

Validation uses the existing Release host build, isolated Release provider, CUDA 12.9 runtime on
the local SM120 GPU, and `ptxas` assembly for SM70/SM80/SM90. CUDA 13 and physical SM70/80/90
runtime gaps remain explicit productionization work.

## Milestones

1. Turn the old scalar-Double and Double-vector resource negatives into focused positive topology
   tests, while retaining incompatible layout and unsupported Boolean/recursive neighbors.
2. Generalize the one resource-element classifier and add a shared raw-buffer external-layout
   check used for conventional globals, entry parameters, and helper parameters.
3. Build and run focused unit/runtime probes. Audit every original aggregate `struct field address`
   row plus the numeric-resource family, recording new first failures without widening the slice.
4. Promote every workload correct at both O0/O3, regenerate the complete census/Pareto evidence,
   and compare exact success identities against Slice 136.
5. Update durable design/ledger/report evidence, run representative compile/PTX/runtime metrics and
   SM70/80/90 assembly, complete the self-review, and commit.

## Validation and Acceptance

Acceptance requires focused fake-provider topology for Half/Double scalar/vector resources;
deterministic pre-provider rejection for incompatible external layout and deferred resource leaves;
Release host and isolated provider builds outside the sandbox; differential CUDA runtime success at
O0/O3 for every promoted fixture; no Slice-136 correct-workload regression; complete selected NVVM
prefix success; representative workload success and compile/PTX metrics; CUDA 12.9 `ptxas`
acceptance for direct O3 SM70/SM80/SM90; formatting; `git diff --check`; and an input-shape audit of
every new helper or widened classification.

The 405/405 selected prefix remains a regression score, not the coverage denominator. The report
must use the fixed 452-row census and 427 healthy-MVP denominator.

## Failure and Recovery

If a newly admitted buffer reaches an unsupported operation, classify that next producer and leave
it for a later vertical slice unless generic transport itself is incomplete. If CUDA and LLVM
layouts disagree, retain preflight rejection rather than packing/reconstructing values downstream.
If LLVM verification or libNVVM rejects a supposedly generic topology, preserve its IR/provider
diagnostic below ignored `build/nvvm-census/`, narrow the contract, and add a negative unit test.

## Self-Review

Inventory every new helper, widening, and special case. For each, record the exact final IR shape,
producer, why the shape is canonical, the test that fails without it, and why this layer owns it.
Remove duplicated type lists, fixture/source-name checks, syntax recovery, custom equivalence,
compatibility fallbacks, and any downstream repair of malformed IR. Perform a revert drill on the
central classifier when practical.

## Artifacts and Hand-Off

Keep final-IR dumps, focused logs, PTX, cubins, raw census results, and timing samples under ignored
`build/nvvm-census/slice137-*`. Commit this completed plan with implementation, focused tests,
fixture promotions, regenerated census TSV/Pareto JSON, report, and durable design/ledger updates.
The outcome must quantify exact O0/O3 gains, later blockers exposed, and zero-regression evidence.
