# Return Statement

## Syntax

`return` statement without a return value:

> **`'return'`** **`';'`**

`return` statement with a return value expression:

> **`'return'`** *`return-expr`* **`';'`**

## Description

A `return` statement exits the enclosing [function](declarations-functions.md), possibly with a return value.

If the return value type of the enclosing function is not [void](types-fundamental.md), a return value
(*`return-expr`*) must be provided. If the type of the provided return value does not match the function
return value type, the provided return value is [implicitly converted](expressions-conversions.md) to the
function return value type. The return value expression may also be an initializer list in which case the
initializer list is passed to the appropriate constructor of the return value type.

A function with a non-void return value type must execute a `return` statement on all execution paths.
Omitting the `return` statement on a path is an error.

If the return value type of the enclosing function is [void](types-fundamental.md), the return value may be
omitted. It is also permitted to explicitly return a `void` value. Omitting the `return` statement is also
allowed, in which case the function exits when the last statement in the function body has been executed.

[Deferred statements](statements-defer.md) are executed when the enclosing function returns, after the return
value expression has been evaluated. A `return` statement must not appear inside a deferred statement.

If the enclosing function is a [lambda expression](expressions-lambda.md), the type of the return value
expression determines the return value type of the lambda expression. All `return` statements within a lambda
expression must return the same type.

## Examples

Simple function returning an integer value:

```hlsl
int sum(int a, int b)
{
    return a + b;
}
```

Function returning a value with an initializer list:

```hlsl
uint3 sumUint3(uint3 v0, uint3 v1)
{
    return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
}
```

Function with multiple `return` statements:

```hlsl
enum Sign
{
    Plus,
    None,
    Minus,
}

Sign determineSign(int v)
{
    if (v > 0)
        return Sign::Plus;

    if (v < 0)
        return Sign::Minus;

    return Sign::None;
}
```

Void values:

```hlsl
void incrementIfNonZero(inout uint v)
{
    if (v == 0)
        return;

    ++v;
}

uint multiplyBy2(uint val)
{
    return val * 2;
}

static uint s_callCount = 0U;

// count the calls and pass the parameter as is
T callCounter<T>(T val)
{
    ++s_callCount;
    return val;
}

RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint a = tid.x;

    // incrementIfNonZero() returns a void value,
    // which callCounter() passes through
    callCounter(incrementIfNonZero(a));

    // multiplyBy2() returns an unsigned integer,
    // which callCounter() passes through
    a = callCounter(multiplyBy2(a));

    output[0] = a;
    output[1] = s_callCount;
}
```

Evaluation order of the return expression and deferred statements, example 1:

```hlsl
RWStructuredBuffer<uint> output;

uint f(uint val)
{
    // Note: this function is called before the deferred
    // statement in multiplyBy2(). The value written to
    // output[0] is overwritten.
    output[0] = 2;

    return val;
}

uint multiplyBy2(uint val)
{
    // The deferred statement is executed after
    // the return expression is evaluated and
    // before the function returns. This leaves
    // the value 1 in output[0] when this function exits.
    defer output[0] = 1;

    return f(val * 2U);
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[1] = multiplyBy2(tid.x);
}
```

Evaluation order of the return expression and deferred statements, example 2:

```hlsl
RWStructuredBuffer<uint> output;

uint f()
{
    uint ret = 1;
    defer ret = 2;

    // Returns 1, since the return expression is evaluated
    // before the deferred statement assigns 2 to 'ret'.
    return ret;
}

[numthreads(1,1,1)]
void computeMain()
{
    output[0] = f();
}
```
