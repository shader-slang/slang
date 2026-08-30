# Preserve UserPointer values across helper and aggregate boundaries

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct NVVM path accepts the canonical `AddressSpace::UserPointer` spelling
for selected copyable pointees across helper parameters/results, local storage, finite arrays and
structs, AnyValue pointer-bit transport, ordinary dereferences, and conventional global parameter
storage. The compiler preserves producer-proven global address space during ordinary execution
and widens to LLVM generic address space only at a helper-value boundary that can also carry a
local address.

The fixed 452-workload census, not the selected regression prefix, measures the result. Every
newly correct workload receives explicit direct O0 and O3 runtime lanes.

## Progress

- [x] (2026-08-30) Repartitioned the Slice 138 helper/type failures by exact linked signature and
  selected the `UserPointer`/finite-aggregate family.
- [x] (2026-08-30) Traced the canonical producer through CUDA address-space specialization,
  AnyValue packing, helper calls, conventional global storage, and local helper storage.
- [x] (2026-08-30) Added a finite cycle-safe helper-value algebra, role-specific type lowering,
  provenance-sensitive global-to-generic widening, pointer bit transport, and reachable-type
  discovery.
- [x] (2026-08-30) Added the two concrete provider operations that revision 28 could not express,
  advanced the forward-only ABI to revision 29, and validated the LLVM implementation.
- [x] (2026-08-30) Promoted all 20 newly correct workloads with direct O0/O3 lanes and ran every
  affected fixture file.
- [x] (2026-08-30) Passed the 407/407 selected regression, regenerated the three-mode 452-workload
  census/Pareto evidence, measured representative gates, and assembled direct O3 PTX for
  SM70/SM80/SM90.

## Surprises and Discoveries

- `AddressSpace::UserPointer` is the 64-bit value `0x100000001`. The old diagnostic truncated it
  to 32 bits and made it look like `ThreadLocal`; the complete value proves the canonical CUDA
  specialization result.
- The initial 11-row dynamic-dispatch probe understated the family. The fixed census found 20
  newly correct workloads at both modes, including wide scalar intrinsics, struct bit casts, an
  optional existential, and a uniform-pointer kernel.
- Converting every kernel pointer to generic space at entry was semantically valid for loads and
  stores but erased producer-proven global information, regressed global PTX evidence, and made
  global atomic descriptors disagree with their operands. The correct invariant is boundary-only
  widening, not one generic representation for every use.
- Pointer-bearing helper structs were admitted before their field structs entered the selected
  reachable-type inventory. Deriving reachability from every admitted function and instruction
  type fixed the producer/consumer contract rather than adding a module-scope exception.
- AnyValue reconstructs a UInt64 pointer payload with an Int32 shift count. Slang preserves that
  count width; LLVM requires a physical same-width operand. The provider must normalize the
  already-classified count instead of rejecting the canonical mixed-width operation.

## Decision Log

- Decision: recognize only exact read-write `Ptr<T, UserPointer, DefaultBufferLayout>` whose
  pointee is an already selected finite copyable value.
  Rationale: the complete linked pointer type is the semantic source of truth. Adjacent address
  spaces, access qualifiers, layouts, resources, and recursive storage graphs remain separate
  clusters.
  Date/author: 2026-08-30, Codex.
- Decision: model helper values as a finite recursive algebra of selected copyable leaves, exact
  UserPointer leaves, nonempty fixed arrays, and nonempty structs, with cycle detection.
  Rationale: this is the common canonical representation produced by dynamic dispatch and
  AnyValue lowering; following pointees recursively would admit arbitrary storage graphs.
  Date/author: 2026-08-30, Codex.
- Decision: preserve AS1 for kernel parameters and pointers loaded from conventional global
  storage, and emit `addrspacecast` only when such a value enters helper transport.
  Rationale: the producer proves global provenance for ordinary execution, while helper values
  must also represent `__getAddress` of local storage. The boundary is where those valid origins
  intentionally meet.
  Date/author: 2026-08-30, Codex.
- Decision: advance the provider ABI from 28 to 29 with generic pointer-bitcast and pointer
  address-space-cast callbacks.
  Rationale: revision 28 has no operation for pointer-to-integer/integer-to-pointer transport or
  typed LLVM `addrspacecast`; both are concrete canonical operations exposed by this family.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The fixed corpus remains 452 workloads from 448 sources: 430 MVP and 22 extension. Native
CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures. Direct O0 is correct for
318, has eight runtime mismatches, 119 preflight failures, and seven provider failures. Direct O3
is correct for 323, has eight runtime mismatches, 119 preflight failures, and two provider
failures.

Compared with Slice 138, both direct modes gain the same 20 exact success identities and lose
none. Among the 427 healthy MVP references, O0 correctness is 317/427 (74.2%), O3 correctness is
321/427 (75.2%), and both-mode correctness is 317/427 (74.2%). The selected prefix passes 407/407
and remains a regression score rather than a coverage denominator.

The leading remaining healthy-MVP clusters are wave/reconvergence GenericAsm (19), helper ABI/type
contracts (16), aggregate/pointer/layout transport (14), ordinary numeric/bit operations (11),
residual target markers/undefined values (9), and atomic/wave operations (8). These measured
clusters, rather than slice count, determine the next priorities.

All 20 promoted files pass their native and explicit direct O0/O3 lanes. The three representative
MVP gates remain differentially correct. CUDA 12.9 `ptxas` accepts their direct O3 PTX for SM70,
SM80, and SM90; runtime comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical
SM70/SM80/SM90 workers remain productionization gaps.

## Context and Current Pipeline

Consider a dynamic-dispatch implementation that carries a device pointer:

```slang
struct IndirectNode : INode
{
    IValue* source;
    int evaluate() { return source->get(); }
}
```

CUDA specialization preserves the source device address as this final linked type:

```text
Ptr<IValue, addressSpace=UserPointer, access=ReadWrite, layout=DefaultBufferLayout>
```

AnyValue lowering may reinterpret that pointer as `UInt64` or `UInt2`, store it in a finite
aggregate, reconstruct it, pass the aggregate through a helper, and dereference the recovered
pointer. The direct path previously admitted pieces of this graph but rejected the helper
signature or aggregate before provider module creation.

## Scope and Non-Goals

In scope are exact UserPointer values with selected copyable pointees; finite pointer-bearing
helper arrays/structs; helper parameters, results, calls, returns, phis, locals, loads/stores,
aggregate construction/extraction, null pointers, pointer bit transport, and producer-proven
global-to-generic widening.

Out of scope are arbitrary local/shared pointer types, atomic `RefParam` contracts, resource-object
helper results, append/consume buffers, recursive storage graphs, arbitrary integer-pointer casts,
OptiX, RDC/device LTO, dynamic parallelism, device syscalls, FP8, advanced waves, and debugging.

## Architecture and Invariants

- The complete post-specialization `IRPtrTypeBase` is authoritative: opcode, four operands,
  read-write access, 64-bit UserPointer address space, default layout, and selected pointee must
  all match.
- Helper values form one finite cycle-safe algebra. Pointer leaves terminate recursion and have
  eight-byte alignment.
- Entry parameters and conventional-global pointer fields lower to LLVM AS1. Helper pointer
  leaves lower to AS0 because a helper can consume either those global values or local addresses.
- A producer-proven AS1 value is widened once and cached at the helper-value boundary. Ordinary
  memory operations retain AS1 and continue to emit global PTX operations.
- Pointer bit transport admits exactly UInt64 and UInt2 payloads. It does not infer an
  integer-pointer conversion from arbitrary numeric bit casts.
- Reachable type discovery starts from every admitted function result, parameter, instruction
  result, and local helper pointee. Module-scope declarations are checked against that inventory.
- Capability collection validates the complete accepted closure before provider module creation.

## Interfaces and Dependencies

Primary compiler work is in `slang-emit-nvvm-type-lowering.*` and `slang-emit-nvvm.cpp`.
Forward-only ABI revision 29 adds `emitBitCast` and `emitPointerAddressSpaceCast` to the existing
construction interface. The isolated LLVM 14 provider implements exact `ptrtoint`/`inttoptr`,
UInt2/i64 reinterpretation, and typed `addrspacecast`; it also normalizes canonical mixed-width
integer shift counts after semantic classification.

Validation uses Release host/provider builds outside the sandbox, the 407-test selected prefix,
the fixed 452-workload census, CUDA 12.9.86, the local RTX 5090/SM120 runtime, and CUDA 12.9
`ptxas` for SM70/SM80/SM90.

## Validation and Acceptance

Acceptance requires Release provider/host/unit builds; real-provider pointer-bitcast and
address-space-cast positive/negative tests; all promoted O0/O3 runtime lanes; 407/407 selected
regression; regenerated 452-row census and Pareto artifacts with no old-correct regression; three
representative compile/PTX/runtime measurements; SM70/SM80/SM90 assembly; formatting;
`git diff --check`; and the repository input-shape audit.

## Failure and Recovery

If a shape has a different address space, access, layout, pointee class, recursive topology, or
producer role, retain its first diagnostic for a later cluster. Do not erase provenance, rebuild
syntax, add fixture checks, infer compatibility from layout alone, or patch malformed downstream
IR.

## Self-Review

The new-helper inventory is: exact UserPointer classifier; recursive helper-value classifier and
alignment; helper array/struct/local-pointer classifiers; pointer-bitcast resolver; null-pointer
materializer; provenance-sensitive helper-value materializer; recursive reachable-type collector;
provider pointer-bit-pattern predicate; and fake-provider typed cast records. Each consumes a
canonical linked type or producer-owned value. No helper walks arbitrary syntax or witness graphs,
uses a fixture name, reconstructs source syntax, defines custom semantic equivalence, or keeps a
compatibility fallback.

The rejected entry-wide AS1-to-AS0 conversion was removed after the revert drill showed regressions
in ordinary global stores and atomics. Boundary-only widening is retained because removing it makes
the newly promoted helper/AnyValue fixtures fail exact provider type checking, while the 407-test
regression proves ordinary AS1 execution remains intact.

## Artifacts and Hand-Off

Committed evidence is `census.slice-139.tsv`, `census.slice-139-clusters.json`, the Slice 139
report, durable design status, and the promoted fixture directives. Raw logs, LLVM/NVVM IR, PTX,
cubins, and timing samples remain under ignored `build/nvvm-census/slice139-*`.
