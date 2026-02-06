# Program Execution

At a high level, Slang program execution is defined as follows:

1. Workload for a Slang program is *dispatched* (compute) or *launched* (graphics).
2. The dispatched or launched workload is divided into entry point *invocations*, which are executed by
   *threads*.

An individual entry point invocation handles one point of parametric input. The input parameters and the entry
point return value are specific to the types of the entry points. For example:
- A fragment shader entry point is invoked once per rasterized fragment. The set of entry point invocations is
  determined by the rasterizer. Per-invocation inputs for the fragment shader come from the rasterizer and the
  vertex shader stage. The output of a fragment shader is a per-fragment value. For example, a vector with
  red/green/blue/alpha color components for an RGBA render target.
- A compute kernel entry point is invoked once per user-defined input parameter point. The inputs are the
  *thread coordinates* that identify the invocation. A compute kernel has no intrinsic output. Instead, it
  stores the results in output buffers.

Inputs and outputs of different graphics shaders and compute kernels are described in more detail in [graphics
shaders and compute kernels](shaders-and-kernels.md).

The graphics launches are determined by the draw calls and the graphics pipeline configuration. How a launch
is precisely divided into shader entry point invocations depends on the target.

A compute dispatch is explicit, and it has an application-defined subdivision structure. For a compute
dispatch, the application defines the input parameter space as a grid of thread groups as follows:

1. A compute dispatch is a user-specified 3-dimensional set of integer-valued points. The user passes a
   3-dimensional grid dimension vector `grid_dim`, which specifies the grid points `g` such that
   `0`&le;`g.x`&lt;`grid_dim.x`, `0`&le;`g.y`&lt;`grid_dim.y`, `0`&le;`g.z`&lt;`grid_dim.z`.
2. For every point in the grid, a thread group is instantiated. The thread group size is similarly specified
   with a 3-dimensional vector `group_dim`. Within the thread group, individual thread invocations `b` are
   instantiated such that `0`&le;`b.x`&lt;`group_dim.x`, `0`&le;`b.y`&lt;`group_dim.y`,
   `0`&le;`b.z`&lt;`group_dim.z`. The thread group dimensions are typically specified in the compute entry
   point as an attribute or as compute dispatch parameters.
3. An individual invocation is executed by an individual thread for every grid and thread group point
   combination. There are a total of
   `grid_dim.x`\*`grid_dim.y`\*`grid_dim.z`\*`group_dim.x`\*`group_dim.y`\*`group_dim.z` invocations per
   dispatch.

In both graphics launches and compute dispatches, individual invocations are grouped into waves. The wave size
is a power-of-two in the range [4, 128] and is defined by the target.

In graphics launches, waves are formed from the launch by a target-defined mechanism. They need not have
more in common than that they belong in the same pipeline stage using the same entry point. In particular, a
wave in a fragment stage may process fragments from different geometric primitives.

In compute dispatches, a wave is subdivided from a thread group in a target-defined manner. Usually, a wave
consists of adjacent invocations but, in general, the application should not make any assumptions about the
wave shapes.

Some waves may be only partially filled when the compute thread group or the graphics launch does not align
with the wave size. In compute dispatches, the thread group size should generally be a multiple of the wave
size for best utilization.


# Thread Group Execution Model

All threads within a thread group execute on the same set of execution resources. This allows a thread group
to share local memory allocated with the `groupshared` attribute. Related barriers include
`GroupMemoryBarrier()` and `GroupMemoryBarrierWithGroupSync()`.

The thread group execution model applies only to compute kernels.


# Wave Execution Model

All threads in a wave execute in the single instruction, multiple threads (SIMT) model.

Threads in a wave can synchronize and share data efficiently using *wave-tangled* functions such as ballots,
reductions, shuffling, control flow barriers with the wave scope, and similar operations. For example, atomic
memory accesses to the same memory location by multiple threads can often be coalesced within the wave and
then performed by a single thread. This can significantly reduce the number of atomic memory accesses, and
thus, increase performance.

Wave-tangled functions operate over all participating threads in the wave. In general, the inputs for a
wave-tangled function are the inputs of all participating threads, and similarly, the outputs of a
wave-tangled function are distributed over the participating threads executing the function.

Usually, the participating threads are the active threads on a [mutually convergent
path](basics-execution-divergence-reconvergence.md).

The threads within a wave belong to one of the following classes:
- *active thread*---a thread that participates in producing a result.
- *inactive thread*---a thread that does not produce any side effects. A thread can be inactive because the
  wave could not be fully utilized when assigning threads. A thread can also be inactive because an active
  thread executed the `discard` statement, which disables the thread (fragment shaders only).
- *helper thread*---a thread that is used to compute derivatives, typically for fragment quads. A helper
  thread does not produce any other side effects, and it does not participate in wave-tangled functions unless
  otherwise stated.

Despite the SIMT execution model, Slang does not require that wave invocations execute in lockstep, unless
they are on a mutually convergent control flow path and they are executing synchronizing functions such as
control flow barriers (*e.g.*, `GroupMemoryBarrierWithWaveSync()`).

> 📝 **Remark 1:** The actual execution hardware may or may not be implemented using an SIMT instruction
> set. In particular, a CPU target would generally not use SIMT instructions.

> 📝 **Remark 2:** In SPIR-V terminology, wave-tangled functions are called *tangled instructions* with the
> subgroup scope.
