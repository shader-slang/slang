# Defer Statement

## Syntax

`defer` statement:

> **`'defer'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`deferred-stmt`*

## Description

A `defer` statement schedules the enclosed statement (*`deferred-stmt`*) to be executed when execution exits
the enclosing scope. The enclosed statement is said to be _deferred_. Multiple deferred statements are
executed in _last-in, first-out_ (LIFO) order.

A `defer` statement may appear only within the body of a [function](declarations-functions.md) or a
[lambda expression](expressions-lambda.md).

The enclosing scope of a `defer` statement is the innermost enclosing
[block statement](statements-block.md). If the `defer` statement is itself a sub-statement of another
statement without an intervening block, for example the body of an [`if` statement](statements-if.md) or a
[loop](statements-loop.md), that sub-statement is the enclosing scope. Note that the enclosing scope
definition is different from the usual [scope](basics-scope.md) definition.

A deferred statement is scheduled when the `defer` statement itself is executed. Local variables that are
declared before the `defer` statement are available to the enclosed statement.

The enclosed statement is evaluated in its entirety at the time it is executed, that is, on exit from the
enclosing scope. No part of it is evaluated when it is scheduled by the `defer` statement.

When the enclosing function or lambda expression returns, deferred statements are executed after the return
value expression has been evaluated.

If a deferred statement is scheduled within the `do` body of a
[`do-catch` statement](statements-do-catch.md) and an error object is caught, the deferred statement is
executed after the `catch` body has been executed. This applies to every scope nested within the `do` body
that is still active when the error object is thrown. The pending deferred statements of those scopes are
executed after the `catch` body, in LIFO order across the scopes. A nested scope that has already exited
normally is unaffected, because its deferred statements were executed at that exit.

If an error object propagates out of the enclosing function or lambda expression, the deferred statements of
the exited scopes are executed before the error object propagates to the caller.

A [`discard` statement](statements-discard.md) does not trigger the execution of deferred statements before
the thread is disabled. As a result, pending deferred statements have no effect.

A [break](statements-break-and-continue.md), [continue](statements-break-and-continue.md),
[return](statements-return.md), or [throw](statements-throw.md) may not escape an enclosing deferred
statement. Similarly, a [try expression](expressions-try.md) within a deferred statement must have
its matching catch handler within that same deferred statement.

> 📝 **Remark 1:** Deferred statements can be useful for cleanup. Once scheduled, they are executed regardless
> of the exit path of the scope, except when the thread is disabled by a `discard` statement.

> 📝 **Remark 2:** Writing a `defer` statement as sub-statement of `if` is rarely useful, because the deferred
> statement is then executed immediately after being scheduled. Use a block statement when the intent is to
> defer to the end of the surrounding block.

> 📝 **Remark 3:** If control never reaches the `defer` statement, nothing is scheduled. If the `defer`
> statement is executed more than once, for example in a loop body, each execution schedules a separate
> execution of the enclosed statement.

> 📝 **Remark 4:** A single `defer` statement never has more than one pending scheduled execution per function
> invocation. See the first example below.

> 📝 **Remark 5:** `defer` statements do not usually have an additional cost over regular statements. In this
> context, scheduling refers to the Slang compiler reordering the execution of the statements, essentially
> moving the execution of deferred statements to after all other statements within the enclosing scope. The
> exceptions are:
> - When the enclosing scope has multiple exits (e.g., `break`, `continue`, `return`), the deferred statements
>   may be replicated at each exit.
> - When a `defer` statement is in the `do` body of a `do-catch` statement, the compiler may replicate
>   statements and add runtime conditions for execution.

> ⚠️ **Warning 1:** The enclosed statement does not currently form a nested [scope](basics-scope.md), so
> declarations within it are visible after the `defer` statement. This is tracked by GitHub issue
> [#12266](https://github.com/shader-slang/slang/issues/12266).

> ⚠️ **Warning 2:** A `defer` statement may appear as a deferred statement. However, since there is little
> reason to ever use a `defer defer` construct, it is possible that it will be diagnosed as an error in a
> future Slang version.

## Examples

Scheduling and evaluation of deferred statements:

```hlsl
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = 1;

    // The enclosed statement reads 'value' when it is executed on
    // scope exit, thus writing 3 to output[0].
    defer output[0] = value;

    value = 3;

    for (uint i = 0; i < 4; ++i)
    {
        if (i == 2)
            continue;

        // Each iteration that reaches this statement schedules its
        // own execution of the enclosed statement, which runs when
        // that iteration exits the loop body. The iteration where
        // 'i' is 2 skips this statement, so output[3] is left
        // untouched.
        defer output[1 + i] = i;
    }
}
```

A deferred statement and error handling:

```hlsl
struct DivisionByZero
{
    uint dividend;
    uint divisor;
}

uint checkedDivide(uint a, uint b) throws DivisionByZero
{
    if (b == 0)
        throw DivisionByZero(a, b);

    return a / b;
}

struct Input
{
    uint dividend;
    uint divisor;
}

struct Result
{
    uint result;
    bool error;
    bool completed;
}

StructuredBuffer<Input> input;
RWStructuredBuffer<Result> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;

    do
    {
        // whatever happens, mark the result completed at the end
        defer output[index].completed = true;

        output[index].result =
            try checkedDivide(
                input[index].dividend, input[index].divisor);
        output[index].error = false;

        // if no error was thrown, the deferred statement is
        // executed here
    }
    catch (err : DivisionByZero)
    {
        // the divisor was zero, so report the dividend as the
        // result and flag the error
        output[index].result = err.dividend;
        output[index].error = true;

        // if an error object is caught, the deferred statement
        // is executed here
    }
}
```

See also [`return` statement](statements-return.md) for examples of the ordering of deferred statement
execution and return value evaluation, and
[`break` and `continue` statements](statements-break-and-continue.md) for an example of deferred statements
and a multi-level `break`.
