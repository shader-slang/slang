# Defer Statement

## Syntax

`defer` statement:

> **`'defer'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`deferred-stmt`*

## Description

A `defer` statement schedules the enclosed statement (*`deferred-stmt`*) to be executed when execution exits
the enclosing scope. The enclosed statement is said to be _deferred_. Multiple deferred statements are
executed in _last-in, first-out_ (LIFO) order.

A `defer` statement may appear only within the body of a [function](declarations-functions.md).

The enclosing scope is the innermost scope containing the `defer` statement. If the `defer` statement is
itself the sub-statement of another statement, for example the body of an [`if` statement](statements-if.md)
written without a [block statement](statements-block.md), that sub-statement is the enclosing scope, and the
deferred statement is executed at the end of it.

A deferred statement is scheduled when the `defer` statement itself is executed. If control never reaches
the `defer` statement, nothing is scheduled. If the `defer` statement is executed more than once, for example
in a loop body, each execution schedules a separate execution of the enclosed statement.

Local variables that are declared before the `defer` statement are available to the enclosed statement.

The enclosed statement is evaluated in its entirety at the time it is executed, that is, on scope exit. No
part of it is evaluated when it is scheduled by the `defer` statement.

The enclosed statement forms a nested scope, limiting the scope of any declarations within it to that
statement.

A `defer` statement may appear within a deferred statement. The nested deferred statement is scheduled when
the enclosing deferred statement is executed, and it is executed when the enclosing deferred statement exits
its scope.

When the enclosing function returns, deferred statements are executed after the return value expression has
been evaluated.

If a deferred statement is scheduled within the `do` body of a
[`do-catch` statement](statements-do-catch.md) and an exception is caught, the deferred statement is
executed after the `catch` body has been executed. This applies to every scope nested within the `do` body
that is still active when the exception is thrown, not only to the body itself. The pending
deferred statements of those scopes are executed after the `catch` body, in LIFO order across the scopes. A
nested scope that has already exited normally is unaffected, because its deferred statements were executed
at that exit.

If an exception propagates out of the enclosing function, the deferred statements of the exited scopes are
executed before the exception is delivered to the `catch` body of the caller.

A [`discard` statement](statements-discard.md) does not trigger the execution of deferred statements before
the thread is disabled. Therefore, the deferred statements of the exited scopes have no effect.

A [break](statements-break-and-continue.md), [continue](statements-break-and-continue.md),
[return](statements-return.md), or [throw](statements-throw.md) may not escape an enclosing deferred
statement. Similarly, a [try expression](expressions-try.md) within a deferred statement must have
its matching catch handler within that same deferred statement.

> 📝 **Remark 1:** Deferred statements can be useful for cleanup. Once scheduled, they are executed regardless
> of the exit path of the scope, except when the thread is disabled by a `discard` statement.

> 📝 **Remark 2:** Writing a `defer` statement as sub-statement of `if` is rarely useful, because the deferred
> statement is then executed immediately after being scheduled. Use a block statement when the intent is to
> defer to the end of the surrounding block.

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

A deferred statement and structured error handling:

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

        // if no error was thrown, the deferred statement is
        // executed here
    }
    catch (ex : DivisionByZero)
    {
        // the divisor was zero, so report the dividend as the
        // result and flag the error
        output[index].result = ex.dividend;
        output[index].error = true;

        // if an exception is caught, the deferred statement
        // is executed here
    }
}
```

See also [`return` statement](statements-return.md) for examples of the ordering of deferred statement
execution and return value evaluation, and
[`break` and `continue` statements](statements-break-and-continue.md) for an example of deferred statements
and a multi-level `break`.
