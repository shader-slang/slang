# Special Topics

## Function Parameters with in/out/inout Modifiers

Arguments to function parameters with `in`/`out`/`inout` directions may be accessed during the lifetime of the
function call as follows:

- The function may read the argument passed to the `in`-parameter any number of times during the function
  call. This is the default direction.
- The function may write the argument passed to the `out`-parameter any number of times during the function call.
- The function may read and write the argument passed to the `inout`-parameter any number of times during the
  function call.

Even if the function does nothing with the parameter, the argument passed to an `in`/`out`/`inout`-parameter
may still be accessed during a function call.

The usual [data race](basics-memory-model-consistency.md#data-race) rules apply to arguments passed to
functions. That is, consider a variable passed as an argument to a function parameter in one thread. A data
race occurs if the same variable is also passed as an argument to an `out`/`inout` function parameter in
another thread, unless one function call completes entirely before the other begins.

In addition, it is [undefined behavior](basics-behavior.md) to pass the same variable as an argument to two
parameters of a function if at least one parameter is `out`/`inout`.

**Example 1:**

```hlsl
RWStructuredBuffer<int> output;

void addConsumeTwoValues(
    inout int i1, inout int i2, inout int o)
{
    o += i1;
    i1 = -1; // mark consumed

    o += i2;
    i2 = -1; // mark consumed
}

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int a = dispatchThreadID.x;
    int b = dispatchThreadID.y;

    // undefined behavior: a is passed twice to
    // inout parameters
    addConsumeTwoValues(a, b, a);

    output[0] = a;
}
```


> 📝 **Remark:** There are two main implementation approaches for `in`/`out`/`inout`-parameters:
> - Read the `in`/`inout` arguments on function entry and write back `out`/`inout` arguments on function
>   exit.
> - Convert `in`/`out`/`inout` arguments to pointers, and replace parameter accesses with pointer indirections.
>
> However, it is unspecified which approach (if either) is used. The exact mechanism is target-dependent.


## Memory Aliasing via Binding

If the same underlying memory is bound to multiple resources such as `ConstantBuffer<T>`,
`RWStructuredBuffer<T>`, or `Texture2D<T>`, it is aliased. For the purposes of data race analysis, aliased
memory access is considered overlapping if the accesses to the underlying memory overlap. Therefore, a data
race can occur even when two concurrent memory accesses use different buffer handles. This happens when one or
both accesses modify overlapping underlying memory, and the accesses are non-atomic without established
happens-before relationships.

The client API may impose additional restrictions on aliased memory.
