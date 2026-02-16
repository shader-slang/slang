# Glossary

[Compute Dispatch](basics-program-execution.md)
: See Dispatch.

[Dispatch](basics-program-execution.md)
: A single dispatch of compute work. A dispatch is an explicit operation that specifies the input parameter
  grid on which thread groups are instantiated.

Entry point (TODO: link)
: A designated function from which a thread begins execution.

[Graphics launch](basics-program-execution.md)
: See Launch.

[Implementation-defined behavior](basics-behavior.md#classification)
: The observable behavior is defined by the implementation, and it is documented in the [target platforms
  documentation](../target-compatibility.md) or documentation provided by the implementation. Implementation
  includes the target language, the device and its driver, and declared extensions and available capabilities.

[Launch](basics-program-execution.md)
: A single launch of graphics work. A graphics launch consists of an unspecified number of draw calls that
  activate the graphics pipeline.

[Mutually convergent set of threads](basics-execution-divergence-reconvergence.md)
: A set of threads in a wave that are on the same uniform control flow path. When the execution has diverged,
  there is more than one such set.

[Observable behavior](basics-behavior.md#observable)
: Program behavior observable over the execution interface. The interface includes resource variables,
  shared memory, and execution control.

[Precisely defined behavior](basics-behavior.md#classification)
: The observable behavior is precisely defined for all targets.

Program (TODO: link)
: A program is a composition of units of linkable code. A program includes a set of entry points, which may be
  invoked by a compute dispatch or by a graphics launch.

[Tangled function](basics-program-execution.md)
: A function in which a set of threads participates. The scope of a tangled function is either wave or thread
  group. Tangled functions include synchronous operations such as control barriers and cooperative functions
  that collect inputs from and distribute outputs to the participating threads.

[Thread](basics-program-execution.md)
: A sequential stream of executed instructions. In Slang, thread execution starts from an entry point
  invocation. The thread terminates when it finishes executing the entry point, when it is discarded, or when
  it exits abnormally.

[Thread group](basics-program-execution.md)
: The second-level group of threads in the execution hierarchy. The thread group size is determined by the
  application within target-specified limits. A thread group executes on the same execution resources, and it
  can communicate efficiently using shared memory (`groupshared` modifier).

[Thread-group-tangled function](basics-program-execution.md)
: A function in which all threads of a thread group participate. Examples include thread-group-level control
  barriers. Unless otherwise stated, it is [undefined behavior](basics-behavior.md#classification) to invoke
  thread-group-tangled functions on non-thread-group-uniform paths.

[Thread-group-uniform path](basics-execution-divergence-reconvergence.md)
: All threads in the thread group are on a uniform path.

[Undefined behavior](basics-behavior.md#classification)
: The observable behavior is not defined. Possible results include crashes, data corruption, and inconsistent
  execution results across different optimization levels and different targets.

[Uniform control flow](basics-execution-divergence-reconvergence.md)
: All threads are on a uniform path.

[Uniform path](basics-execution-divergence-reconvergence.md)
: A control flow path is uniform when the control flow has not diverged or it has reconverged. Divergence
  occurs when threads take different paths on conditional branches. Reconvergence occurs when the conditional
  branches join. Control flow uniformity is usually considered in the thread-group and the wave scopes.

[Unspecified behavior](basics-behavior.md#classification)
: The observable behavior is unspecified but within boundaries. Documentation is not required.

[Wave](basics-program-execution.md)
: The smallest-granularity group of threads in the execution hierarchy. The wave size is a power of two in
  the range [4, 128] defined by the target. Threads in a wave may participate in wave-tangled functions such as
  wave ballots and wave reductions. For an example, see `WaveActiveMin()`.

[Wave-tangled function](basics-program-execution.md)
: A function in which a subset of threads of the wave participates. Typically, the subset consists of the
  active and mutually convergent threads. Wave-tangled functions include reductions such as `WaveActiveMin()`,
  ballots such as `WaveActiveBallot()`, and functions that imply a wave-level control flow barrier such as
  `GroupMemoryBarrierWithWaveSync()`.

[Wave-uniform path](basics-execution-divergence-reconvergence.md)
: All threads in the wave are on a uniform path.

