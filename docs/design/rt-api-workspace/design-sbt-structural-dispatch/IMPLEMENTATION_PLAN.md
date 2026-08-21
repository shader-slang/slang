# Structural Ray Tracing API Implementation Plan

Status: implementation plan for [PROPOSAL.md](PROPOSAL.md).

The proposal defines the source API and its semantics. This document defines the implementation
boundaries, compiler order, milestones, and acceptance requirements.

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

`core` never depends on `slang.raytracing`. The compiler loads the module only after an explicit
import, and the import requires `-experimental-feature`. Build and install it after `core`, following
the `slang.neural` standard-module model; it does not need to participate in core bootstrap.

### 1.2 Implementation Boundary

| `slang.raytracing` owns | Compiler owns |
| --- | --- |
| Public interfaces, contexts, primitives, groups, slots, and layouts | Canonical role recognition and illegal-use diagnostics |
| Generic constraints and placeholder stages | Structural entry-point lookup and layout discovery |
| Zero-storage stage-input properties | Reachable property-use analysis and adapter synthesis |
| Shader-side wrappers over existing intrinsics | Metal tags, tables, descriptor resources, and dispatch |

Implement behavior in the module unless it requires compiler-owned entry points, unavailable target
ABI state, or a restriction ordinary Slang cannot express. Reuse existing ray tracing operations,
capability propagation, legalization, and emission wherever possible.

All compiler behavior is guarded by use of the canonical standard-module declarations. Without the
import, the module is not loaded. Without a selected structural entry or trace program, the new
lowering phases do no work.

Version one excludes SER, `intersection_function_buffer`, and `user_data`. Target-specific features,
including curves and Metal multilevel acceleration structures, remain capability-gated.

### 1.3 Expected Compiler Impact

| Area | Impact | Constraint |
| --- | --- | --- |
| Front end | Medium | Add canonical-role checks and structural stage entry lookup |
| IR, linking, and specialization | Medium | Preserve derived interface identity and selected witnesses |
| D3D/Vulkan lowering | Low | Reuse native ray tracing paths |
| Binding and reflection | Medium | Extend parameter groups for Metal function-table resources |
| Metal lowering | High | Generate tables, candidate functions, and post-trace dispatch |

## 2. Planned Repository Layout

Keep feature logic in owned files and limit existing compiler files to narrow scheduling or query
hooks. The layout below is part of the implementation contract.

### 2.1 Standard Module

```text
source/standard-modules/
├── CMakeLists.txt
├── README.md
├── slang-standard-module-config.h.in
└── raytracing/
    ├── CMakeLists.txt             # Build raytracing.slang-module
    ├── raytracing.slang          # Module declaration and ordered includes
    ├── ray-types.slang           # Rays, traversal, acceleration structures, primitives
    ├── contexts.slang            # Trace and shader-group contexts
    ├── internal-operations.slang # Non-public compiler operations
    ├── stage-inputs.slang        # Zero-storage inputs and properties
    ├── stage-contracts.slang     # Executable interfaces and placeholders
    ├── program-layout.slang      # Slots, groups, lists, and layout
    ├── descriptor.slang          # Opaque descriptor resource
    └── trace.slang               # RayTracer and structural trace call
```

Each included source file uses `implementing raytracing;` and exposes public declarations only in
`namespace rt`. Keep compiler operations internal and expose them through typed properties or
methods.

The parent standard-module CMake file owns the common compiler selection, output directory,
aggregate `slang-standard-modules` target, and one shared install rule. The ray-tracing child owns
only its source list and `slang-raytracing-module` compile target. The installed artifact is named
exactly `slang/raytracing.slang-module`, matching generic standard-module lookup. Do not add a
ray-tracing-specific path or module-name field to the session.

### 2.2 Compiler Files

New feature-owned files under the existing flat `source/slang` convention:

```text
source/slang/
├── slang-structural-raytracing.{h,cpp}              # Roles and canonical registry
├── slang-check-structural-raytracing.cpp            # Front-end checks
├── slang-ir-structural-raytracing.{h,cpp}           # Shared IR queries
├── slang-ir-synthesize-structural-raytracing.{h,cpp} # Retention and synthesis
├── slang-ir-metal-structural-raytracing.{h,cpp}     # Metal lowering
└── slang-reflection-structural-raytracing.{h,cpp}   # Logical SBT reflection
```

Front-end files use AST declarations and no IR. Shared IR files contain no AST lookup. Synthesis is
target-neutral and emits existing ray tracing IR for D3D/Vulkan. Metal mechanics remain in the
Metal-owned file, and reflection consumes only the canonical logical layout.

Existing files receive only integration changes:

```text
source/standard-modules/{CMakeLists.txt,neural/CMakeLists.txt}
source/slang/{slang-session.cpp,slang-check-*.cpp,slang-check-impl.h}
source/slang/{slang-lower-to-ir.cpp,slang-ir-insts.lua,slang-ir.h,slang-ir.cpp}
source/slang/{slang-ir-insts-stable-names.lua,slang-ir-link.cpp,slang-emit.cpp}
source/slang/{slang-parameter-binding.cpp,slang-type-layout.cpp}
source/slang/{slang-ir-metal-legalize.cpp,slang-emit-metal.cpp}
source/slang/{slang-reflection-api.cpp,slang-reflection-json.cpp,slang-diagnostics.lua}
include/slang.h
```

D3D/Vulkan do not receive feature-specific target files. If significant structural algorithms
begin accumulating in any integration file, move them into the appropriate owned file above.

### 2.3 Compiler And Code-Generation Tests

```text
tests/ray-tracing-2/
├── support/
├── frontend/
│   ├── module/
│   ├── contracts/
│   ├── entry-point/
│   ├── diagnostics/
│   └── capabilities/
├── ir/
│   ├── identity/
│   ├── serialization-linking/
│   ├── liveness/
│   ├── requirements/
│   └── synthesis/
├── target/
│   ├── portable/
│   ├── d3d/
│   ├── vulkan/
│   └── metal/
├── reflection/
├── compatibility/
│   ├── legacy-only/
│   ├── mixed-api/
│   └── no-import/
├── runtime/
│   ├── shaders/
│   └── expected/
├── integrate/
└── coverage-manifest.md
```

Portable sources contain multiple target directives instead of duplicated shader files. Backend
directories contain only genuinely target-specific behavior. `support/` contains imported helper
modules with no test directive. `integrate/` contains complete programs that exercise layout,
tracing, stage dispatch, and target emission together. The coverage manifest maps every existing
ray tracing test scenario to its structural integration coverage, supported targets, runtime
coverage, and Metal validation status so no scenario disappears silently.

### 2.4 Runtime Tests And Local Platform Runners

Keep the test shaders and expected results under `tests/ray-tracing-2/runtime/`. Use separate host
implementations because Slang RHI does not support Metal:

```text
tools/gfx-unit-test/structural-ray-tracing/       # D3D12 and Vulkan through Slang RHI
├── structural-ray-tracing-tests.cpp
├── structural-ray-tracing-test-util.{h,cpp}
└── structural-ray-tracing-scenes.{h,cpp}

tools/metal-structural-raytracing-test/           # Local macOS validation through native Metal
├── CMakeLists.txt
├── main.mm
├── metal-test-host.{h,mm}
└── metal-test-scenes.{h,mm}
```

The RHI harness owns the D3D12/Vulkan pipelines, SBTs, acceleration structures, dispatch, and
readback. The Metal host performs the equivalent work directly with native Metal APIs, including
IFT/VFT and descriptor setup. Both hosts use the same portable shaders and expected results; Metal
capability tests such as curves and multilevel acceleration structures remain separate.

The Metal host is excluded from default builds, installation, deployment, and the regular test
suite for version one. It is built and run explicitly on the local macOS worker to validate the
generated code and runtime behavior.

Cross-platform execution uses the local build farm with the Linux checkout as the only writer:

```text
Linux runner    -> Vulkan compile and runtime through Slang RHI
Windows runner  -> D3D12 and Vulkan compile and runtime through Slang RHI
macOS runner    -> native Metal compilation and direct-host runtime, local only
```

The runner recipe and logs remain outside the repository:

```text
~/.codex/local-build-farm/projects/slang-structural-raytracing.json
~/.codex/local-build-farm/runs/slang-structural-raytracing/<run-id>/
```

Workers receive disposable snapshots and return logs only; all fixes are made in the Linux
workspace.

## 3. Front-End Contract

### 3.1 Stage Contracts And Inputs

Each executable stage interface carries its logical stage requirement. For example:

```slang
[require(closesthit)]
public interface IClosestHitShader<Context>
    where Context : IHitContext
{
    void invoke(ClosestHitInput<Context> input);
}
```

Apply the corresponding requirement to *AnyHit*, *Intersection*, *Miss*, and *Callable*. Validate
each `invoke` witness and its reachable helpers against that logical stage. The metadata-only
`IIntersectionStage` remains stage-neutral so it can also represent `NoIntersection`.

All stage inputs are compiler-provided, zero-storage property views. Payload, built-ins, primitive
data, and intersection reporting are properties or methods mapped to compiler-known operations;
they are never stored fields. Put common properties on the input type and primitive-specific
properties in constrained extensions.

Map properties to existing core ray tracing operations when they already express the required
semantics. Add a new IR operation only for state or behavior with no existing representation.

Generic constraints enforce context, payload, primitive, attribute, and group agreement. Full-layout
validation is limited to facts unavailable until group packs and slots are concrete.

### 3.2 Structural Use Rules

These restrictions are hard semantic errors and remain active under `-ignore-capabilities`.
`[require(stage)]` separately validates which operations are legal inside a stage body.

| Kind | Allowed | Rejected |
| --- | --- | --- |
| Stage implementation | Conformance, type-only layout use, reflection, and compiler-selected entry | User construction, runtime storage, existential conversion, or direct `invoke` call |
| Stage input | Compiler-provided `invoke` parameter and same-stage helper flow | User construction, binding, storage, return, or cross-stage use |
| Slot, group, group list, or program layout | Associated types and static metadata | Runtime materialization |
| `TraceProgramDescriptor<Layout>` | Opaque resource binding and trace argument | Shader construction, field access, or copies outside opaque-resource rules |
| `RayTracer<Layout>` | Local zero-storage facade | Calling `trace` from a stage where tracing is unavailable |

The descriptor type itself is stage-neutral. `RayTracer.trace` uses the same stage capability as the
existing `TraceRay` operation, preserving recursive traces from *ClosestHit* and *Miss*.

### 3.3 Structural Entry Points

Entry lookup must accept a conforming struct as well as a function:

```text
-stage closesthit -entry ClosestHit
```

This lookup happens before IR generation. Resolve the struct through the canonical interface
declarations, validate its conformance against `-stage`, and diagnose a mismatch immediately. The
public entry-point and reflection name is the struct name.

A stage struct can compile without an `ITraceProgramLayout` or descriptor. Its context, primitive,
and reachable property uses provide the native signature. Hit attributes come from
`Context.Primitive.Attributes`, never from an `IHitGroup`. D3D/Vulkan emit a native stage adapter;
Metal emits the corresponding standalone helper. A runnable Metal trace path still requires a
selected layout.

Multiple entry-point components follow existing Slang behavior. An explicit stage entry roots only
that stage. A selected whole layout roots all of its stages. Merely declaring a layout roots no
executable code.

### 3.4 Mixed API Use

During semantic checking, classify each module as using the legacy API, the structural API, both, or
neither, and retain representative source locations. Diagnose a module that directly uses both APIs
before IR generation. Imported declarations alone do not count as use.

Serialize this summary with the module. After program composition, diagnose cross-module mixing in
the selected linked program; legacy stages do not need to be ordinary call-graph callees. Legacy use
includes user-authored *ClosestHit*, *AnyHit*, *Intersection*, *Miss*, and *Callable* entry points or
direct calls to `TraceRay` and `TraceMotionRay`. Ignore internal calls made by `slang.raytracing`,
and allow ordinary ray-generation entry points that call `RayTracer<Layout>`. Structural use
includes conformance to the canonical stage or layout contracts and use of a structural descriptor
or trace call.

## 4. Compiler Representation

### 4.1 Canonical Stage Interfaces

The compiler registers the exact executable interface declarations from the trusted
`slang.raytracing` module. A user interface with the same name or structure remains ordinary.
Only a compiler-designated build of that packaged standard module may create the special IR
operations; source code and a user-shadowing module cannot request them.

The canonical interfaces use normal interface requirements and witness tables, but lower to this IR
hierarchy:

```text
IRInterfaceType
    IRRaytracingStageInterface
        IRClosestHitStageInterface
        IRAnyHitStageInterface
        IRIntersectionStageInterface
        IRMissStageInterface
        IRCallableStageInterface
```

A stage implementation remains an `IRStructType`. Its ordinary `IRWitnessTable` refers to the
corresponding derived interface type. The selected witness, rather than a source name, determines
the logical stage role.

Create the derived operations when compiling the standard module. Preserve and register them when
loading its serialized form. Audit interface factories, exact `kIROp_InterfaceType` checks, cloning,
serialization, linking, specialization, and generic-wrapper resolution so the derived identity is
never replaced by an ordinary interface type.

### 4.2 Other Structural Identity

| Source concept | Representation |
| --- | --- |
| Stage input | Compiler-known zero-storage IR type carrying logical stage and concrete context |
| Stage-input property | Existing intrinsic IR, or a dedicated property operation when required |
| Slots, groups, lists, and layouts | Ordinary types and witnesses canonicalized after specialization into compiler-side layout metadata |
| `TraceProgramDescriptor<Layout>` | Opaque resource associated with one specialized layout |
| `RayTracer.trace` | Existing trace path with a compiler-private `ProgramLayout` marker |
| Selected stage | Concrete `invoke` function plus its stage witness and temporary liveness root |

Preserve the `ProgramLayout` marker through linking and specialization. D3D/Vulkan erase it after
adapter and reflection generation; Metal consumes it when generating traversal and dispatch.

Direct calls to a concrete `invoke` can become ordinary `IRCall` instructions, so structural-use
diagnostics must run in semantic checking. IR identity supports downstream discovery and validation;
it is not the only enforcement mechanism.

## 5. Compilation Flow

The implementation adds two guarded compiler phases: structural synthesis before DCE and target
structural lowering before existing target legalization.

```text
semantic checking
    register canonical declarations
    validate stage capabilities and structural uses
    resolve structural -entry selections
    record user-authored legacy ray tracing use

IR generation, linking, and specialization
    emit and preserve structural identity
    install temporary roots before simplification
    retain explicitly selected or layout-reachable stages

structural synthesis
    canonicalize selected layouts
    resolve stage witnesses and slots
    collect reachable ABI and Metal-tag requirements
    diagnose mixed API use
    generate target adapters and Metal helpers
    remove temporary liveness roots

dead-code elimination
    generated functions retain selected stages
    unselected stages are removed

target structural lowering
    lower the descriptor and structural trace marker

existing target legalization and emission
```

Both new phases return immediately when no selected structural entry or trace program is present.

### 5.1 Layout Discovery And Liveness

For each selected concrete `ITraceProgramLayout`:

1. Expand the hit, *Miss*, and *Callable* group packs.
2. Resolve each group slot, context, primitive, and executable stage witness.
3. Omit placeholder *ClosestHit*, *AnyHit*, and *Intersection* stages.
4. Diagnose invalid or duplicate slots.
5. Produce one canonical layout shared by code generation and reflection.

Temporary liveness applies only to explicit structural entries and witnesses reachable from a
selected layout or trace marker. Install those roots before linking or specialization can simplify
uncalled `invoke` methods. Generate adapters after specialization and before DCE, then remove the
temporary roots. Generated calls and table metadata become the permanent roots.

### 5.2 Reachable Requirements And Adapters

Walk the specialized call graph from each selected `invoke` and collect only:

- stage-input properties that affect the target ABI;
- operations that contribute Metal tags; and
- operations requiring structural transformation, such as Metal `reportHit` handling.

Continue using the existing capability system for ordinary target requirements. Ignore property
uses eliminated by specialization.

Generate each native signature from mandatory target ABI parameters plus the data required by the
reachable properties. The adapter performs a trusted compiler dispatch to `invoke`; user code cannot
make that call directly. Its public symbol and reflection name remain the stage struct name, while
its private identity includes the layout, section, slot, and stage kind to avoid collisions.

Generated Metal functions are also trusted dispatch boundaries. Validate every source stage body
under its own logical stage, but do not combine mutually exclusive logical stage atoms when one
physical Metal function dispatches both *Intersection* and *AnyHit*, or *ClosestHit* and *Miss*.
Non-stage capability requirements still propagate normally.

## 6. Target Implementation

| Concern | D3D/Vulkan | Metal |
| --- | --- | --- |
| Stage code | Generate native entry adapters | Generate candidate and visible functions |
| Trace call | Reuse existing `TraceRay` lowering | Generate traversal and post-trace dispatch |
| `reportHit` | Reuse native reporting | Lower through generated per-hit-group candidate state |
| Attributes | Reuse native hit-attribute ABI | Transport custom attributes in private generated `ray_data` |
| Descriptor | No physical shader resource | Lower through the parameter-group binding system |
| SBT records | Host builds native SBT from reflection | Generate function tables and record-data buffer bindings |

### 6.1 D3D And Vulkan

Reuse existing payload, hit-attribute, `ReportHit`, trace, legalization, and emission paths. After
retaining layout reflection and generated entry symbols, erase the shader-visible descriptor. Native
SBT construction remains a host responsibility.

### 6.2 Metal

Implement the following units:

1. **Tag inference:** collect and normalize the sources defined in
   [PROPOSAL.md Section 2.5](PROPOSAL.md#25-inferring-the-metal-tag-list), and diagnose conflicts
   before emission.
2. **Function dispatch:** generate one candidate function per concrete hit group, visible functions
   for *ClosestHit*, *Miss*, and *Callable*, IFT-to-logical-slot mapping, and post-trace selection.
3. **Candidate reporting:** lower every source `reportHit` with portable range/current-distance
   behavior, Boolean feedback, closest accepted candidate retention, payload writes on rejection,
   and accept-and-end unwinding through helper calls. Dispatch *AnyHit* only when it exists and the
   candidate opacity and ray flags allow it.
4. **Custom attributes:** carry the committed custom attributes and hit kind in private generated
   `ray_data` alongside the user payload. Rejected candidates never overwrite committed attributes.
5. **Descriptor binding:** specialize `TraceProgramDescriptor<Layout>` into an existing
   parameter-group layout containing the IFT, visible-function tables, and record-data buffer. Reuse
   the normal allocator, legalization, and binding reflection.

The private generated state is absent from source reflection and contributes no Metal tag. Target
ABI size reporting must still account for it. Validate custom attribute types against the portable
D3D/Vulkan hit-attribute rules and the Metal representation. `ignoreHit()` must immediately unwind
the logical *AnyHit* invocation, including when called through a helper.

## 7. Implementation Milestones

### Phase 0: Prototype Integration Points

- Prototype a canonical stage-interface IR operation and preserve it through standard-module
  serialization and import.
- Generate one *Miss* and one *ClosestHit* adapter after specialization and before DCE.
- Preserve the `ProgramLayout` trace marker through linking and specialization.
- Specialize a mock descriptor through the parameter-group binding system.

Exit: the insertion points, marker representation, adapter layout path, and descriptor specialization
path are proven with focused tests.

### Phase 1: Module And Front End

- Add, build, install, and gate `slang.raytracing`.
- Implement the public contracts and property-only stage inputs.
- Implement and preserve the canonical stage-interface IR hierarchy.
- Register canonical declarations and implement structural-use diagnostics.
- Add struct-based structural entry lookup and stage-capability validation.

Exit: proposal examples type-check through the explicit import, standalone stages resolve by struct
name, and invalid structural uses receive early diagnostics.

### Phase 2: Structural Synthesis And D3D/Vulkan

- Implement selected-stage liveness and structural synthesis.
- Canonicalize layouts and collect reachable requirements.
- Generate native adapters and structural reflection.
- Diagnose mixed structural and legacy API use.
- Implement the D3D12/Vulkan Slang-RHI runtime harness.
- Add integration coverage for every in-repository non-SER ray tracing test scenario, and run D3D12
  on Windows and Vulkan on Linux and Windows.

Exit: the complete integration suite compiles, its portable runtime cases pass on D3D12 and Vulkan,
and no structural IR remains in emitted target code.

### Phase 3: Metal

- Implement tag inference, function-table resources, and descriptor binding.
- Generate candidate functions, visible functions, and post-trace dispatch.
- Implement `reportHit`, *AnyHit* control flow, and opaque custom-attribute transport.
- Implement the local native Metal test host.
- Generate the integration suite, compile it with the native Metal compiler, and run the portable
  and Metal-specific runtime cases through that host on macOS.

Exit: all generated Metal is accepted by the native compiler, and reflection agrees with generated
tags, functions, logical slots, and resource bindings. The local Metal runtime suite also passes;
deploying that host is not required.

### Phase 4: Hardening

- Complete serialization, reflection, diagnostics, and target-specific tests.
- Run focused legacy-only compatibility tests and the non-ray-tracing regression suite unchanged.
- Measure compilation with and without importing `slang.raytracing`.

Exit: the supported target matrix passes without changing legacy behavior, and the module remains
unloaded when it is not imported.

## 8. Test Plan

Add focused tests with each implementation phase. The integration suite exercises complete ray
tracing pipelines; it does not replace focused compiler regression tests.

### 8.1 Module And Front-End Tests

- Explicit import succeeds only with `-experimental-feature`; compilation without the import does
  not load `slang.raytracing`.
- The precompiled module preserves its canonical declarations and special IR identity. A user
  module with matching names or shapes remains ordinary.
- Generic constraints diagnose context, payload, primitive, attribute, group, and slot errors.
- Each stage contract rejects operations unavailable in its logical stage, including transitive
  helper use.
- Construction, storage, escape, cross-stage input use, and direct `invoke` calls are rejected.
  These diagnostics remain active under `-ignore-capabilities`.
- `-entry ClosestHit -stage closesthit` selects a struct and preserves its name in reflection;
  incompatible `-entry`/`-stage` pairs fail before IR generation.
- A standalone procedural stage obtains attributes from `Context.Primitive.Attributes` without a
  program layout or descriptor.
- Same-module mixed API use fails in the front end. Cross-module mixing fails after composition.
  Imported declarations and internal calls from `slang.raytracing` do not create false positives.

### 8.2 IR And Pipeline Tests

- Derived stage-interface operations and ordinary witnesses survive serialization, cloning,
  linking, generic specialization, and interface wrapping.
- The `ProgramLayout` marker survives linking and specialization and is removed by target lowering.
- Explicit entries and stages in a selected layout survive early simplification. Unselected stages
  and stages in an unselected layout are removed after adapter generation.
- Group-pack expansion produces the canonical layout, diagnoses duplicate or invalid slots, and
  omits `NoClosestHit`, `NoAnyHit`, and `NoIntersection` functions.
- Direct and transitive property uses affect generated signatures and Metal tags; uses removed by
  specialization do not.
- Single-entry, multi-entry, and whole-layout compilation retain exactly the requested stages.
- Generated symbols, reflection names, logical slots, payload binding, and attribute binding remain
  stable.

### 8.3 D3D And Vulkan Tests

- Generate valid native adapters for every supported stage and primitive combination.
- Verify payload, hit attributes, `ReportHit`, recursive trace calls, and placeholder omission.
- Verify that the descriptor has no physical shader binding while structural layout reflection is
  retained.
- Add structural integration coverage for every ray tracing test scenario under this repository's
  `tests/` tree, with SER as the only exclusion, and compile and run the complete suite on both
  targets.

If an in-scope non-SER test cannot be expressed, treat it as an API or implementation gap rather
than adding another exclusion.

### 8.4 Metal Tests

- Cover every valid primitive and stage combination from the proposal, including absent
  *ClosestHit*, *AnyHit*, and *Intersection* stages.
- Cover every tag source and conflict rule, and verify that specialized-away property uses do not
  contribute tags.
- Validate IFT entries, visible-function tables, logical-slot mappings, post-trace dispatch,
  descriptor bindings, and reflection.
- Exercise zero, one, and multiple `reportHit` calls; absent or bypassed *AnyHit*; opaque candidates;
  ray flags; payload writes on rejected candidates; and closest accepted candidate replacement.
- Exercise `ignoreHit()` and accept-and-end through nested helpers and verify that their control flow
  unwinds the correct logical stages.
- Verify custom attributes for accepted and rejected bounding-box candidates. Private generated
  `ray_data` must add no tag and must not appear in source reflection.
- Generate Metal for the complete integration suite and compile it with the native Metal compiler
  on macOS.
- Build the native Metal host and run the portable and Metal-only runtime cases locally on macOS.
  This host is not installed, deployed, or part of the regular test suite in version one.

### 8.5 Cross-Platform Runtime Tests

- Run the same portable shader cases through Slang RHI on Vulkan/Linux and D3D12/Vulkan/Windows,
  then through the native test host on Metal/macOS. The host implementations may differ, but their
  expected result records must match. A compile-only result does not satisfy local runtime
  validation.
- Cover triangle hit and miss selection, payload mutation, multiple logical slots, *Miss*,
  *ClosestHit*, *AnyHit*, *Callable*, procedural intersection, multiple `reportHit` calls, custom
  attributes, and recursive tracing.
- Add Metal-only runtime cases for curves and multilevel acceleration structures.
- Build deterministic scenes, write compact result records to a buffer, read them back, and compare
  exact stage IDs, instance paths, payload values, hit distances, hit kinds, and attributes.
- Skip a target-specific case only when the device lacks its declared capability, and record the
  skip in the local-build-farm summary.
- Run workers from disposable snapshots and retain the per-platform build, compiler, and runtime
  logs under the local-build-farm run directory.

### 8.6 Compatibility And Cost Tests

- Keep focused legacy-only ray tracing tests for backward-compatibility coverage.
- Run the non-ray-tracing regression suite unchanged.
- Measure compilation with and without `import slang.raytracing`; the no-import path must not load
  the module or run structural lowering.
