# Structural Ray Tracing API Implementation Plan

Status: implementation architecture for [PROPOSAL.md](PROPOSAL.md). The draft implementation
through the dynamic-schema and open-section work is recorded by checkpoint
`dcec1f948` (`(Checkpoint Commit) Finalize dynamic structural ray tracing`) in PR #12691.
[PROPOSAL.md](PROPOSAL.md) is the sole normative design.

The revised API describes a compile-time **trace program schema**. The host creates the runtime
shader binding table (SBT) from that schema. Shader source no longer declares record slots.

Stable implementation slices use the commit subject `(Checkpoint Commit) <message>`. A checkpoint
means its focused tests pass; it does not mean that the complete feature is finished.

## 1. Fixed Decisions

### 1.1 Standard Module

The API ships as a precompiled experimental standard module:

```slang
import slang.raytracing;
```

The dependency direction is:

```text
user module -> core
user module -> slang.raytracing -> core
```

`core` never imports `slang.raytracing`. The compiler loads the module only after an explicit
import, and the import requires `-experimental-feature`. Build and install it after `core`, following
the `slang.neural` standard-module model. It does not participate in core bootstrap.

### 1.2 Ownership Boundary

`slang.raytracing` owns:

- Context, primitive, stage, hit-group, section-list, schema, descriptor, and trace declarations.
- Closed, open, and empty section-list forms.
- Generic constraints and placeholder stages.
- Property-only stage inputs and shader-side wrappers over existing intrinsics.

The compiler owns only semantics ordinary Slang cannot express:

- Unforgeable identity for the five structural stage interfaces and structural runtime types.
- Retention and synthesis of interface-conforming stage entry points.
- Canonical schema discovery after linking, including open-section entries.
- Payload partitioning, deterministic function indices, and schema reflection.
- Demand-driven native stage signatures and Metal tag inference.
- Metal descriptor resources, candidate dispatchers, and post-trace/callable dispatch.
- Structural-only use restrictions and mixed-API diagnostics.

The host owns each SBT instance:

- Record counts, record positions, and the entry selected by each record.
- Per-record data.
- Acceleration-structure record contributions.
- The `sbtOffset`, `sbtStride`, and `missIndex` convention used by trace calls.

The implementation must not reintroduce a shader-side slot or another declaration of this runtime
mapping.

Version one excludes SER, Metal `intersection_function_buffer`, and Metal `user_data`. Curves and
Metal multilevel acceleration structures remain capability-gated.

### 1.3 Compiler-Bug Policy

Do not add a structural-ray-tracing workaround for an independent compiler defect. Reduce and file
the defect, fix it in a separate PR/branch when it blocks this work, then rebase the implementation
on that fix. Record nonblocking defects in PR #12691 without changing unrelated compiler behavior.

The checkpoint contains the prerequisite associated-type fixes it needs. There is no unresolved
compiler prerequisite blocking the current draft. Any newly exposed independent defect still
follows the policy above.

Use a separate `__constraint` declaration for every associated-type bound or relationship. Reserve
`where` clauses for generic parameters, functions, extensions, and other ordinary generic
constraints.

The removed variadic list constraint is not replaced with a compiler special case. Schema
canonicalization checks every listed and linked entry uniformly.

## 2. Repository Layout

### 2.1 Standard Module

```text
source/standard-modules/raytracing/
├── CMakeLists.txt
├── raytracing.slang          # Module declaration and ordered includes
├── ray-types.slang           # Rays, traversal, acceleration structures, primitives
├── contexts.slang            # ITraceContext and the stage-context hierarchy
├── stage-inputs.slang        # Zero-storage inputs and intrinsic properties
├── stage-contracts.slang     # Stage interfaces and placeholders
├── program-schema.slang      # Hit groups, section lists, and ITraceProgramSchema
├── descriptor.slang          # Opaque TraceProgramDescriptor<Schema>
└── trace.slang               # RayTracer<Schema>, trace, and callShader
```

Each included file uses `implementing raytracing;` and publishes declarations only in
`namespace rt`. Keep compiler operations internal and surface them through typed properties or
methods.

The installed artifact remains `slang/raytracing.slang-module`. Do not add a special session lookup
path or make `core` depend on it.

### 2.2 Compiler Files

Keep the existing feature-owned files:

```text
source/slang/
├── slang-structural-ray-tracing.{h,cpp}                 # Canonical declarations and roles
├── slang-check-structural-ray-tracing.cpp                # Front-end restrictions
├── slang-ir-structural-ray-tracing.{h,cpp}               # Shared IR queries
├── slang-ir-synthesize-structural-ray-tracing.{h,cpp}    # Schema discovery and adapters
├── slang-ir-metal-structural-ray-tracing.{h,cpp}         # Metal descriptor and dispatch
├── slang-ir-optix-ray-tracing-abi.{h,cpp}                # OptiX payload/attribute ABI
└── slang-reflection-structural-ray-tracing.{h,cpp}       # Schema reflection
```

Existing compiler files receive narrow registration, scheduling, IR-definition, binding, emit, and
API hooks only. D3D, Vulkan, and OptiX continue through existing ray-tracing IR and target paths.

The public reflection API uses `TraceProgramSchema`, never `TraceProgramLayout`, because
`slang::ProgramLayout` already names the program reflection object and the schema declares no byte
or record positions.

### 2.3 Tests And Runtime Hosts

Keep focused compiler tests under `tests/ray-tracing-2/` and the existing runtime hosts:

```text
tests/ray-tracing-2/
├── frontend/
├── ir/
├── target/{portable,vulkan,metal}/
├── reflection/
├── compatibility/
├── runtime/{shaders,metal}/
├── integrate/
└── coverage-manifest.md

tools/gfx-unit-test/structural-ray-tracing/       # D3D12/Vulkan through Slang RHI
tools/metal-structural-raytracing-test/           # Native Metal host, local only
```

Extend the Slang RHI shader-table description with arbitrary per-record data pointers and sizes;
the existing eight-byte overwrite is not a portable record-data API. Keep this append-only and
independent of the structural shader API.

The Metal host writes the revised fixed-stride record buffer and binds the reflected per-payload
tables. It remains outside installation, deployment, and the regular test suite.

## 3. Source Contract

### 3.1 Type Hierarchy

```text
ITraceProgramSchema
├── TraceContext : ITraceContext
│   ├── AccelerationStructure
│   └── Motion
├── HitGroups : IHitGroupList
│   ├── HitGroupList<...>
│   ├── OpenHitGroups<Tag, ...>
│   └── NoHitGroups
├── MissShaders : IMissShaderList
│   ├── MissShaderList<...>
│   ├── OpenMissShaders<Tag, ...>
│   └── NoMissShaders
└── CallableShaders : ICallableShaderList
    ├── CallableShaderList<...>
    ├── OpenCallableShaders<Tag, ...>
    └── NoCallableShaders
```

The context hierarchy is:

```text
IStageContext
├── TraceContext : ITraceContext
└── Record

IPayloadContext : IStageContext
└── Payload

IHitContext : IPayloadContext
└── Primitive

ICallableContext : IStageContext
└── CallableData
```

Each stage interface exposes `Context` as an associated type. `IHitGroup` contains one context and
uses separate `__constraint` declarations to require its `_ClosestHit_`, `_AnyHit_`, and
`_Intersection_` associated types to name that exact context. _Miss_ and _Callable_ sections list
stage types directly; only native multi-stage hit records use a group abstraction.

Schemas describe executable entries only; the host owns physical record positions. `ITraceContext`
does not own a payload. Each hit or miss context owns the payload used by its stages.

### 3.2 Trace And Stage Inputs

`RayTracer<Schema>.trace` infers its payload from the argument:

```slang
void trace<Payload>(
    RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure accelerationStructure,
    TraceProgramDescriptor<Schema> descriptor,
    inout Payload payload);
```

Structural synthesis diagnoses a payload that no hit group or miss shader in `Schema` serves. A
separate no-payload overload is available only when the schema has exactly one empty payload type.
The compiler supplies the mandatory native payload operand internally and rejects attempts to name
that type as an explicit `trace` payload, pass its value to `trace`, or access it through
`input.payload`. Constructing the same empty struct outside the structural API remains valid; its
payload role may be declared in another separately compiled module and is not an intrinsic property
of the struct declaration.

All stage inputs remain zero-storage property views. `input.payload` has type
`Context.Payload`; `input.record` has type `Context.Record` and means the concrete runtime record
that selected the stage on every target. On Metal the dispatch resolves the record and passes a
pointer to the generated stage function.

Map properties to existing ray-tracing IR whenever it expresses the semantics. Add IR only for
state or behavior with no existing representation.

### 3.3 Structural Use And Entry Points

The existing structural-use restrictions remain:

- Stage implementations are stateless and cannot be constructed, stored, converted to
  existentials, or called directly.
- Stage inputs cannot be constructed, stored, returned, used as generic arguments, passed by
  writable direction, or cross stage boundaries.
- Schemas, section lists, and hit groups have no runtime representation.
- `TraceProgramDescriptor<Schema>` is an opaque resource used only for binding and dispatch.
- `RayTracer<Schema>` is a local zero-storage facade.

These are hard semantic errors even under `-ignore-capabilities`. `[require(stage)]` separately
checks operations reachable from each stage body.

`-entry ClosestHit -stage closesthit` continues to select a conforming struct before IR generation.
The struct name remains the public entry-point name. A standalone stage compiles without a schema
or descriptor; its associated `Context` supplies the payload, primitive, attributes, and record
types needed for its native signature.

The front end diagnoses legacy/structural mixing within one module. Linked-program synthesis checks
selected cross-module components. Merely importing `slang.raytracing` does not count as structural
use.

## 4. Compiler Representation

### 4.1 Stage Identity

Keep the compiler-owned IR interface hierarchy:

```text
IRInterfaceType
└── IRRaytracingStageInterface
    ├── IRClosestHitStageInterface
    ├── IRAnyHitStageInterface
    ├── IRIntersectionStageInterface
    ├── IRMissStageInterface
    └── IRCallableStageInterface
```

A concrete implementation remains an `IRStructType` with an ordinary witness table. The selected
witness determines the stage role. Its associated `Context`, rather than a generic interface
argument, determines the stage ABI.

The reusable stage identity is:

```text
(stage kind, concrete invoke implementation)
```

Metal adds the concrete payload-specific `ray_data` ABI when selecting or generating an adapter.
Schema type, declaration order, function index, and runtime record index do not specialize the
source stage body.

### 4.2 Canonical Schema Metadata

After linking and specialization, build one canonical schema shared by reflection and code
generation:

```text
TraceProgramSchema
├── trace context
├── payload partitions
│   ├── payload type and target layout
│   ├── hit-group entries in function-index order
│   └── miss-shader entries in function-index order
├── callable-shader entries in function-index order
├── open/closed state for each section
└── target ABI and record-layout facts

HitGroupEntry
├── group, context, primitive, record, and attributes types
├── closest-hit, any-hit, and intersection witnesses
├── listed/linked origin
└── function index within its payload
```

For closed sections, declaration order determines function indices. For open sections, listed
entries come first and linked entries follow by qualified type name. Hit and miss indices restart
for each payload; callable indices are program-wide. Function indices are dense and valid only for
one linked program. Reflection exposes qualified entry names as the stable host key.

The structural trace marker retains both the selected `Schema` and concrete trace `Payload` through
linking and specialization. D3D/Vulkan/OptiX consume or erase it after native adapter/reflection
generation. Metal consumes it when selecting the payload-specific tables and generated state.

## 5. Compilation Flow

Use the existing two guarded feature phases:

```text
semantic checking
    register canonical declarations
    validate stage capabilities and structural uses
    resolve structural struct entry points
    record same-module legacy and structural use

IR generation, linking, and specialization
    preserve structural identities and trace markers
    retain selected closed entries
    retain tagged conformances needed by selected open sections

structural synthesis before DCE
    discover the complete linked schema
    validate and partition its entries by payload
    assign function indices
    collect reachable ABI and Metal-tag requirements
    generate native adapters and Metal helpers
    publish canonical schema metadata for reflection

dead-code elimination
    generated functions and reflected table metadata retain selected stages

target structural lowering
    lower the descriptor and structural trace/callable markers

existing target legalization and emission
```

Every phase returns immediately when no selected structural entry, descriptor, trace, or schema is
present.

### 5.1 Schema Discovery And Validation

For each selected `ITraceProgramSchema`:

1. Expand the closed or known entry pack in each section.
2. For an open section, enumerate retained linked witness tables conforming to its tag.
3. Union listed and linked entries by concrete type, then order them deterministically.
4. Resolve each entry's context, record, payload/callable data, primitive, attributes, and stage
   witnesses.
5. Validate the entry trace context against `Schema.TraceContext` and all three hit-stage contexts
   against `IHitGroup.Context`.
6. Diagnose duplicate entries, invalid open tags, non-plain-data payloads/records, callable-data
   disagreement, and unsupported target combinations.
7. Derive `payloads(Schema)`, partition hit and miss entries, and assign function indices.
8. Diagnose trace payloads the completed linked schema does not serve.
9. Produce one canonical metadata object for reflection and target lowering.

Open-section witness retention is a canonical producer responsibility. Do not scan arbitrary
post-DCE IR or recover entries by source names.

### 5.2 Liveness And Requirement Collection

Install temporary roots before simplification can remove selected `invoke` methods or open-section
conformances. Remove them after generated adapters and table metadata become permanent roots.

Walk each specialized stage call graph and collect only stage-input properties, target ABI state,
Metal tags, and structural transformations that remain after specialization. Requirement analysis
is per concrete stage and per payload partition where Metal table signatures or candidate state
differ.

Placeholder behavior is explicit:

- `NoAnyHit` and `NoIntersection` generate no stage function.
- Every hit group still has a function index.
- If a payload partition contains any `NoClosestHit` group, Metal reflects one compatible no-op
  closest-hit function for the host to install at every placeholder entry, including when all hit
  groups in that partition use `NoClosestHit`.

## 6. Target Implementation

### 6.1 D3D, Vulkan, And OptiX

Reuse existing payload, hit-attribute, `ReportHit`, `TraceRay`, `CallShader`, legalization, and emit
paths. Generate one native stage adapter per concrete stage specialization and one native hit-group
definition per reflected `IHitGroup`.

The host writes an arbitrary number of records. Each hit record carries the native group identifier
and that record's data; miss and callable records use their stage identifiers and record data.
Reflection supplies every entry name, context/record layout, payload size, and native hit-attribute
size. The shader-visible descriptor is erased on these targets.

### 6.2 Metal Descriptor And Runtime SBT

For each payload partition, synthesize:

- One IFT resource.
- One `_Miss_` visible-function table.
- One `_ClosestHit_` visible-function table.

Add one program-wide `_Callable_` visible-function table and one shared record buffer. The generated
descriptor therefore has `3 * payloadCount + 2` resource fields. Reflection reports each field's
resource kind, payload partition, and Metal argument-buffer `[[id]]`; enumeration order is not a
host binding contract.

The record buffer is:

```text
16-byte header
├── instanceTrieOffset
├── hitOffset
├── missOffset
└── callableOffset

instance-path trie
├── non-leaf u32: word offset from the trie root to the next node
└── leaf u32: logical hit-record contribution

fixed-stride record
├── +0  u32 functionIndex
├── +4  padding through byte 15
└── +16 Context.Record bytes
```

Hit, miss, and callable strides are compile-time constants derived from the largest record in the
corresponding section and rounded to 16 bytes. Section offsets, counts, function indices, and record
values are host-written runtime data. `0xFFFFFFFF` denotes an empty record.

`records[0]` is the byte offset to the trie root. For `max_levels<N>`, one shared generated helper
walks all `instance_id` values from outermost to innermost; each intermediate value is relative to
that same root, and the leaf is the record contribution. Single-level instancing uses the root as a
flat table indexed by scalar `instance_id`. Primitive-AS traversal performs no instance lookup.
Candidate dispatch and committed _ClosestHit_ dispatch call the same helper, so they cannot select
different records for one hit.

Post-trace and callable dispatch read the resolved record's function index, then call the
corresponding VFT entry while passing `record + 16`. Generated stage inputs read `input.record`
through that pointer.

### 6.3 Metal Candidate Dispatch

For each payload partition that has candidate logic, generate at most one IFT dispatcher per
primitive kind, at fixed indices:

```text
0 = triangle
1 = bounding box
2 = curve
```

The dispatcher obtains the instance contribution from the shared lookup above, combines it with
`geometryIndex`, `sbtStride`, and `sbtOffset`, reads the record's function index, and switches over
the hit groups of that payload. Existing per-group `_AnyHit_`/`_Intersection_` composition becomes
an ordinary dispatcher arm. A trace whose payload partition has no candidate logic passes no IFT,
even when another payload in the same schema needs one. Keep the existing Metal `reportHit`
accumulator semantics.

The host assigns geometry IFT offsets only by primitive kind and instance offsets to zero. Material,
ray-type, payload, and group selection come from the SBT record, not the acceleration structure.

Committed custom attributes in generated `ray_data` are keyed by attributes type, allowing one
stage adapter to be shared by groups with the same concrete stage. Continue to reject unsupported
global-parameter use in Metal candidate logic.

## 7. Reflection And Host Construction

Use `TraceProgramSchema` consistently in the reflection surface and expose no physical-record
position accessor. Reflect:

- Schema and trace-context types.
- Payload partitions and payload layouts.
- Hit groups and miss shaders in per-payload function-index order.
- _Callable_ shaders in program-wide function-index order.
- Listed versus linked origin and open-section state.
- Context, record, attributes, and exact emitted stage symbols.
- Native payload/attribute sizes, per-entry record layouts, and Metal record-buffer strides.
- Metal descriptor fields, argument-buffer IDs, no-op closest-hit functions, candidate
  dispatchers, and finalized per-payload IFT signatures in target metadata.

Hosts look up entries by qualified name after every link and write the reflected function index or
native identifier into each runtime record. Numeric function indices are not persistent identifiers.

## 8. Implementation Milestones

Phases 0A through 3 are present in checkpoint `dcec1f948`. Phase 4 remains the validation and
performance-completion list; its Windows D3D12 runtime rerun and cost measurements must not be
treated as complete until they are recorded separately.

### Phase 0A: Schema Terminology

- Rebase PR #12691 on the required independent compiler fixes already on `master`.
- Mechanically rename public standard-module declarations and files, feature-owned compiler
  metadata exposed by the unreleased API, diagnostics, and reflection to schema terminology.
- Retain internal IR operand/accessor and lowering vocabulary until the semantic cutover; renaming
  that representation independently would create churn without changing behavior.
- Update focused tests for the rename without changing payload ownership, record placement, or
  group structure.

Exit: the existing implementation behaves as before, all focused tests pass, and no old layout
name remains in the public unreleased structural surface.

### Phase 0B: Associated Stage Contexts

- Change each stage interface from a generic context argument to an associated `Context`.
- Express every associated-type bound or relationship with a separate `__constraint`, including
  the three hit-stage context equalities and the custom-primitive requirement on
  `IIntersectionShader`.
- Update placeholders, witnesses, standalone entry lookup, adapters, and tests to read the
  associated context.
- Keep the existing payload location, record-position declarations, and miss/callable wrappers for
  this phase; changing them independently would create a transient mixed contract.

Exit: the pre-cutover schema shape works with associated-context stages, standalone stage
compilation still works, and the focused tests pass.

### Phase 1: Atomic Closed-Schema Cutover And Portable Targets

Land the following source, compiler, reflection, and test changes as one checkpoint:

- Introduce the `IStageContext` hierarchy and move payload ownership from `ITraceContext` to hit
  and miss contexts.
- Remove record-position types and accessors, remove miss/callable wrapper groups, and make section
  lists use hit groups, miss shaders, and callable shaders directly.
- Canonicalize closed section lists, validate contexts, and derive payload partitions.
- Assign and reflect dense function indices.
- Make trace payload-generic and implement the empty-payload contract.
- Preserve one stage specialization across schemas and repeated records.
- Extend Slang RHI record data and update D3D12/Vulkan runtime construction.

Do not expose an intermediate public contract containing only part of this cutover.

Exit: one schema supports multiple payloads and arbitrary host record counts on D3D12, Vulkan, and
OptiX; no record-position API or wrapper group remains; all portable integration/runtime tests
pass.

### Phase 2: Metal Runtime Records

- Synthesize per-payload descriptor fields and table signatures.
- Replace slot-indexed record lookup with fixed-stride runtime records carrying function indices.
- Pass resolved record pointers into generated stages.
- Generate per-payload, per-primitive candidate dispatchers and function-index switch arms.
- Update Metal reflection and the native Metal runtime host.

Exit: generated Metal compiles natively; multiple payloads, repeated groups with different record
values, candidate logic, callables, and dynamic record counts pass locally on macOS.

### Phase 3: Open Sections

- Retain relevant tagged conformances through linking.
- Discover and validate tagged hit groups or shaders from linked modules.
- Deduplicate and deterministically order listed and linked entries.
- Include linked entries in payload derivation, tag inference, adapters, tables, and reflection.

Exit: separately compiled Slang modules can contribute entries through each open-section form, and
closed programs pay no discovery or retention cost.

### Phase 4: Hardening

- Complete serialization, diagnostics, reflection, target, and compatibility coverage.
- Run the complete non-SER ray-tracing integration inventory.
- Run the non-ray-tracing regression suite unchanged.
- Measure compiler startup without the import and Metal dispatch cost with the Cornell harness.

Exit: the supported platform matrix passes, legacy behavior is unchanged, and the module remains
unloaded when it is not imported.

## 9. Essential Tests

### 9.1 Front End And IR

- Associated-context stage conformances, all context hierarchy relationships, and all placeholders.
- Closed, open, and empty section forms; sealed list markers; valid and invalid open tags.
- Entry trace-context and hit-stage-context mismatches.
- Duplicate entries and deterministic function-index assignment.
- Payload served/not served, multiple payloads, one empty payload, ambiguous empty payloads, and
  illegal empty-payload values.
- Plain-data payload and record restrictions.
- Struct entry lookup and exact emitted names.
- Structural-use, stage-input, capability, and mixed-API diagnostics.
- Schema and concrete payload markers through serialization, linking, and specialization.
- Open conformance retention, link discovery, deduplication, and ordering.
- One concrete stage referenced by multiple groups/schemas produces one source-stage
  specialization; only ABI-distinct target wrappers may differ.

### 9.2 Portable Targets And Runtime

- One schema with radiance and shadow payloads sharing one native SBT.
- Repeating one entry across many records with different `Record` values.
- Runtime `sbtOffset`, `sbtStride`, `missIndex`, instance, and geometry contributions.
- `_ClosestHit_`, `_AnyHit_`, `_Intersection_`, `_Miss_`, `_Callable_`, recursive trace, custom
  attributes, multiple `reportHit` calls, motion, and all supported primitives.
- Descriptor erasure on D3D/Vulkan/OptiX while schema reflection remains.
- Full in-repository non-SER ray-tracing scenario inventory on D3D12 and Vulkan.

### 9.3 Metal

- `3 * payloadCount + 2` descriptor resources whose reflected Metal argument-buffer `[[id]]`
  matches the generated physical layout independently of enumeration order.
- Per-payload VFT/IFT signatures and static payload-table selection at each trace.
- Fixed primitive-kind dispatcher indices and record-driven candidate switch arms.
- Fixed-stride hit, miss, and callable records, empty records, heterogeneous record sizes, and
  per-record `input.record` semantics.
- Real and placeholder `_ClosestHit_` entries.
- Candidate opacity/ray-flag behavior, `ignoreHit`, accept-and-end, `reportHit`, and committed custom
  attributes.
- Runtime record replacement without rebuilding function tables.
- Generated code accepted by the native Metal compiler and runtime results matching portable
  D3D12/Vulkan cases.
- Metal-only curve and multilevel-acceleration cases, including sibling outer instances whose leaf
  `instance_id` values are equal but whose record contributions differ.

### 9.4 Cross-Platform And Cost

Use the local build farm with the Linux checkout as the only writer:

```text
Linux   -> Vulkan compile and runtime through Slang RHI
Windows -> D3D12 and Vulkan compile and runtime through Slang RHI
macOS   -> native Metal compile and direct-host runtime
```

Workers receive disposable snapshots and return logs only. Keep runner recipes and logs outside the
repository. Cap every native build at eight jobs.

Also verify:

- Legacy-only tests and the non-ray-tracing suite remain unchanged.
- Importing the module is required to activate structural work.
- Compilation without the import does not load the module or run structural phases.
- Closed schemas do not pay the open-section discovery cost.
