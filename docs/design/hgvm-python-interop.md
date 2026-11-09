# HGVM Python Interop and Shared Staging Plan

Status: managed-value implementation and focused validation complete, 23 October 2026.

Implementation follows lowered runtime type identity, including compiler-erased distinctions.
The pure-value API and shared staging changes completed their first validation on 21 October 2026:
31 focused compiler/native/debugger tests, 16 Python tests, and five additional checks with event
tracking enabled. The Point example also runs successfully. Broad repository suites were skipped at the
user's request. The managed-value extension described below reuses that ownership and staging
infrastructure. It passes 24 focused native tests with allocation tracking enabled, 19 Python
tests, and 37 browser assertions against the rebuilt WASM package, including WebGPU and JSPI.
The GPU-value example extension also passes the native buffer ownership/debugger regression and
44 browser assertions, plus an end-to-end run of the example's animation, resize, and shutdown.
Python device initialization was added on 26 October 2026: `backend` defaults to `"none"`,
`"default"` delegates to RHI, and explicit backends use the existing device factory. The interpreter
owns device lifetime, and GPU values use the same native ownership path as CPU references.
All 25 Python integration tests pass with Vulkan; the four GPU tests also pass with D3D12.
The Point, class-reference, and GPU-buffer demos run successfully, including default device selection.

Extend the existing scalar invocation API so a Slang function can return an opaque Python value
that another function in the same VM can consume. Stage loaded programs into shared VM storage,
intern nominal types by their canonical specialized mangled names, and use the address of each
canonical type record as its runtime identity. Resolve names and HGBC IDs during loading and
staging; ordinary invocation compares canonical addresses and uses existing argument transfers.

The first milestone supports pure value structs. The next extends the same ownership and identity
model to strings, class references, runtime arrays, and aggregates containing them. Python and
browser JavaScript remain thin adapters over the native invocation API used by `slangi`, with a
direct C++ usage path.

IR, HGBC, and internal C++ representations below are schematic unless explicitly identified as
current code. The public C++ invocation signatures below match the implementation. Symbolic names such as `M_Point` stand
for real compiler-generated mangled strings; they are not proposed literal linkage spellings.

## Scope and decisions

| Area | Decision |
| --- | --- |
| Python entry point | Preserve `import slangvm`, `vm = slangvm.init(settings)`, and `m = vm.load(path)`. |
| Scalars | Preserve current conversions, range checks, return values, and synchronous behavior. |
| Struct values | Return an opaque `slangvm.Value` carrying native storage and canonical type identity. |
| Python inspection | Provide `type_name` and a useful `repr`; field access and Python constructors are unnecessary. |
| Cross-program use | Accept values in another loaded program when both use the same canonical type. |
| Cross-VM use | Reject opaque values from another VM initially, including pure structs, for one consistent contract. |
| Runtime type identity | A VM-relative address of a canonical staged type record. No independent numeric type-ID allocator. |
| Load-time nominal identity | A canonical, fully specialized lowered runtime type name. |
| Layout calculation | Reuse the stager's VM layout rules and calculate each canonical layout once. |
| Closing a VM | Invalidate outstanding values and release their native payloads without waiting for Python GC. |
| Managed values | Preserve object aliases and use existing reference counting; no automatic deep copy. |
| Execution | Keep load, invocation, debugging mutations, and close serialized initially. |

Supported external values include scalars, strings, owning VM objects, nested structs, and
inline fixed arrays, vectors, and matrices whose transitive components are owning values.
Resources, boxed values, and tagged unions use the same managed-leaf retain/release path.
Eligibility follows the staged layout rather than a resource-kind allowlist: address layouts
are borrowed, and unsupported fields or typed payloads propagate to a fixed point during staging.
Safe recursive class/array graphs are permitted; a borrowed
pointer anywhere in such a graph rejects the enclosing type. No object-graph traversal is needed
on each call. An empty managed-reference-offset list is insufficient to establish safety because
raw pointers also have no owning reference-counted leaves.

Generic functions, overload selection, `out`/`inout`/`ref` parameters, Python callbacks, mutable
field views, cross-VM serialization, hot reload, and individual program-memory reclamation are
separate features. A concrete host function may still use a specialized generic value type.
The original goal of exposing module methods and types as members can be extended later; this
milestone needs exported host callables and opaque results, not generated Python struct classes.

## Example behavior

The shared declaration lives in a module imported by both programs.

```slang
// geometry.slang
public struct Point
{
    public float x;
    public float y;
}
```

```slang
// producer.slang
import geometry;

[host]
public Point makePoint(float x, float y)
{
    return Point(x, y);
}
```

```slang
// consumer.slang
import geometry;

[host]
public float lengthSquared(Point p)
{
    return p.x * p.x + p.y * p.y;
}

[host]
public Point translate(Point p, float dx, float dy)
{
    return Point(p.x + dx, p.y + dy);
}
```

```python
import slangvm

vm = slangvm.init({"search_paths": ["./shaders"]})
producer = vm.load("./shaders/producer.slang")
p = producer.makePoint(3.0, 4.0)

# Loading another program must preserve addresses held by existing values.
consumer = vm.load("./shaders/consumer.slang")
print(p.type_name)                         # A readable name for geometry.Point
assert consumer.lengthSquared(p) == 25.0
q = consumer.translate(p, 1.0, 2.0)
assert consumer.lengthSquared(q) == 52.0
assert consumer.lengthSquared(p) == 25.0    # The original value is unchanged.

del producer                              # p remains valid while vm is open.
vm.close()
print(p.type_name)                         # Cached metadata remains printable.
# consumer.lengthSquared(p) raises RuntimeError: the VM is closed.
del p, q                                  # Safe after close; no VM-memory access.
```

Two independently declared structs named `Point` do not become interchangeable because their
fields look alike. Both programs above import the same nominal declaration. Python helpers that
need components can call additional Slang functions such as `getX(Point)`.

## Existing infrastructure and required extensions

The scalar implementation is already present in the working tree. It provides a reusable
`HGBCCompiler`, `Program` loading, scalar layout/conversion helpers, a shared `Interpreter::invoke`
path, and a nanobind extension. `slangi` and the debugger use the native convenience call path.

| Existing component | Extension |
| --- | --- |
| `HGBCCompiler` | Preserve lowered runtime type identity in compiled artifacts; maintain consistent module bindings and compiler settings. |
| `Program` | Decode `TypeMangledName` metadata while retaining ordinary HGBC-local IDs. |
| `Stager::getTypeLayout` | Separate canonical layout construction from per-program bindings and reuse layouts through a VM context. |
| `StagedRecordStorage` and builder | Support incremental, transactional appends to shared instruction/type/literal storage. |
| `StagedRecord::TypeInfo` | Make child/name references independent of the current program and preserve all allocation metadata needed for reuse. |
| `Interpreter` | Own shared staging state, bind functions once, and accept/return owned native values. |
| `BoxObject` storage logic | Share aligned payload copying and managed-leaf retain/release operations with external values. |
| Python `Module` and function wrappers | Cache native bound functions and wrap aggregate results in `Value`. |

This is not a new evaluator or a Python-specific call ABI. Existing frame initialization, call,
return, and cleanup operations remain the execution mechanism.

## Type identity from compilation through execution

### Three representations with distinct scopes

| Representation | Scope and use |
| --- | --- |
| HGBC type ID | Local to one bytecode program; used during decoding and staging. |
| Lowered runtime mangled name | Identifies a nominal type within the VM's consistent module-linkage domain; used for interning during loading. |
| Canonical type address | Identifies the one staged record in a particular live VM; used by values, signatures, instructions, and object headers. |

An address from another VM may have the same numeric bits. Every externally supplied value must
therefore be checked for VM ownership and liveness before comparing its type address.

### Lowered runtime identity and IR specialization

The boundary identifies lowered runtime types. Source distinctions deliberately erased by the
compiler are accepted as the same runtime type. For example, if enum lowering turns both
`Token<KindA>` and `Token<KindB>` into `Token<int>`, those values are interchangeable. This includes
phantom generic arguments. The VM does not reconstruct the original AST types.

Nominal distinctions that survive lowering must remain distinct. Existing specialized linkage
used `getTypeNameHint()` for some composite arguments, so `Wrapper<alpha.Item[2]>` and
`Wrapper<beta.Item[2]>` could receive the same linkage. Readable names are insufficient for identity.

The producer attaches `RuntimeTypeIdentity` metadata containing the unobfuscated declaration
mangled name. Generic cloning appends the substituted specialization operands to that metadata.
The HGBC emitter encodes these lowered operands with stable opcode names, explicit operand tags,
and length-delimited nominal names. The resulting runtime key retains the full encoding;
ordinary specialized code linkage may hash the same encoding. Debug names and obfuscation do
not determine runtime identity. Unknown or unresolved nominal identity remains program-local.

This reuses canonical lowered IR as its source of truth. No type-name parsing, AST reconstruction,
or alternate source-language type equivalence is needed. Direct and indirect instantiations
must produce the same lowered identity. Generic type/value arguments retain distinctions that
survive lowering, including nested nominal module qualification.

Ordinary nominal fields survive disjoint-field consumers and optimization. Empty host-boundary
types previously disappeared; preserve their transitive type graph before empty-type legalization
using the existing `BinaryInterfaceTypeDecoration` contract. This is an ABI preservation rule,
not a global optimization switch.

Schematic source and identity relationship:

```slang
public struct Sample<T, let N : int>
{
    T values[N];
}
```

```text
M_Point        = runtimeIdentity(loweredType(geometry.Point))
M_SampleFloat4 = runtimeIdentity(loweredType(geometry.Sample<float, 4>))
M_SampleInt4   = runtimeIdentity(loweredType(geometry.Sample<int, 4>))

M_SampleFloat4 != M_SampleInt4
```

Schematic IR after specialization, with runtime identity decorations:

```text
%Point = struct_type {
    %x : float
    %y : float
}
decorate %Point RuntimeTypeIdentity M_Point
decorate %Point ReflectionName "Point"
decorate %Point TypeModuleName "geometry"

%Sample_float_4 = struct_type {
    %values : array<float, 4>
}
decorate %Sample_float_4 RuntimeTypeIdentity M_SampleFloat4
decorate %Sample_float_4 ReflectionName "Sample<float, 4>"

# Any code-linkage decoration is distinct metadata; it is not used as a fallback key.
```

`RuntimeTypeIdentity` is implemented as declaration identity plus substituted lowered operands.
The schematic strings above stand for the emitter's complete runtime encoding; they do not
imply preservation of source distinctions that lowering deliberately erases.

### HGBC metadata

The runtime `TypeMangledName` decoration is defined through the canonical HGBC opcode schema,
generated declarations, decoder/disassembler, and emitter. `SlangTypeName` remains readable naming
and reflection metadata. Optional `NameHint` metadata does not establish identity.

Emit stable identity for named types reachable from host signatures, including named nested
components that need not themselves be public exports. A compiler-generated nominal type without
a stable identity remains local to its loaded program. That locality propagates through
structural types containing it; such a type cannot accidentally become shareable through an array
or other wrapper.

Schematic HGBC for two independently compiled programs:

```text
# Program A
%f32A = OpFloatType
%nameA = OpStringLit "Point"
%moduleA = OpStringLit "geometry"
%mangledA = OpStringLit M_Point
%xA = OpStringLit "x"
%yA = OpStringLit "y"
%17 = OpStructType %xA %f32A %yA %f32A
OpDecorate SlangTypeName %17 %nameA
OpDecorate ModuleName %17 %moduleA
OpDecorate TypeMangledName %17 %mangledA
# makePoint has result type %17.

# Program B
%f32B = OpFloatType
%nameB = OpStringLit "Point"
%moduleB = OpStringLit "geometry"
%mangledB = OpStringLit M_Point
%xB = OpStringLit "x"
%yB = OpStringLit "y"
%42 = OpStructType %xB %f32B %yB %f32B
OpDecorate SlangTypeName %42 %nameB
OpDecorate ModuleName %42 %moduleB
OpDecorate TypeMangledName %42 %mangledB
# lengthSquared has parameter type %42 and result type %f32B.
```

String literal lengths, literal result types, section framing, and complete function records are
omitted. The `%17` and `%42` IDs are illustrative. The actual bytecode does not contain VM
addresses; it remains independently loadable into different VMs.

HGBC currently requires a matching compiler/runtime rather than providing a cross-version binary
compatibility promise. Update its format/schema handling as required by the existing convention.
Bytecode without the new identity can retain local type bindings, but cannot claim cross-program
nominal compatibility by falling back to a display name.

### Interning and definition consistency

Nominal types use `mangled name -> canonical record`. Structural types use keys made from their
kind, canonical operand addresses, and semantic parameters.

| Type | Load-time key |
| --- | --- |
| Scalar | Scalar kind. |
| Fixed array | Element type address and element count. |
| Vector or matrix | Element type address, dimensions, and representation-relevant qualifiers. |
| Runtime array | Canonical element type address and any semantic qualifiers. |
| Pointer or address wrapper | Wrapper kind, pointee type address, and qualifiers; still excluded from the host-value allowlist. |
| Named struct or class | Fully specialized lowered runtime identity. |
| Private generated nominal type | Loaded-program identity plus local type ID. |

Do not merge differently named nominal types because their sizes or field lists match.

A mangled name identifies a symbol; it does not prove that two definitions agree. Enforce one
consistent definition per symbol. On a repeated definition, compare its normalized type
description against the existing definition: ordered fields, canonical field types, semantic
field names, and relevant qualifiers. Reuse the physical layout once the definition agrees.
This validation does not recalculate byte offsets or build a second physical layout.

Module loading must also keep symbol origins and source revisions consistent. Retain a fixed
compiler snapshot and diagnose conflicting bindings of the same module/linkage symbols. Module
provenance or definition digests can support that check, but do not become another runtime type
ID. Do not hash the entire consuming program into a type key: two programs importing the same
type must share it. A common compiler session can simplify module reuse, but is not required for
address equality once HGBC carries authoritative identities. The current compiler's separate
sessions must not silently permit incompatible symbol definitions in one VM.

Host-visible representations also need a compiler invariant. Their fields and canonical VM
representation must be consistent across compilation units, even if consumers use different
field subsets. Preserve that boundary through aggregate lowering and optimization. A surviving
semantic name cannot justify reusing a layout for a differently pruned physical struct. Keep
context-dependent shader/device layouts separate from canonical host layouts.

## Shared staging architecture

```text
VM lifetime state
  shared StagingContext
    staged instruction, type, and literal storage
    nominal interning cache: mangled name -> type address
    structural interning cache: kind and canonical operands -> type address
    canonical layout metadata
    loaded programs and their debug/source descriptions
  native owned values
  interpreter and device state

Program A                            Program B
  HGBC %17 -> type address X           HGBC %42 -> type address X
  bound makePoint                     bound lengthSquared
  fixed program const heap            fixed program const heap
                  \                   /
                   shared Point record X
```

### Split canonical layouts from local staging information

Today, `Stager::TypeLayout` contains local field-name/type IDs and local element/pointee IDs.
Moving that map unchanged into a VM would be incorrect. Separate it into:

1. Canonical layout metadata using canonical child addresses and shared field names.
2. Per-program bindings translating HGBC type IDs and field selectors to canonical metadata.

Retain allocation size/alignment, class managed-offset placement, runtime-array element layout,
and element ownership metadata as well as ordinary inline size/alignment. Some of this exists
only in the current transient `TypeLayout`; the current `TypeInfo` record alone is insufficient
to stage all allocation operations in a second program without repeating layout work.

Schematic native structures:

```cpp
struct CanonicalFieldInfo
{
    Address typeAddress;
    Address nameAddress;       // Shared immutable string storage.
    uint32_t nameByteCount;
    uint32_t byteOffset;
};

struct CanonicalTypeInfo
{
    TypeKind kind;
    uint32_t byteSize;
    uint32_t byteAlignment;
    Address displayNameAddress;
    Address elementTypeAddress;
    uint32_t elementCount;
    uint32_t elementStride;
    // Immutable tails/records contain fields, owning leaf offsets,
    // and class/array allocation metadata as applicable.
};

struct ProgramTypeBindings
{
    Dictionary<HGBCId, Address> typeAddresses;
    // Local field selectors map to canonical fields during staging.
};
```

These extend/refactor existing staged records rather than duplicating them. The same canonical
metadata drives execution, ownership, host validation, and host-to-device conversion.

### Address stability and incremental publication

Keep the existing encoded `Address` model. `InstructionAbsolute` addresses become offsets into
VM-owned staged storage; they retain their word units. Frame addresses remain invocation-local
byte offsets. Canonical type addresses are immutable and are not recycled while the VM is live.

The initial implementation can use a growable contiguous staged buffer with persistent logical
offsets. Loading and execution are serialized, so buffer growth happens outside execution.
Every reference retained across a load must be an offset or separately owned stable storage,
never a native pointer into that buffer. Audit lazy resources and asynchronous work as well as
the interpreter. The existing debug `getRecord()` pointer contract must become a scoped borrow
or copy/handle API if it cannot survive buffer growth. If preserving persistent pointers is
required, choose nonmoving storage before enabling incremental loads; silently invalidating
them is not an acceptable implementation.

Convert the one-shot builder finalization into a per-load transaction with a committed extent:

1. Decode/validate the program and begin an unpublished staging transaction.
2. Resolve identities against committed caches plus a transaction-local reservation map.
3. Reserve nominal records before resolving recursive references. Recursive classes may refer
   through fixed-size handles; impossible infinitely sized inline layouts remain errors.
4. Reuse existing layouts or populate new records, literals, and function bodies.
5. Finish fixups, definition checks, cumulative address-limit checks, and fixed const-heap setup.
6. Publish the new program, completed canonical records, bound exports, and debug metadata together.

On failure, destroy only newly constructed const objects, remove pending cache entries, and
discard the uncommitted suffix. Previously loaded programs and values remain usable. Serialize
these transactions so no caller observes a partially populated record. Shared storage is subject
to the cumulative instruction-address limit, not a separate limit for each program.

### Program const heaps and device layouts

The shared space initially covers instructions, type records, and literals. Keep each
`ProgramInstance` const heap fixed and nonmoving. It contains placement-constructed C++ objects
with strings, lists, COM references, and pointers to other lazy objects; appending and reallocating
it as raw bytes would invalidate those objects.

`ConstHeap` addresses remain explicitly scoped to a program instance. The live VM owns actual
instances. A bound function retains the lifetime control block and a non-owning instance pointer;
close invalidates that pointer. An active invocation temporarily retains the instance for lazy
operations and debugging. Python module facades hold decoded `Program` objects, which their VM
clears during close. No additional public `LoadedProgram` class is introduced. Closing the VM
clears execution-instance ownership even if wrappers survive, so their later destruction cannot
release const objects after the services those objects borrow have gone away.
Do not allow an unscoped const-heap address to escape as a host value. Canonical type records
contain no such addresses. Sharing types does not require deduplicating function bodies or making
cross-program VM-to-VM calls; the initial cross-program path invokes B through the common host API.

Retain loaded program instances until close initially. Device-memory layouts, shader parameter
layouts, pipelines, and converters retain their domain/usage/context in their cache keys.
Use canonical host type addresses for their logical-type checks while preserving explicitly
scoped source IDs where compiler reflection still needs them.

### VM representation of the Point example

The addresses below are illustrative instruction-word offsets; field offsets and sizes are bytes.

```text
VM staged storage
  T_float = InstructionAbsolute(0x0100)
    kind: Scalar(Float32), byteSize: 4, byteAlignment: 4

  T_Point = InstructionAbsolute(0x0200)
    kind: Struct, byteSize: 8, byteAlignment: 4
    displayName: shared string "geometry.Point"
    fields:
      x: type T_float, byteOffset 0
      y: type T_float, byteOffset 4
    managedLeafOffsets: []

Staging bindings
  A: %f32A -> T_float; %17 -> T_Point
  B: %f32B -> T_float; %42 -> T_Point

Bound makePoint
  owner: VM; programContext: A
  parameters: [T_float, T_float]
  result: T_Point
  entry: shared instruction address for A.makePoint

Bound lengthSquared
  owner: VM; programContext: B
  parameters: [T_Point]
  result: T_float
  entry: shared instruction address for B.lengthSquared

External p
  owner: VM lifetime state
  type: T_Point
  payload: [float32(3.0), float32(4.0)]
```

The binding map is consulted while staging/binding functions. A staged field access to `p.y`
embeds byte offset 4. A copy embeds size 8. Operations requiring type metadata contain `T_Point`
directly. They do not perform a local-ID lookup at execution time.

## Common invocation API

Bound native functions cache canonical type addresses. Decoded `FunctionSignature` IDs remain
program-local for bytecode inspection. `BoundFunction::getParameterTypes()` and `getResultType()`
expose cached `HostValueType` metadata without exposing mutable canonical addresses.

The implemented native surface is below. Scalars and aggregates both use native owned result
state initially; a default/void `Value` owns no payload. `Program::load` still loads validated
HGBC, and `Interpreter::prepareProgram` stages it without execution. Binding also stages a program
on first use.

```cpp
SlangResult Interpreter::bindFunction(
    Program* program,
    HGBCId functionId,
    RefPtr<BoundFunction>& outFunction,
    String& outError);

SlangResult Interpreter::invoke(
    BoundFunction* function,
    ConstArrayView<Argument> arguments,
    Value& outResult,
    String& outError);

SlangResult Interpreter::close(String& outError);
bool Interpreter::isClosed() const;
```

```cpp
RefPtr<BoundFunction> makePoint;
RefPtr<BoundFunction> lengthSquared;
String error;
SLANG_RETURN_ON_FAIL(vm.bindFunction(
    producer, producer->findFunctionExport("makePoint")->id, makePoint, error));
SLANG_RETURN_ON_FAIL(vm.bindFunction(
    consumer, consumer->findFunctionExport("lengthSquared")->id, lengthSquared, error));

Argument makeArgs[] = {Argument::scalar(3.0f), Argument::scalar(4.0f)};
Value point;
SLANG_RETURN_ON_FAIL(vm.invoke(makePoint, makeArrayView(makeArgs), point, error));

Argument lengthArgs[] = {Argument::fromValue(point)};
Value result;
SLANG_RETURN_ON_FAIL(vm.invoke(lengthSquared, makeArrayView(lengthArgs), result, error));
float length = 0;
SLANG_RETURN_ON_FAIL(result.getScalarData(
    ScalarType::Float32, &length, sizeof(length), error));
```

Native scalar arguments carry explicit scalar kinds; the native API does not implicitly reinterpret
a C++ integer as a float. Python performs its existing number conversions against the bound
parameter types before calling this API. Opaque aggregate arguments come from valid native
`Value` handles, not arbitrary Python buffers or forged managed-reference bytes.

The existing scalar `ValueView` overload adapts to the bound invocation path. It,
`slangi`, the debugger, Python, and the new native API share the execution and transfer path.
`slangi` discards results through normal result ownership, replacing its scalar scratch-buffer
assumption. This plan does not add aggregate command-line syntax.

### Cross-program call sequence

For `consumer.lengthSquared(p)`:

1. Check that the function and value belong to the live invoking VM.
2. Check argument count and supported parameter modes/types before execution begins.
3. Compare `p.typeAddress` with the cached parameter address `T_Point`.
4. Copy p's eight payload bytes using the existing parameter-transfer machinery.
5. Execute the consumer's staged function with its own program context.
6. Return the scalar result through existing Python decoding.

There is no pairwise source/destination type map, name lookup, layout comparison, serialization,
or recursive type/field walk during this transfer. Pure-value payloads contain no program-local
type IDs to rewrite. The source `Value` remains unchanged and can be reused in either program.

Managed values follow the same sequence, with ordinary argument copying retaining each owning
leaf at its precomputed offset. Heap objects already carry canonical type addresses, so they need
no relabeling or conversion. Copies preserve object identity: mutating a referenced class or array
through one program is visible through aliases held by another program or external value.

### Aggregate returns and failure handling

Root invocation now sizes the result prefix from the actual result and aligns the following root
frame. Checked arithmetic preserves frame-address limits, and results larger than 64 bytes use
ordinary internal return lowering. Empty structs retain the VM's one-byte representation; zero
storage denotes void.

Invocation zeroes the result prefix and gives it a scope guard using the result type's managed-leaf
offsets. Ordinary return lowering stores the result and retains its owning leaves. The guard
releases any owned result on failure or cancellation, including a late failure from
`completeInvocation()`, while the VM and its dependencies are still alive.

A temporary native result is allocated before execution. Only after execution and
`completeInvocation()` succeed are the result bytes moved into its `OwnedPayload`; clearing the
root prefix transfers ownership without another retain. Invocation then replaces `outResult`.
Failure leaves the caller's output unchanged. An input `Value` may also be the output variable;
argument handles retain its payload through execution. CLI result disposal uses the same owner.

A lowered HGBC instruction can contain several execution records, including releases, a store,
and retains. Records after its first carry `kInstructionContinuationBit`. Debugger stops, stepping,
and cancellation observe complete source-instruction boundaries, so they cannot interrupt an
ownership transition halfway through. This applies to ordinary copies and aggregate construction
as well as root returns; it does not require a separate external-return opcode or partial-result
ownership state.

## Owned values and VM lifetime

Python, JavaScript, and C++ use one native ownership implementation. `OwnedPayload` shares the
existing `BoxObject` operations: aligned allocation, byte copy, retain of each managed leaf, and release
of each owning leaf. It stores the canonical layout's managed-leaf offsets alongside the owned
bytes. Pure values use an empty leaf list. External values are not language-level boxes.

Conceptual aggregate value state:

```cpp
struct NativeValueState : RefObject
{
    RefPtr<VMControlBlock> lifetime; // Survives as closed state after VM memory is released.
    Address typeAddress;        // Dereference only while live.
    HostValueType type;         // Includes a cached readable name.
    OwnedPayload storage;      // Cleared by destruction or VM close, exactly once.
    // Intrusive membership in the VM's live-value registry.
};
```

The VM tracks live native value states through non-owning intrusive links so close can clear all
payloads. Each state retains only the small lifetime control block, avoiding a strong VM/value
ownership cycle. Copying a Python reference to a wrapper or copying a native
handle shares its state. Passing a value by value into Slang makes the usual parameter copy.
Copies of values containing managed handles retain each owning leaf and preserve the
referenced objects' identity. Two fields referencing one object each own a reference; do not
deduplicate release operations by object address.

Close is idempotent and serialized with execution:

1. Mark the lifetime state closed, making calls and payload access fail.
2. Clear external value payloads while type records, program instances, and device state exist.
3. Settle/abandon backend work according to the existing shutdown contract and destroy dependent
   lazy/resource objects while their required services remain alive.
4. Release program instances, staged storage, and interpreter/compiler resources in dependency order.
5. Leave only closed control blocks and cached wrapper metadata for later wrapper destruction.

The guarantee is that close does not wait for Python or JavaScript garbage collection. It may still
synchronize with backend work as required for safe shutdown. Existing reference-counted object
cycles do not gain a cycle collector through this feature.

Dropping a Python module facade no longer releases its program cache entry. Loaded records and
instances remain owned until close, independently of module or bound-function wrappers.
`releaseProgram` is a compatibility hook with no individual reclamation in this shared model;
independent unloading is a later feature.

## Managed values

### Canonical identity in allocations and checks

Class, runtime-array, and box producers/consumers use canonical type addresses. Classes and runtime
arrays can now cross the external value boundary when their transitive payload types are supported.
Boxes retain their internal canonical representation but remain unsupported externally. The old
and new representations are:

```text
Before shared staging
  AllocateObject.classTypeId = A.%17
  ClassObject.typeId         = A.%17
  B field access expects      B.%42

Canonical representation
  AllocateObject.typeAddress       = T_Counter
  ClassObject.typeAddress          = T_Counter
  FieldAccess.expectedTypeAddress  = T_Counter

  AllocateArray.typeAddress        = T_RuntimeArray_Point
  RuntimeArrayObject.typeAddress   = T_RuntimeArray_Point
  ArrayOperation.expectedType     = T_RuntimeArray_Point

  Box.payloadTypeAddress          = T_Point
  BoxObject.payloadTypeAddress    = T_Point
  Unbox.expectedPayloadType       = T_Point
```

The runtime equality check becomes `actualTypeAddress == expectedTypeAddress`. A box compares
the concrete payload type, not the generic box-handle type. Array length is a property of an
allocation; it does not distinguish the type `RuntimeArray<Point>`.

Audit all staged fields carrying type IDs, including array copy/append, field access, byte-stream
endpoints, box extraction, tagged-union case descriptors, and layout-converter logical-type checks.
Keep source/debug IDs only where explicitly scoped. Do not relabel objects when passing them;
their original program can still hold aliases.

### Payloads and dependencies

For `struct Holder { Counter counter; float weight; }`, inline copies retain the `Counter` handle
using canonical managed-leaf offsets. A host call to another program copies the holder and
preserves the same counter object. A new holder returned by Slang owns its own reference.

Shared literal storage also makes instruction-addressed string literals meaningful across
programs: the existing `StringValue` literal address resolves against the VM's arena. Dynamically
created strings retain their owning `StringObject`. Both can now be carried by opaque external
values and passed to other programs in the same VM. This does not add automatic conversion to or
from Python or JavaScript strings.

After staging every type layout, initialize eligibility from the supported type kinds and
repeatedly reject structs/classes with an unsupported field and arrays/vectors/matrices with an
unsupported element type. The resulting greatest fixed point admits recursive owning graphs and
rejects transitive borrowed references without depending on field order. Cache the result in
canonical layout metadata before function binding; repeated calls only check that cached policy.
Boxes and tagged unions need their own transitive ownership contract before they can be enabled.

Structured buffers now cross the external value boundary when their elements are supported plain
data. Their wrappers own the RHI buffer and pool, and submitted work retains buffer leases until
GPU completion. The allocating program's element layout remains alive because program instances
are retained until close. Close releases external payloads and drains submitted work before
destroying programs. Debugger inspection resolves the buffer's canonical element address to a type
ID in the inspecting program. The browser and Python GPU value examples exercise cross-program
buffer ownership; the native regression also checks debugger element-type metadata.

Other resource kinds still require a dependency audit. For example, deferred readbacks borrow a
converter and device state. Enable each category only after its dependencies and failure cleanup
are covered. Reference counting and a canonical type address alone do not prove a resource can escape.

## Debugging and diagnostics

Preserve per-program source identity and debug maps even though execution offsets become global.
Breakpoints, function/source lookup, and frame context must agree on the new offset convention.
Source breakpoints resolve to the first record of an instruction, including an internal ownership
update when it begins that instruction. Continuation records are not independent stop locations;
record stepping and cancellation also finish the current instruction's ownership updates first.
Use a canonical diagnostic type reference or an explicit reverse binding when a debugger needs
to describe a heap object's type. Do not interpret a canonical address as a local HGBC ID, and do
not resolve an object's local type through whichever program happens to be active.

The invocation boundary distinguishes closed values/VMs, wrong VM ownership, type mismatch, and
unsupported host types; staging reports conflicting definitions. Current type mismatch diagnostics
identify the failed category, while cached readable names remain available through value metadata.
Including both expected and actual declaration/module names is a diagnostic improvement for later.
No GPU device is needed to load, stage, or invoke the scalar, pure-value, or supported managed-object
examples.

## Implementation sequence

Each step includes focused regression coverage. Steps 1 and 2 establish the identity/representation
contract before shared layout reuse; steps 3 through 7 build the native foundation; step 8 exposes
the complete pure-value workflow. Step 9 extends the same opaque-value model to managed values.
Step 10 remains future work for categories with additional ownership or service dependencies.

| Step | Work and affected area | Completion evidence |
| --- | --- | --- |
| 1. Preserve lowered specialized identity | Carry nominal declaration identity through specialization and encode canonical lowered arguments; separate runtime identity from code linkage, readable hints, and obfuscation. | Shared runtime types agree across programs; nested nominal distinctions survive; compiler-erased enum distinctions are accepted. |
| 2. Define the host representation and HGBC metadata | Preserve boundary-visible aggregate definitions through lowering; add `TypeMangledName` emission/decoding/schema support; define duplicate-definition validation and local-only fallback for unnamed types. | HGBC exposes concrete identities without debug info; separate consumers using different fields agree; inconsistent definitions fail loading. |
| 3. Add the VM staging context | Move instruction/type/literal storage ownership, retain program contexts, split canonical metadata from local bindings, and add transactional builder publication. Audit persistent native pointers before permitting growth. | Load A, invoke it, load B, and invoke A again; injected load failure leaves A and its values intact. |
| 4. Intern and reuse canonical layouts | Add nominal/structural caches, canonical child/name references, recursive reservations, and complete allocation metadata. Preserve debug/source bindings. | A and B share one address and one layout calculation per shared type; private/different nominal types remain distinct. |
| 5. Migrate runtime type consumers | Replace local type IDs in relevant staged operations, object headers, box checks, converter checks, and debugger object descriptions. Update allocation and consumption together. | Existing internal class/array/box tests pass; deliberate local-ID collisions cannot fool canonical checks. Establishes the identity groundwork for managed host inputs. |
| 6. Add native owned values | Implement live-value tracking, aligned payload ownership, cached names, close invalidation, and shared copy/retain/release helpers. Adjust program release semantics. | Values survive facade deletion; close frees payloads before dependencies; later finalizers are harmless. |
| 7. Extend the common invocation path | Bind canonical signatures; normalize scalar/owned arguments; allocate arbitrary-size result prefixes; publish temporary results only on success; adapt old scalar and CLI paths. | Native same/cross-program struct calls work, large/aligned results succeed, discarded results and failures clean up. |
| 8. Expose Python values and examples | Bind `Value`, `type_name`, and `repr`; preserve scalar conversion behavior; add producer/consumer examples and integration tests. | The Point example works in a headless VM, including loading the consumer after creating p. |
| 9. Enable managed values | Add root-result scope cleanup and ownership transfer; observe complete source-instruction debugger/cancellation boundaries; enable strings, classes, runtime arrays, and aggregates containing them after transitive fixed-point validation. | Same-VM cross-program aliasing and balanced reference counts; safe failure/cancellation, disposal, and close; recursive types containing borrowed pointers rejected before execution. |
| 10. Audit remaining managed categories | Establish transitive ownership rules for boxes/interfaces and tagged unions, and audit GPU resource dependencies before enabling each category. | Safe cross-program use and teardown under each category's supported backend behavior. |

A single compiler session per VM is a useful follow-on simplification if module-search and naming
semantics remain correct. It is not a prerequisite to accept equal authoritative mangled symbols
from independently compiled HGBC, nor a substitute for definition consistency checks.

## Validation plan

| Area | Cases |
| --- | --- |
| Identity | Shared import; unrelated same-named types; aliases; generic type/value arguments; nested generics and packs; composite arguments containing differently qualified same-named types; equivalent witness routes; debug and supported obfuscation settings. |
| Definition consistency | Same mangled symbol with conflicting fields/qualifiers; conflicting module bindings; different consuming field subsets; types with private/generated components. |
| Shared staging | Load orders A/B and B/A; repeated loads; buffer growth after values exist; cumulative address overflow; recursive class references; rollback after a reserved type, function fixup, or const-object initialization fails. |
| Layouts | Nested structs; bool/float padding; fixed arrays; vectors/matrices; empty structs; results larger than 64 bytes; transitive borrowed pointers and unsupported managed categories rejected before execution. |
| Invocation | Same-program and cross-program results; many repeated calls; wrong count/type/VM; scalar regressions; no argument execution on validation failure; result aliasing guarantees for legacy scalar storage. |
| Ownership | Wrapper aliases; facade deletion; modules and bound functions surviving close; prompt native payload destruction; ordinary finalization; repeated close; use after close; failure before/after result construction; native input/output aliasing; result discarded by CLI; managed leaf counts, repeated fields owning the same object, recursive type graphs, and cross-program aliases. |
| Debug and device contexts | Global instruction offsets; source breakpoints and cancellation at complete instruction boundaries, including managed returns; type names for foreign-origin objects; fixed const heaps; distinct shader layouts for one canonical host type; deferred/resource teardown. |
| Performance contract | Instrument layout construction/interner access: a reused canonical type has one layout calculation, and repeated bound calls perform no name lookup or type remapping. |

Put compiler/format tests in `tests/hgbc`, ordinary execution tests in `tests/compute/hgvm`,
and rendering/autodiff cases in `tests/render/hgvm` and `tests/autodiff/hgvm` where relevant.
Keep native invocation tests with the existing HGVM tests in `tools/gfx-unit-test` and Python
coverage in `python/tests/test_slangvm.py`. Use a test-only counter or equivalent focused
instrumentation to verify layout reuse instead of inferring it solely from successful calls.

For this implementation, the user waived broad repository checks. Rebuild affected targets and
run focused compiler, native invocation, shared staging, and Python regressions headlessly with
one test server. Record skipped or unavailable validation explicitly.

## Source map

The following locations are the main integration points. Identity, layout, and ownership live in
the native HGVM/compiler layers; the Python binding contains no parallel type registry or layout
calculator.

| Source | Role |
| --- | --- |
| [slang-mangle.cpp](../../source/slang/slang-mangle.cpp) | Semantic type/declaration mangling and generic substitution encoding. |
| [slang-session.cpp](../../source/slang/slang-session.cpp) | Public nominal RTTI-name API. |
| [slang-lower-to-ir.cpp](../../source/slang/slang-lower-to-ir.cpp) | Declaration identity, linkage, and reflection-name production. |
| [slang-ir-clone.cpp](../../source/slang/slang-ir-clone.cpp) and [slang-ir-specialize.cpp](../../source/slang/slang-ir-specialize.cpp) | Existing specialization-name construction and metadata propagation. |
| [slang-emit-hgbc.cpp](../../source/slang/slang-emit-hgbc.cpp) and [slang-hgbc-opcodes.lua](../../source/slang/slang-hgbc-opcodes.lua) | HGBC type metadata emission and authoritative opcode/decoration schema. |
| [slang-hgvm-compiler.cpp](../../source/slang-hgvm/slang-hgvm-compiler.cpp) | Shared source compilation and current session-cache policy. |
| [slang-hgvm-interpreter.cpp](../../source/slang-hgvm/slang-hgvm-interpreter.cpp) | Staging, layout calculation, program instances, heap objects, root invocation, and cleanup. |
| [slang-hgvm-staged-records.h](../../source/slang-hgvm/slang-hgvm-staged-records.h) and [slang-hgvm-staged-record-builder.h](../../source/slang-hgvm/slang-hgvm-staged-record-builder.h) | Runtime type records, typed operands, and storage/finalization rules. |
| [slang-hgvm-invocation.h](../../source/slang-hgvm/slang-hgvm-invocation.h), [slang-hgvm-runtime.cpp](../../source/slang-hgvm/slang-hgvm-runtime.cpp), and [slang-hgvm.h](../../source/slang-hgvm/slang-hgvm.h) | Common scalar and owned-value invocation contracts and validation. |
| [slang-hgvm-debug-session.cpp](../../source/slang-hgvm/slang-hgvm-debug-session.cpp) | Source-breakpoint mapping to complete instruction boundaries. |
| [slangvm.cpp](../../source/slangvm/slangvm.cpp) and [Python facade](../../python/slangvm/__init__.py) | Thin host conversions, wrappers, and facade lifetime. |

## Milestone acceptance

The pure-value milestone is complete when independently compiled producer and consumer programs
share canonical type/layout records, the Point workflow works through both Python and the common
native API, and invocation performs only liveness/VM/type-address checks plus ordinary transfers.
The same result must survive subsequent loads and facade deletion, while close invalidates it
safely. Identity propagation, layout reuse, failure rollback, and large aggregate returns need
explicit regression coverage before this milestone is considered finished.

Managed-value acceptance extends that coverage to strings, classes, runtime arrays, and nested
aggregates. Copies must preserve aliases and balance every owning leaf, including repeated handles
to the same object. Recursive owning types are accepted, transitive borrowed pointers are rejected,
and cancellation or late invocation failure releases any constructed root result without changing
the caller's output. Python, JavaScript, and C++ wrappers all use the common native owner; disposal
and VM close must leave later finalizers harmless.
