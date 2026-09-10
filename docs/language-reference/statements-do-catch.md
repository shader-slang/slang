# Do-catch Statement

## Syntax

`do-catch` statement:

> **`'do'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`do-body-stmt`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(*`catch-clause`* | *`catch-all-clause`*)+
>
> *`catch-clause`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'catch'`** **`'('`** *`param-decl`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`catch-body-stmt`*
>
> *`catch-all-clause`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;**`'catch'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`catch-body-stmt`*

## Description

A `do-catch` statement defines an exception handling block. If an exception is thrown within the `do` body,
its type is matched against the catch handlers. If a catch handler matches, control is transferred to its
`catch` body. If none of the catch handlers match, the exception is passed through the `do-catch`
statement and propagates to the rest of the exception handling stack.

A catch handler matches if:

- its parameter declaration type is the same as the type of the thrown exception (*`catch-clause`*); OR
- the catch handler matches any type (*`catch-all-clause`*).

The catch handlers are matched in the order they appear in the `do-catch` statement, and control is
transferred to the first matching handler. Since a *`catch-all-clause`* matches any type, a catch handler
that follows it is unreachable.

Implicit conversion is not performed in `throw`/`catch` exception handling. The type of the thrown object must
exactly match the parameter declaration type of a catch handler, unless the catch handler is a catch-all
handler.

A call to a function that may throw must be made with a [try expression](expressions-try.md).

See [`throw` statement](statements-throw.md) for a description of the exception handling stack.

See [`defer` statement](statements-defer.md) for the execution of deferred statements when an exception is
thrown and caught.

## Examples

```hlsl
struct ErrorObject
{
}

enum ErrorCode
{
    SomeError = 3,
    AnotherError = 4
}

void f0(uint v) throws ErrorObject
{
    if (v == 1)
        throw ErrorObject();
}

void f1(uint v) throws ErrorObject
{
    if (v == 2)
        throw ErrorObject();
}

void g(uint v) throws ErrorCode
{
    if (v == 3)
        throw ErrorCode.SomeError;
    if (v == 4)
        throw ErrorCode.AnotherError;
}

void h(uint v) throws uint
{
    if (v >= 5 && v <= 7)
        throw v;
}

void exceptionHandlingFunction(uint v) throws ErrorCode
{
    do
    {
        try f0(v); // may throw ErrorObject (handled below)
        try f1(v); // may throw ErrorObject (handled below)
        try g(v);  // may throw ErrorCode (not handled below,
                   // propagates to the caller)
        try h(v);  // may throw uint (handled below)
    }
    catch (ex : ErrorObject)
    {
        // TODO: handle exception from f0() or f1()
    }
    catch (ex : int)
    {
        // unreachable block (no implicit conversions)
    }
    catch (uint ex) // traditional declaration style is also allowed
    {
        // TODO: handle exception from h()
        //
        // The value of parameter `ex` is the value thrown
        // in the throw statement (5, 6, or 7)
    }
}

[numthreads(16,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    do
    {
        try exceptionHandlingFunction(tid.x);
    }
    catch
    {
        // Catch-all handlers are typically used when the
        // precise exception information is not required
        //
        // TODO: handle exception
    }
}
```
