# Throw Statement

## Syntax

`throw` statement:

> **`'throw'`** *`throw-expr`* **`';'`**

## Description

A `throw` statement throws the value of *`throw-expr`* as an error object. An error object must either be
[caught](statements-do-catch.md) within the enclosing function, or the enclosing
[function](declarations-functions.md) must have a corresponding `throws` clause.

When an error object is thrown with a `throw` statement, control is transferred to the first matching catch
handler in the error handling stack. If that handler declares a parameter, the thrown error
object is passed to it.

A catch handler matches if:

- its parameter declaration type is the same as the type of the thrown error object; OR
- the catch handler matches any type (catch-all).

The error handling stack consists of the `do` bodies of the enclosing `do-catch` statements in the
current function, followed by those of the enclosing `do-catch` statements at each call site down the call
stack.

Implicit conversion is not performed in `throw`/`catch` error handling. The type of the thrown object must
exactly match the parameter declaration type of a catch handler. Likewise, when the error object is not caught
within the enclosing function, the type of the thrown object must exactly match the error type declared in
the `throws` clause of that function.

A `throw` statement must not escape an enclosing deferred statement. That is, if a `throw` statement appears
within a [deferred statement](statements-defer.md), the matching catch handler must be within that same
deferred statement. See [`defer` statement](statements-defer.md) for the execution of deferred statements when
an error object is thrown and caught.

## Examples

Error handling basics:

```hlsl
// Error object
struct DivisionByZero
{
    uint dividend;
}

// Throws an error object on division by zero
uint checkedDivide(uint dividend, uint divisor)
    throws DivisionByZero
{
    if (divisor != 0U)
        return dividend / divisor;

    // Since this statement does not have an enclosing
    // catch handler, the throw statement causes the
    // function to throw an error object
    throw DivisionByZero(dividend);
}

struct Input
{
    uint dividend;
    uint divisor;
}

struct Output
{
    uint result; // division result
    uint error;  // 0 on success; 1 on error
}

StructuredBuffer<Input> inputBuffer;
RWStructuredBuffer<Output> outputBuffer;

[numthreads(16,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;
    Input input = inputBuffer[index];
    Output output = { };

    // error handling block
    do
    {
        // checkedDivide() may throw, so it must be called
        // using a try expression. In case checkedDivide()
        // throws, control is transferred to the catch
        // handler before the assignment is performed.
        output.result =
            try checkedDivide(input.dividend, input.divisor);
    }
    catch (err : DivisionByZero)
    {
        // division by zero is caught, signal error
        output.result = err.dividend;
        output.error = 1U;
    }

    // store output
    outputBuffer[index] = output;
}
```

**TODO:** Add more examples after GitHub issues #12343, #12361, and #12362 have been addressed.
