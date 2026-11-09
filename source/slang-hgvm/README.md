# HGVM runtime notes

## Owned existential and union values

HGBC preserves ownership when interface and union values erase their concrete payload types.
`OpBox` copies a concrete value into immutable heap storage with that type's managed-reference
recipe. `OpUnbox` checks the concrete type before returning an owned copy. Tagged unions carry
an explicit case-to-payload-type mapping; construction, extraction, and conversion validate the
active case and payload together. Different cases may have the same payload type.

Copying an interface value shares its immutable box. Mutating a struct payload extracts a copy
and replaces the box, preserving value semantics. A class payload still refers to the same class
object. Empty default values are uninitialized unions, and a declared void case has no payload.
Raw reinterpretation cannot erase managed references into ordinary bytes; boxes and tagged
unions are host values and cannot be used directly as device data or ray-tracing captures.

## Deferred ray tracing

The initial single-tracker implementation supports `OpRaytracingPipelineType`,
`OpMakeRaytracingPipeline`, `OpSetRayMiss`, and `OpSetRayHitGroup`. Creation accepts
1 through 15 ray slots. A registration owns copies of its typed argument values and retains
their managed resources, so a resource created in a helper stays alive after the helper
returns. Replacing a registration releases the previous values. Entry-point identities remain
deferred: registration does not load device modules or create native pipelines.

A reference to the owning tracker inside an argument is an internal non-owning edge.
References to other trackers, address values, class captures, and runtime-array captures are
rejected in this initial subset. This makes destruction deterministic without introducing a
general cyclic registry graph. Dispatch converts tracker identity to an ordinary table offset;
native pipelines, tables, and descriptor resources are retained independently until GPU completion.

The execution milestone connects `OpDispatchRays` to Vulkan/D3D12 with explicit resource
descriptor handles. Ordinary buffers/textures/AS values captured as shader arguments are
rejected; use `.getDescriptorHandle()` before constructing a callback. Automatic managed-resource
conversion remains a separate compiler task. A dispatch accepts one distinct tracker, binding
its device offset to zero; aliases and self references are valid. The context library supplies
its ray-type count as the hit-table stride. A ray-generation callback need not capture a tracker
or trace any ray. Zero extents produce no dispatch.

Texture handles support the existing single-sample, non-array 2D texture subset on Vulkan/D3D12.
`texture.getDescriptorHandle()` preserves the shader element, access, and storage format. Read
handles expose all mips; writable handles expose mip zero. Depth textures expose depth as float.
Native descriptor views are cached and held through submission completion. Their states are
declared before a pass binds the descriptor heap; mixed read-only/writable descriptor aliases in
one dispatch are rejected. Ordinary resource captures still require explicit handle conversion.

Use the standard `nonuniform(handle)` annotation in device code when descriptor selection can
differ between invocations. For example, a closest-hit callback can use
`nonuniform(materialBufferHandle)[index]` when different SBT records capture different buffers.
An explicit descriptor handle does not itself promise that its index is uniform.

Dispatch retains immutable registration values and descriptor resources while work is queued.
The native pipeline cache describes code and static state; changing captured values or copying
the same shader to another record changes SBT contents without changing the code identity.
The SBT owns backend-aligned record storage; the submission retains both it and the native pipeline.

Record storage grows automatically to fit each shader's reflected captures, with independent
strides for the miss, hit, and callable sections. D3D12 temporarily limits application data to
512 bytes because larger local root constants trigger a driver blocker. The cap excludes the
shader identifier and alignment padding. Vulkan retains the full device limit derived from
`maxShaderGroupStride`, handle size, and record alignment. RHI rejects oversized
reflected layouts when creating the shader program and oversized explicit record data before
copying it. Shader field layouts remain fixed by specialization; record contents and SBT
strides are runtime data and do not add a pipeline cache key or an HGBC operand.
D3D12's rejection explicitly identifies the driver blocker; see the
[RHI API notes](../../external/slang-rhi/docs/api.md).

BLAS and TLAS allocation/build operations query native sizes rather than accepting guessed byte
sizes. Initial BLAS inputs are indexed `float3` triangles. TLAS builds retain an instance snapshot
and its BLAS realizations; native TLAS construction is lazy and cached per snapshot/ray stride.
The runtime derives instance SBT contributions from flattened geometry order. Only full builds
are supported initially; updates/refits are rejected rather than silently treated as builds.

The bundled [ray-tracing context API](../standard-modules/host/runtime/ray_tracing/ray_tracing_context.slang)
provides the Slang interface. See the [ray-tracing guide](../../docs/heterogeneity/ray-tracing.md)
for usage and [runtime tests](../../tests/standard-library/runtime/ray_tracing/) for
regression coverage.

## Autodiff checkpoint reports

Pass `-report-checkpoint-intermediates` before the input filename to report saved
reverse-mode intermediates in both the HGBC host program and shaders compiled on demand:

```powershell
& .\build\windows-vs2022-dev\Release\bin\slangi.exe -api vulkan `
    -report-checkpoint-intermediates `
    examples/host-compute/tensor-affine-fit.slang 2> checkpoints.txt
```

Reports go to stderr. Host reports appear after compilation; downstream shader reports are
collected through RHI and printed when execution finishes. A precompiled `.hgbc` input can
still report its downstream shader checkpoints. The flag requires HGVM; it is not supported
with `-legacy`. It does not require the event-tracking build option.

The byte counts describe checkpoint values retained by the compiler, grouped by original
function and source location. Loop checkpoints include their bounded array storage. Reports
for a caller and its callees can describe nested contexts; do not add them together as if they
were independent allocations. These counts are not GPU register, scratch-memory, or hardware
ray-tracing stack measurements, and do not measure dynamic allocation traffic.

## HGVM event tracking

Configure CMake with `-DSLANG_ENABLE_HGVM_EVENT_TRACKING=ON` to enable instrumentation call sites.
The event API and implementation are compiled in both configurations. The option defaults to
`OFF`; that configuration removes interpreter event calls, argument evaluation,
and tracking fields from heap objects, command batches, and the buffer pool. Link consumers
against the `slang-hgvm` CMake target to inherit the same definition for internal headers.

Use `slangi -trace-all program.slang` (or `slangi -trace-all program.hgbc`) to print readable event
lines to stdout, alongside program output. Each line starts with `[hgvm]`, followed by the
event name and relevant operation details.
For example, dispatch and submission lines share a `batch` ID. Tracing requires a `[host]`
entry point. Use `slangi -trace-n 100 program.slang` to print only the first 100 events. The limit
counts event lines, including cleanup events, and does not affect program output or execution.
Zero suppresses all trace lines. If multiple trace options are supplied, the last one takes effect.
Builds with tracking disabled reject either trace option with a configuration diagnostic.

Module-load, buffer-creation, and pipeline-creation events include `durationNanoseconds`, printed
as `duration-ms` in traces. This measures host elapsed time around the underlying Slang/RHI call,
including failed attempts, and excludes event formatting and delivery. Buffer allocation retries
are timed separately, excluding the intervening pool trim. Timers read the clock only when
tracking is active; timing call sites also compile out with instrumentation disabled. These
durations do not measure GPU execution or work deferred until later by the underlying API.

Draw and dispatch events also carry an `executionId`, printed as `execution`. Each submitted
execution gets a `PipelineExecutionComplete` event (`pipeline-complete` in traces) when HGVM
observes its batch completing. Match it to the draw/dispatch using the same tracker and execution
ID; IDs increase across invocations and query-pool reuse. The completion also repeats the batch
ID, pipeline identity, and pipeline kind. Internal blits use their underlying draw's execution ID.

Native acceleration-structure builds emit `AccelerationStructureBuild` and
`AccelerationStructureBuildComplete`, using the same execution-ID sequence and batch lifecycle.
The `accelerationStructureBuild` payload preserves the native resource identity and BLAS/TLAS
kind. Logical TLAS snapshots produce no build event until binding requires a native realization;
cached realizations produce no additional build event. A lazy build's origin identifies the
binding operation that records it. Its completion preserves that origin.

When RHI supports timestamp queries, two timestamps bracket the pipeline pass or native AS build.
Completion events set `hasDuration` and `durationNanoseconds`, printed as `gpu-duration-ms`.
This measures device elapsed time for the command, including setup, stalls, and overlapping work;
it is separate from CPU pipeline-creation time. Unsupported queries, failed query allocation or
readback, and unavailable/invalid timestamp results leave `hasDuration` false, and the trace omits
`gpu-duration-ms`. The completion is still emitted after queue completion is known.

Results are collected during existing fence retirement or existing queue waits, including
interpreter shutdown. Tracking adds no GPU waits. Query pages move with in-flight submissions,
and only resolved pages are reset and cached for reuse (at most four idle pages). Abandoned
batches and failed submissions do not emit successful execution completions; their discard or
submission event identifies the affected batch. Completions may arrive after later commands have
been recorded, and are observed only when the interpreter next polls or drains submissions.

Include `slang-hgvm-events.h`, implement `Slang::HGVM::IEventSink::onEvent`, and supply it in
the interpreter description:

```cpp
RefPtr<MyEventSink> sink = new MyEventSink;
Slang::HGVM::Desc desc;
desc.device = device;
desc.eventSink = sink;
desc.enableEventTracking = true;
Slang::HGVM::Interpreter interpreter(desc);
```

Tracking is disabled by default at runtime too. The configuration is fixed when constructing
the interpreter. Every tracking method checks the enabled flag before accessing its operands,
constructing an event, reading a clock, or calling the sink. The tracker retains the sink through
delayed cleanup of tracked objects and buffer leases. Supplying no sink disables tracking.

Events arrive synchronously on the HGVM caller's thread. A callback must not throw or re-enter
HGVM. A sink shared by multiple interpreters must synchronize its own output. Sequence numbers
and command batch IDs are local to the tracker identified by `TrackingEvent::tracker`; they
start at one. The timestamp uses the host's monotonic clock, in nanoseconds. Tracker and resource
pointers are temporary identities, can be reused after destruction, and must not be dereferenced
by a consumer. Copy module names during the callback if retaining them.

## Debug maps

`DebugMap::findPipelineName(descriptorWordOffset)` resolves staged compute and graphics pipelines.
Names use the serialized module, entry point, and specialization arguments, for example
`kernels::fill<float, 4>`. Graphics names combine the vertex and fragment entry points with `+`.
During tracking, `event.tracker->findPipelineOrigin(pipeline)` identifies the pipeline's program
and descriptor. Use the interpreter to get that program's map, independently of the event's source
location. `slangi` prints the resulting name on creation, draw, dispatch, and completion events.
The association does not retain the pipeline; handles are valid only for their resource lifetime.
Internal blit pipelines without staged entry points have no debug-map name.

`DebugMap::findLambdaLocation(name, moduleName)` resolves a lambda's reflection name to its
definition path, line, and column. The module is optional; ambiguous names have no result.
HGBC preserves these associations even for closures used only as shader specialization arguments.
`slangi` substitutes `lambda@file(path:line:column)` when displaying pipeline and heap type names,
using paths relative to the working directory where possible. The stored reflection and pipeline
names remain unchanged. Missing debug information leaves the original name visible.

`slang-hgvm-debug-map.h` exposes an immutable debug map for each staged program. Type IDs
remain HGBC IDs; pair an ID with `DebugMap::getProgramId()` to distinguish programs and
specializations with overlapping local IDs. `findType(id)` returns the opcode, qualified Slang
type name (including generic arguments), and declaring module when available. Module names are
preserved separately from reflection names, which intentionally omit the module. Older HGBC
files and builtin types may have no module name; unnamed types may have no qualified name.

Each event's `origin` contains only a program ID and the current staged record's word offset.
During a callback, use `event.tracker->getInterpreter()` and its `getDebugMap(origin.programId)`
to obtain the map, then call `findSourceLocation(origin.recordWordOffset)` in the event handler.
This returns the source path, line/column range, function name, and
available inline call sites. It returns null when that record has no associated line information;
event capture does not collect a stack trace. Source locations use sorted address ranges, with
binary search lookup and explicit ranges without a location. Generated
function cleanup has no instruction location; root cleanup and operations outside an invocation
can have no source location.

Heap events also carry `heap.typeId` and `allocationOrigin`. Resolve the type using
`getDebugMap(allocationOrigin.programId)->findType(event.heap.typeId)` when both are available.
A free keeps the original allocation metadata even when destruction occurs in another invocation or after
the program is released; its separate `origin` describes the freeing operation. Internal helper
objects without a corresponding source-visible result type use type ID zero. Pipeline completion
keeps the same origin as the recorded draw or dispatch, rather than the instruction that happens
to observe completion. The interpreter retains maps until teardown, including after releasing
executable programs. Events retain no maps, interpreter state, or GPU resources. A receiver that
needs to decode saved events after interpreter destruction must retain the maps itself during
callbacks. Trackers surviving teardown return null from `getInterpreter()`.

Type associations are separate, sparse entries at exact record starts. The allocation staging
methods register their emitted record and type explicitly; ordinary records need no entry.
Free events use the type retained on the allocated object, including during shared cleanup.
`StagedRecordBuilder` has no debug-map callbacks. The stager records source-range boundaries
when processing debug directives and entering generated code.

`DebugMapBuilder` builds the map during staging in every configuration, independently of event
tracking. Events capture raw addresses on demand; source lookup occurs in the sink, after
`slangi` checks its event limit. With runtime tracking disabled, HGVM captures no event origins
and performs no event source lookups. The build flag removes runtime instrumentation and event
tracking fields from objects, while leaving debug-map construction available.

Tracing source input requests minimal compiler debug information automatically. For precompiled
input, use `slangc program.slang -target hgbc -g1 -o program.hgbc` to retain line information.
Stripped HGBC still emits events and available type metadata, but cannot provide source lines.
Trace lines append `type`, `module`, and `source` where known; structured sinks can resolve the
full metadata through the event API without parsing those strings.
Source paths under the current working directory are printed relative to it.

The event kind selects the corresponding member of the event's payload union:

| Events                                | Payload                      | Meaning                                                                                               |
| ------------------------------------- | ---------------------------- | ----------------------------------------------------------------------------------------------------- |
| Heap alloc/free                       | `heap`                       | Managed HGVM object lifetime and allocation size, including inline class fields and cleanup metadata. |
| Payload alloc/free                    | `payload`                    | Separate aligned runtime-array storage. Free carries only its address.                                |
| Module load                           | `module`                     | An attempted IR/source module load and its result.                                                    |
| Pipeline creation                     | `pipeline`                   | An attempted compute, graphics, ray-tracing, or blit pipeline creation.                               |
| Buffer/texture creation               | `buffer` / `texture`         | An attempted RHI allocation with descriptor details and result.                                       |
| Buffer reuse/recycle/eviction         | `buffer`                     | Resource identity and size at a pool transition.                                                      |
| Batch begin/discard/submit            | `batch`                      | Batch identity, with submission result and fence value where applicable.                              |
| Draw/dispatch                         | `draw` / `dispatch`          | Work recorded into a batch with a stable execution ID; draws indicate whether indexed.                |
| Pipeline execution complete           | `pipelineExecution`          | Completed execution ID and optional GPU duration.                                                     |
| Acceleration-structure build/complete | `accelerationStructureBuild` | Native BLAS/TLAS build identity, batch and execution IDs, and optional GPU duration on completion.    |
| Blit/clear                            | `textureOperation`           | Texture work recorded into a batch.                                                                   |

Only fields applicable to that event kind are meaningful. Creation failures are reported before
the interpreter propagates its existing error. Operations exposed by pointer-returning RHI
helpers report `SLANG_OK` or `SLANG_FAIL`; result-returning calls preserve their result code.
An internal blit also emits its underlying draw event. A discarded batch's recorded operations
were never submitted. Submission success is not a GPU completion notification, and a fence value
of zero denotes the unfenced path.

Heap events cover managed wrappers and explicit aligned payloads, not all interpreter allocations
(for example, string capacity or program constant storage). Pool eviction means HGVM drops a
buffer reference, not that the driver has freed physical GPU memory. Tracking adds no GPU waits,
extra holds on observed pipelines, file output, or default event buffer.

Class objects use one aligned allocation containing the C++ header, inline fields, and a trailing
table of managed-reference offsets. Field offsets and cleanup offsets are measured from the object
base and computed during staging. Field access adds its staged offset directly to the class handle;
it does not load a payload pointer. The trailing table is copied into the allocation so destruction
does not depend on the staged program's lifetime. Class values remain pointer-sized handles, and
interior field addresses borrow the owning object's lifetime. Runtime arrays still own separate
element storage.
