# Execution Divergence and Reconvergence

Threads are said to be on a *uniform path* or *converged path* when either their execution has not diverged or
it has reconverged. When the threads are on a uniform path, the control flow is said to be *uniform*.

In structured control flow, divergence occurs when threads take different control flow
paths on conditional branches. Threads reconverge when the branches join.

Control flow uniformity is considered in the following scopes:
- *Thread-group-uniform path*: all threads in the thread group are on a uniform path.
- *Wave-uniform path*: all threads in the wave are on a uniform path.

In addition, a *mutually convergent* set of threads refers to the threads in a wave that are on a mutually
uniform path. When the execution has diverged, there is more than one such set.


> 📝 **Remark 1:** All threads start on uniform control flow at the shader entry point.

> 📝 **Remark 2:** In SPIR-V terminology: uniform control flow (or converged control flow) is the state when
> all threads execute the same
> [dynamic instance](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#DynamicInstance) of an
> instruction.

> 📝 **Remark 3:** Uniformity does not mean synchronicity. Even when the threads are on a uniform path, it
> does not mean that their progress is uniform. In particular, threads in a wave are not guaranteed to execute
> in lockstep, even if the programming model follows SIMT. Synchronization can be forced with a control flow
> barrier, but this usually incurs a performance overhead.

> 📝 **Remark 4:** Avoiding long divergent execution paths is often a good strategy to improve performance.


## Divergence and Reconvergence in Structured Control Flow {#divergence}

**`if` statements:**

Divergence occurs when some threads take the *then* branch and others take the *else*
branch. Reconvergence occurs when threads exit the *then* and *else* branches.

Example 1:
```hlsl
// divergence occurs when some threads evaluate
// the condition as `true` and others as `false`
if (cond)
{
    // "then" path
}
else
{
    // "else" path
}
// reconvergence
```

Example 2:
```hlsl
// divergence occurs when some threads evaluate
// the condition as `true` and others as `false`
if (cond)
{
    // "then" path
}
// reconvergence
```

> 📝 **Remark**: There is no divergence when all threads take the same branch.


**`switch` statements:**

Divergence occurs when threads jump to different case groups. Reconvergence occurs when threads exit the
switch statement. Additionally, reconvergence between threads on adjacent case label groups occurs on a switch
case fall-through.

A case group is the set of case labels that precede the same non-empty statement.

Example 1:
```hlsl
// divergence occurs when threads jump to different
// case label groups:
switch (value)
{
// first case group
case 0:
case 2:
    doSomething1();
    break;

// second case group
case 1:
    doSomething2();
    break;

// third case group
case 3:
default:
    doSomething3();
    break;
}
// reconvergence
```

Example 2:
```hlsl
// divergence occurs when threads jump to different
// case label groups:
switch (value)
{
case 0: // first case group
    doSomething1();
    // fall-through

case 1: // second case group
    // reconvergence between the first and
    // the second case group
    doSomething2();

    // fall-through

default: // third case group

    // reconvergence between the second and the third case group
    //
    // all threads are now on the same path

    doSomething3();
    break;
}
// no reconvergence here, since it already happened in
// the default case group.
```

**Loop statements:**

Divergence occurs when some threads exit a loop while the rest continue. Reconvergence occurs when all
threads have exited the loop.

Example 1:
```hlsl
[numthreads(128,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint numLoops = 50 + (threadId.x & 1);
    for (uint i = 0; i < numLoops; ++i)
    {
        // divergence after 50 iterations:
        // - even-numbered threads exit the loop
        // - odd-numbered threads continue for one more iteration
    }
    // reconvergence
}
```

## Thread-Group-Tangled Functions on Divergent Paths

Thread-group-tangled functions are supported only on thread-group-uniform paths. It is
[undefined behavior](basics-behavior.md#classification) to invoke a thread-group-tangled function on a
divergent path.


## Wave-Tangled Functions on Divergent Paths

The wave-tangled functions require special consideration when the execution within the wave has diverged:
1. Not all targets support wave-tangled functions on divergent paths. When unsupported, the results are
   [undefined](basics-behavior.md#classification) when invoked on divergent paths. See
   [target platforms](../target-compatibility.md) for details.
2. When supported, wave-tangled functions apply only between the mutually convergent thread
   set by default. That is, synchronization occurs between those threads that are on the same path.

Example 1:
```hlsl
[numthreads(128,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint minimumThreadId = 0;

    // trigger divergence
    if ((threadId.x & 1) == 0)
    {
        // smallest thread id that took the 'then' branch
        minimumThreadId = WaveActiveMin(threadId.x);
    }
    else
    {
        // smallest thread id that took the 'else' branch
        minimumThreadId = WaveActiveMin(threadId.x);
    }
    // reconvergence
}
```
