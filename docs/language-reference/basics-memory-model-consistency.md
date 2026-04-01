# Memory Consistency

In multi-threaded programming, concurrent correctness is one of the harder aspects to achieve. There are
multiple contributing factors, which include the following:

- Compiler optimizations may reorder and recombine memory accesses to improve performance, and in some cases,
  memory accesses may also be duplicated.
- Hardware memory subsystems may reorder and recombine memory accesses to improve performance.
- Hardware may implement out-of-order execution.
- Concurrent threads or thread groups may execute at different rates.

In general, the appearance of program order is maintained for a single thread. That is, given inputs, a
single-threaded program computes the outputs as if everything were executed in the source program order. This
is sometimes referred to as the *"as-if"* rule. As long as the program contains no [undefined
behavior](basics-behavior.md), its output will be computed according to the program.

The behavior of a multi-threaded program is undefined in the presence of *data races*. Broadly speaking, a
[data race](#data-race) results from two threads accessing the same memory location, where at least one access
is a write, using [non-atomic](#atomics) accesses without established [happens before](#memory-order)
relationships between the accesses.

> ⚠️ **Warning:** `slangc` currently emits incorrect code for `Atomic<T>` for multiple targets. See GitHub
> issue [#10683](https://github.com/shader-slang/slang/issues/10683).

> 📝 **Remark 1:** When feasible, the best way to avoid concurrency issues is to design the program such that
> different threads never write to memory locations that other threads may read or write concurrently. Such
> designs are often desirable for performance reasons, too, since concurrent read/write accesses to shared
> memory locations can become a bottleneck for scaling.

> 📝 **Remark 2:** For more formal definitions of memory consistency, refer to the
> [Vulkan Memory Model](https://docs.vulkan.org/spec/latest/appendices/memorymodel.html#memory-model-program-order),
> the C++23 standard (ISO/IEC 14882:2024),
> or [Multi-threaded executions and data races](https://en.cppreference.com/w/cpp/language/multithread) in the
> C++ reference.

> 📝 **Remark 3:** When available, it is generally advisable to use higher-level constructs rather than direct
> atomic variables and memory barriers. See, e.g.,
> [ConsumeStructuredBuffer&lt;T&gt;](../../../core-module-reference/types/consumestructuredbuffer-07h/index.html)
> and
> [AppendStructuredBuffer&lt;T&gt;](../../../core-module-reference/types/appendstructuredbuffer-06g/index.html).

## Data Race {#data-race}

Two memory accesses conflict if they access overlapping memory locations and at least one is a write. A memory
access conflict is a data race unless:

- the memory accesses are executed by the same thread, or
- the memory accesses are [atomic](#atomics), or
- one memory access *happens before* the other.

The *happens before* relation is established by an atomic load-acquire observing an atomic store-release,
memory barriers, or higher-level constructs provided by the Slang Standard Library.


## Memory Order {#memory-order}

The following memory orders are defined for operations in the current thread:

- *Relaxed memory order* --- no constraints are imposed on other memory accesses.
- *Acquire memory order* --- loads and stores after a load-acquire operation cannot be reordered before the
  operation.
- *Release memory order* --- loads or stores before a store-release operation cannot be reordered after the
  operation.
- *Acquire-Release memory order* --- memory accesses cannot cross an acquire-release operation.
- *Sequentially-consistent memory order* --- acquire-release memory order with sequential total order of the
  operation.

Consequences:

<table>
<tr>
  <th>Program sequence</th><th>Memory order of <code>A</code></th><th>Consequences</th>
</tr>
<tr>
  <td rowspan=5 style="text-align:left">
    In program order:
    <ol>
      <li>Memory access <code>M0</code></li>
      <li>Operation <code>A</code></li>
      <li>Memory access <code>M1</code></li>
    </ol>
  </td>
  <td>Relaxed</td>
  <td style="text-align:left">None</td>
</tr>
<tr>
  <td>Acquire</td>
  <td style="text-align:left"><ul>
    <li><code>A</code> happens before <code>M1</code></li>
  </ul></td>
</tr>
<tr>
  <td>Release</td>
  <td style="text-align:left"><ul>
    <li><code>M0</code> happens before <code>A</code></li>
  </ul></td>
</tr>
<tr>
  <td>Acquire-Release</td>
  <td style="text-align:left"><ul>
    <li><code>M0</code> happens before <code>A</code></li>
    <li><code>A</code> happens before <code>M1</code></li>
  </ul></td>
</tr>
<tr>
  <td>Sequentially-consistent</td>
  <td style="text-align:left"><ul>
    <li><code>M0</code> happens before <code>A</code></li>
    <li><code>A</code> happens before <code>M1</code></li>
    <li><code>A</code> is totally ordered with regard to other operations using sequentially-consistent memory order</li>
  </ul></td>
</tr>

</table>



## Atomic Memory Access {#atomics}

All atomic modifications of a single variable occur in a total order. That is, the modifications are
serialized. For example, if one thread increments a 0-initialized atomic variable by 1 and another by 2, all
threads will observe the increments in the same order. That is, either 0 &rarr; 1 &rarr; 3 or 0 &rarr; 2
&rarr; 3.

The *relaxed* memory ordering with atomic memory access does not provide any ordering relationship with other
memory accesses. All that is guaranteed is that the modifications are atomic and the modifications occur in a
total order specific to the variable.

The *release-acquire* ordering with atomic memory access provides an ordering relationship with other memory
accesses as follows:

- All memory loads and stores by a thread, in the program order, *happen before* an atomic store-release to a
  memory location.
- An atomic load-acquire from a memory location by a thread *happens before* all subsequent memory loads and
  stores in the program order.

Therefore, if thread A performs an atomic store-release X to memory location M, and thread B performs an
atomic load-acquire from M and observes the modification X, then thread B will also observe all side effects
by thread A prior to modification X.

The *sequentially-consistent* ordering implies *release-acquire* ordering with the addition that all atomic
memory accesses using the sequentially-consistent ordering occur in a total order.

The Slang standard library provides atomic memory access primitives as follows:
- [Relaxed atomic operations](../../../core-module-reference/global-decls/atomic.html)
- [Atomic\<T\>](../../../core-module-reference/types/atomic-0/index.html) type

> 📝 **Remark:** It is advisable to use only relaxed-memory-order atomics for Slang code that is intended for
> multiple targets. Most targets do not have native support for other memory ordering semantics.


## Memory Barriers {#barriers}

A memory barrier imposes reordering constraints for memory accesses. Memory accesses before and after a
barrier cannot be reordered across the barrier, matching the acquire-release memory order for the
barrier. That is, memory accesses before the barrier *happen before* memory accesses after the barrier.

There are three [address space](basics-memory-model-address-spaces.md) scopes for a memory barrier:
- *All* --- applies to all memory accesses. That is, device and thread group memory accesses.
- *Device* --- applies to device memory accesses. This includes all storage buffers and images.
- *Thread group* --- applies to thread group memory accesses.

The following memory barrier primitives are provided by the Slang standard library:
- [AllMemoryBarrier()](../../../core-module-reference/global-decls/allmemorybarrier-039.html)
- [AllMemoryBarrierWithGroupSync()](../../../core-module-reference/global-decls/allmemorybarrierwithgroupsync-039gkp.html)
- AllMemoryBarrierWithWaveSync() (TODO: link)
- [DeviceMemoryBarrier()](../../../core-module-reference/global-decls/devicememorybarrier-06c.html)
- [DeviceMemoryBarrierWithGroupSync()](../../../core-module-reference/global-decls/devicememorybarrierwithgroupsync-06cjns.html)
- [GroupMemoryBarrier()](../../../core-module-reference/global-decls/groupmemorybarrier-05b.html)
- [GroupMemoryBarrierWithGroupSync()](../../../core-module-reference/global-decls/groupmemorybarrierwithgroupsync-05bimr.html)
- GroupMemoryBarrierWithWaveSync() (TODO: link)


## General Programming Guidance

### Minimize the Number of Atomic Memory Accesses

Atomic memory accesses by multiple threads in a warp can be expensive. When feasible, a single thread should
be elected to perform the atomic memory access.

For example:

```hlsl
RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<Atomic<uint>> syncBuffer;

[numthreads(64,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // write some output...
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;

    // Synchronize the wave and issue a memory barrier.
    //
    // For all threads in the wave, the writes to
    // output happen before the write that signals completion
    AllMemoryBarrierWithWaveSync();

    // signal the completion by the first thread of the wave
    if (WaveIsFirstLane())
    {
        syncBuffer[0].store(1, MemoryOrder.Relaxed);
    }
}
```

A similar consideration applies to reduce operations, which are typical in neural network computation. It is
often better to first collate the results within a wave (e.g., `WaveActiveSum()`) and then use a single thread
to perform the atomic operations to write the results to shared memory.

> 📝 **Remark:** For shared memory reduction operations, atomic reductions that do not provide return values
> may provide better performance than atomic operations that do. See, e.g.,
> [Atomic&lt;T&gt;.reduceAdd()](../../../core-module-reference/types/atomic-0/reduceadd-6.html).
