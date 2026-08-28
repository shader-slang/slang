# Block Statement

## Syntax

Block statement:

> **`'{'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(*`stmt`*)\*<br>
> **`'}'`**

## Description

A block statement (also known as compound statement) groups a sequence of [statements](statements.md) into a
single statement. The statements are executed in sequential order.

Every block statement has its own declaration scope, which is nested within the parent scope. Declarations in
a block are visible to later statements in the same block, but not to statements outside of the block.

When control is transferred out of the block (i.e., the block is exited),
[deferred statements](statements-defer.md) scheduled for that block are executed, regardless of how control
is transferred. An exception is the `do` body of the
[`do-catch` statement](statements-do-catch.md), which executes the pending deferred statements at the end
of the `catch` body when an exception is caught. See [`defer` statement](statements-defer.md) for details.

A block may be empty. An empty block does nothing, and it is generally preferable to an
[empty statement](statements-empty.md) when a statement is required but no action is wanted.

## Examples

Block statements and declaration scopes:

```hlsl
RWStructuredBuffer<uint> output;

[numthreads(8,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;
    uint ret = 0;

    if (index >= 5)
    {
        // Block opens a new scope. Variables of the
        // parent scope are visible to the block.
        //
        // Declarations in this scope are visible to
        // later statements in the same block, including
        // nested block statements.
        uint tmp = index * 3U;

        ret = tmp;
    }

    output[index] = ret;

    // Note: 'tmp' is not visible here, so
    //
    //     output[index] = tmp;
    //
    // would not compile
}
```

[For statement](statements-loop.md) with an empty loop body:

```hlsl
// 0-terminated buffer
StructuredBuffer<uint> input;

RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint len;
    uint bufLen = input.getCount();

    // Find the 0 terminator in the input
    for (len = 0;
         len < bufLen && input[len] != 0;
         ++len)
    {
    }

    // report back the input length
    output[0] = len;
}
```
