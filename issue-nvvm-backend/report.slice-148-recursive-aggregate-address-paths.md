# Slice 148 report: canonical recursive parameter-group aggregate paths

## 1. Motivation

Consider this repository kernel:

```slang
struct MaterialSystem
{
    CB cb;
    RWStructuredBuffer<uint4> data;
}

struct Scene
{
    CB sceneCb;
    RWStructuredBuffer<uint4> data;
    ParameterBlock<MaterialSystem> material;
}

ParameterBlock<Scene> scene;

uint value = scene.sceneCb.value.x + scene.material.cb.value.x;
```

CUDA specialization and linking preserve each member access as a canonical `IRFieldAddress`. The
first address is rooted in the loaded `ParameterBlock<Scene>`; a later address uses the exact
pointer to `CB` or `MaterialSystem` produced by its parent field. Direct NVVM already knew how to
emit every individual struct-field pointer through the generic provider interface, but it rejected
the recursive chain when the parameter-group root was immutable. After that check was corrected,
the canonical CUDA layout query could not size a nested parameter-group field, the direct storage
algebra did not admit all pointer-sized resource leaves in the enclosing struct, and reachable-type
collection did not retain the specialized nested element struct. These are one producer-to-consumer
representation cascade, not four unrelated fixture exceptions.

The Slice 147 discovery census measured seven healthy workloads at a `struct field address` first
blocker. This slice intentionally handles only the exact nested parameter-group/constant-buffer
representation proven by the two promoted kernels. Adjacent root roles, pointer-load chains, and
resource-layout shapes retain typed deterministic diagnostics for later slices.

## 2. Proposed solution

Use one root-derived recursive aggregate-address invariant:

1. `IRBuilder::emitFieldAddress` remains the only admitted producer.
2. The resolver recursively proves the parent path and uses the field key to find the declaration.
3. The parent's declared field type must equal the child's canonical base aggregate type.
4. The child inherits conventional-global provenance and mutability; field selection cannot widen
   access.
5. CUDA target layout represents a nested `ParameterBlock<T>` or `ConstantBuffer<T>` as one
   pointer-sized field, matching native CUDA emission, while recursive storage validation and
   reachable-type collection preserve and prove `T`.

Preflight and emission share the resolver and storage classifiers. The established
`emitStructFieldPointer`, typed load/store, struct declaration, and resource callbacks express the
result, so forward-only provider ABI revision 30 remains unchanged.

## 3. Change summary

- `source/slang/slang-emit-nvvm-type-lowering.cpp`
  - recursively validates specialized parameter-group element storage;
  - admits only already-supported raw buffers, surfaces, sampled textures, samplers, and
    device-copyable pointers as pointer-sized aggregate storage leaves.
- `source/slang/slang-emit-nvvm.cpp`
  - preserves root mutability and provenance through recursive field-address chains;
  - lays out selected parameter groups and resource leaves as pointer-sized fields;
  - includes nested parameter-group element structs in the reachable type closure;
  - retains the complete rejected field-address result type in diagnostics.
- `source/slang/slang-ir-layout.cpp`
  - teaches the canonical CUDA layout query that parameter groups use their emitted pointer
    representation; non-CUDA target paths are unchanged.
- `tests/bindings/nested-parameter-block-2.slang` and `tests/compute/cbuffer-legalize.slang`
  - add stable differential direct-NVVM O0/O3 lanes for nested parameter-block/resource and
    constant-buffer/resource combinations.
- `tools/slang-unit-test/unit-test-nvvm-*`
  - removes three obsolete negative sources rather than retaining now-supported storage as a
    failure or enlarging the legacy fake provider's hard-coded struct vocabulary.
- `issue-nvvm-backend/run-compute-census.py`
  - selects and verifies the exact frozen corpus-v1 ID/source rows so newly promoted discovery
    directives cannot change the historical 452-workload denominator.
- The census summarizers recognize the richer typed field-address diagnostic without changing
  producer ownership or merging the two corpus denominators.

## 4. Concepts and vocabulary

- **Aggregate storage algebra:** the finite recursive family of types that direct NVVM can lay out
  and transport without reconstructing source syntax.
- **Parameter-group storage leaf:** the pointer-sized field used by CUDA for a nested
  `ParameterBlock<T>` or `ConstantBuffer<T>`; `T` remains a separately declared reachable struct.
- **Root role:** the storage provenance and mutability established at the beginning of a pointer
  chain and inherited by every exact child selection.
- **Frozen selector:** an exact ID/source join against `census.slice-146.tsv`, not a new result
  filter; missing, duplicate, or source-drifted rows stop the census.

## 5. Process report

### The recursive field producer preserves access instead of creating it

`IRBuilder::emitFieldAddress` constructs each `IRFieldAddress`. During final preflight,
`_validateNVVMFunction` calls `_getNVVMStructFieldAddress`; emission calls the same resolver before
`NVVMIRBuilder::emitStructFieldPointer`. For a nested address, the resolver first resolves its
parent and then checks that the parent's declared field type is the exact aggregate used as the
child base. The previous `parentAddress.isMutable` condition incorrectly rejected a valid immutable
read. Replacing it with inheritance does not admit a new root or a structural type spelling: an
immutable parameter block remains immutable, and an already-proven mutable local or resource path
remains mutable.

The concrete shape is canonical and intentionally allowed. It is produced by ordinary member
access after CUDA parameter-group lowering, not by a malformed downstream alternative. The
semantic source of truth is the existing field declaration and canonical IR type. The resolver
does not reconstruct syntax, walk an arbitrary operand graph, or invent a type equivalence rule.
Removing the inheritance change makes `nested-parameter-block-2.slang` stop at its second nested
field address, proving this layer owns the recursive provenance check.

### CUDA layout owns the nested parameter-group field representation

Native CUDA emits the `Scene.material` field as a pointer such as
`MaterialSystem_0* material_0`. `IRTypeLayoutRules::calcSizeAndAlignment` is the canonical target
layout producer used to derive the enclosing field offsets. It previously had no CUDA case for
`ParameterBlockType` or `ConstantBufferType`, so direct NVVM could not prove that its provider
struct matched CUDA even though the emitted representation was already unambiguous.

The new case is limited to `isCUDATarget(targetReq)` and uses the target's generic pointer size for
both size and alignment. Other targets retain their existing layout rules. This is producer-side
canonicalization, not a direct-emitter fallback: every CUDA layout consumer now observes the same
representation that native CUDA emission uses. Both promoted tests fail layout/field validation if
this case is removed.

### Storage validation and reachable types preserve the pointee contract

A pointer-sized field is not sufficient by itself. `_isNVVMSupportedAggregateStorageType`
recursively validates the specialized element type behind a nested parameter group and uses its
active set to reject recursive storage cycles. The enclosing structs in the promoted tests also
contain writable buffers, textures, samplers, and value pointers. These are exact storage leaves
already admitted by their dedicated classifiers and emitted as CUDA pointer/handle fields; the
aggregate classifier now reuses those sources of truth instead of naming fixtures or resource
declarations.

`_addNVVMReachableStructTypes` unwraps the same admitted parameter-group type so provider struct
declarations include the specialized element before a field pointer refers to it. This is a finite
type dependency closure, not a graph search for missing context. Removing it causes the real
provider to lack the concrete nested element declaration. No adjacent unsupported resource or
pointer spelling is admitted: the type classifiers still reject it at preflight.

### Exact diagnostics separate later root causes

The old diagnostic reduced every unresolved `IRFieldAddress` to `struct field address`. The new
diagnostic retains the canonical result pointer type, for example
`struct field address result: Ptr<RWStructuredBuffer<int>, ...>`. Its producer remains
`IRBuilder::emitFieldAddress`, and both census summarizers map the typed prefix to the established
aggregate-pointer ownership cluster.

This does not weaken preflight. It shows that the remaining discovery rows use different root or
layout representations, while two other rows now advance to the independently measured
load-to-load device-pointer shape. Those rows are not counted as unlocked until they execute
correctly.

### Permanent lanes and the frozen census have separate jobs

`nested-parameter-block-2.slang` combines two parameter blocks, a nested parameter block, ordinary
struct fields, and writable structured buffers. `cbuffer-legalize.slang` combines a constant
buffer containing a struct, texture, and sampler with a helper call. Both use deterministic existing
inputs and expected output, have healthy native CUDA references, and are correct through the real
provider at O0 and O3. Their four direct lanes protect distinct semantic combinations and are not
fixture dispatch inputs.

Adding those directives means ordinary repository enumeration now finds more than the historical
452 direct workloads. Corpus v1 is nevertheless immutable. `--workload-ids-from` joins dynamic
discovery to every checked-in `id` and `source` from the frozen TSV, rejects duplicate/missing/drifted
rows, and executes only those exact identities. The manifest records both dynamically discovered
and selected counts. Discovery continues through its separate manifest and artifacts.

### Coverage and regression evidence

Frozen corpus v1 remains 452 workloads with a 427 healthy-MVP denominator:

| Frozen corpus-v1 metric | Slice 148 result |
|---|---:|
| Direct O0 correct | 371/427 (86.9%) |
| Direct O3 correct | 375/427 (87.8%) |
| Correct in both modes | 371/427 (86.9%) |
| Old-correct regressions | 0 |
| Selected NVVM regression prefix | 424/424 |

The separate 82-workload discovery set retains 72 healthy native references:

| Discovery metric | Slice 148 result |
|---|---:|
| Direct O0 correct | 47/72 (65.3%) |
| Direct O3 correct | 47/72 (65.3%) |
| Correct in both modes | 47/72 (65.3%) |
| Newly unlocked in both modes | 2 |

Across every selected discovery row, the complete classifications are:

| Route | Correct | Runtime mismatch | Slang NVVM preflight | Provider/libNVVM | Infrastructure/toolchain |
|---|---:|---:|---:|---:|---:|
| Native NVRTC O3 | 72 | 2 | 0 | 0 | 8 |
| Direct NVVM O0 | 47 | 1 | 26 | 1 | 7 |
| Direct NVVM O3 | 47 | 1 | 26 | 1 | 7 |

The healthy-reference failure Pareto is identical at O0 and O3:

| Canonical producer/type/operation cluster | Healthy rows blocked |
|---|---:|
| Device pointer produced by load and consumed by load | 4 |
| Typed aggregate struct-field pointer | 3 |
| Array-element pointer relation | 2 |
| Entry-point parameter ABI | 2 |
| Function identity | 2 |
| Helper aggregate parameter ABI | 2 |
| Helper pointer parameter ABI | 2 |
| Helper resource result ABI | 2 |
| Sequential aggregate pointer | 1 |
| Aggregate storage layout | 1 |
| AnyValue UInt64 reconstruction | 1 |
| Fixed-array value construction | 1 |
| Helper aggregate result ABI | 1 |
| Provider global-to-generic `UserPointer` cast | 1 |

Every row's complete shape, producer, diagnostic, log, and examples are retained in the separate
Slice 148 discovery TSV/JSON artifacts. Compared with Slice 147, the struct-field cluster falls
from seven healthy rows to three. Two rows become correct, while
`generic-shader-object-cbuffer.slang` and `tuple-parameter.slang` advance to the four-row
device-pointer-load cluster. The three other healthy field rows keep exact typed diagnostics; the
fourth selected row in that cluster lacks a healthy native reference.

Corpus v1 independently retains ten helper-ABI, nine other-preflight, seven aggregate/pointer,
six function-identity, and smaller clusters at O0/O3. One frozen failure,
`bugs/gh-5776.slang`, advances from field-address validation to its separate raw structured-buffer
numeric element-pointer blocker without becoming correct. The two corpora continue to identify
helper ABI and aggregate/pointer representation as the largest reusable families, but their counts
and denominators are not combined.

The two newly correct identities are `bindings/nested-parameter-block-2.slang` and
`compute/cbuffer-legalize.slang`; neither Slice 147 correct identity regresses. Corpus denominators
are not combined, and no corpus v2 is proposed.

### Performance and platform evidence remain exploratory

The three established larger discovery gates remain stable. For the two newly unlocked kernels,
three standalone compilations per configuration produce:

| Workload | NVRTC O3 median / PTX | Direct O0 SM70 median / PTX | Direct O3 SM70 median / PTX |
|---|---:|---:|---:|
| Nested parameter blocks/resources | 377.1 ms / 9179 B | 254.2 ms / 1391 B | 255.4 ms / 1166 B |
| Constant-buffer resource helper | 384.2 ms / 9005 B | 268.7 ms / 7355 B | 265.1 ms / 1032 B |

CUDA 12.9 `ptxas` accepts every measured direct O3 module at SM70, SM80, and SM90. It also accepts
the three established discovery gates at those architectures. The census end-to-end compile/load/
execute/compare times for the new workloads are approximately 5.4--6.3 seconds per lane and are not
kernel-only runtimes.

These are smoke measurements, not controlled benchmark claims. Runtime remains on the local GPU;
CUDA 13 and physical SM70/SM80/SM90 workers remain open productionization requirements.

### Self-review inventory

- Recursive provenance inheritance survives. Exact parent resolution and canonical field-type
  equality prove the child, while root mutability can only be preserved.
- CUDA parameter-group layout survives. It fixes the canonical target producer and is restricted to
  the representation native CUDA already emits.
- Recursive parameter-group storage validation survives. It reuses existing exact resource and
  pointer classifiers, retains specialized pointee proof, and rejects cycles.
- Reachable parameter-group element collection survives. It is the direct dependency implied by a
  provider field type and does not rediscover semantic context.
- The typed diagnostic survives. It changes no accepted shape and makes producer/type ownership
  auditable.
- The frozen selector survives. It enforces corpus-v1 identity instead of filtering by outcome.
- The three obsolete negatives remain removed. Two would require recreating arbitrary specialized
  LLVM types in the legacy hard-coded recorder; the fixed sampler-array source now compiles because
  sampler handles are deliberately supported CUDA storage leaves. Retaining any as expected
  failures would let stale test scaffolding contradict the production representation.

The diff introduces no custom AST/IR equivalence, syntax reconstruction, fixture-name check,
compatibility fallback, diagnostic weakening, or downstream repair of malformed upstream IR. Every
retained widening names its canonical producer and is covered by both promoted differential tests.
