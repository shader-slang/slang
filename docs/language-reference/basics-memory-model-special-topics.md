# Special Topics

## Function Parameters with in/out/inout Modifiers

Arguments to function parameters with `in`/`out`/`inout` or no modifiers may be accessed during the lifetime of the
function invocation as follows:

- An argument passed to a parameter without `in`/`out`/`inout` modifiers is value-copied when the function is called.
- The function may read the argument passed to the `in`-parameter any number of times during the function call.
- The function may write the argument passed to the `out`-parameter any number of times during the function call.
- The function may read and write the argument passed to the `inout`-parameter any number of times during the
  function call.

Even if the function does nothing with the parameter, the argument passed to an `in`/`out`/`inout`-parameter
may still be accessed during a function call.

The usual [data race](basics-memory-model-consistency.md#data-race) rules apply to arguments passed to
functions. That is, if a variable is passed as an argument to a function parameter twice, there is a memory
access conflict if one or both parameters are `out`/`inout`. The memory access conflict is a data race, and
thus, [undefined behavior](basics-behavior.md), unless one function call happens before the other.

Note that it is possible to induce a function parameter data race in a single thread by passing the same
variable as an argument to two different parameters of the same function if at least one of the parameters is
`out`/`inout`.

> 📝 **Remark:** There are two main implementation approaches for `in`/`out`/`inout`-parameters:
> - Read the `in`/`inout` arguments on function entry and write back `out`/`inout` arguments on function
>   exit.
> - Convert `in`/`out`/`inout` arguments to pointers, and replace parameter accesses with pointer indirections.
>
> However, it is unspecified which approach (if either) is used. The exact mechanism depends on the target, and
> in some cases, also on the downstream compiler.


## Memory Aliasing via Binding

If the same underlying memory is bound to multiple storage buffers such as `ConstantBuffer<T>`,
`RWStructuredBuffer<T>`, or `Texture2D<T>`, it is aliased. For the purposes of data race analysis, aliased
memory access is considered overlapping if the access to the underlying memory is overlapping. Therefore, even
when two concurrent memory accesses are performed via different buffer handles, a data race occurs if one or
both modify the overlapping underlying memory and the memory accesses are non-atomic without established
happens-before relationships.

The client API may impose additional restrictions on aliased memory.
