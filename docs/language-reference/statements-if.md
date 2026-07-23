# if-statement

## Syntax

Basic form:

> **`'if'`** **`'('`** *`cond-expr`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`then-stmt`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[ **`'else'`** *`else-stmt`* ]<br>

Binding form:

> **`'if'`** **`'('`** **`'let'`** *`identifier`*  **`'='`** *`cond-expr`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`then-stmt`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[ **`'else'`** *`else-stmt`* ]<br>

## Description

An if-statement defines a conditional branch in the program. If *`cond-expr`* evaluates to `true`, *`then-stmt`*
is executed. Otherwise, *`else-stmt`* is executed if one is provided. After *`then-stmt`*/*`else-stmt`* has
been executed, the execution of the if-statement completes.

In the basic form, if *`cond-expr`* is not a scalar [Boolean](types-fundamental.md) expression, it is
implicitly [converted](expressions-conversions.md) to a Boolean.

The *binding form* tests the condition and conditionally binds to the provided identifier as follows:

- If *`cond-expr`* has type `Optional<T>` and the optional contains a value, the optional value is bound to
  *`identifier`* during the execution of the *`then-stmt`*. If the optional does not contain a value,
  *`else-stmt`* is executed (if provided) without the *`identifier`* binding. See
  [optional types](types-optional.md) for details.
- If *`cond-expr`* is a dynamic cast expression `x as Y` and `x` is of type `Y`, `x` is bound to
  *`identifier`* as type `Y` during the execution of the *`then-stmt`*. If the dynamic cast
  fails, *`else-stmt`* is executed (if provided) without the *`identifier`* binding. See
  [dynamic casting](expressions-dynamic-cast.md) for details.

The optional **`'else'`** *`else-stmt`* part of the syntax binds greedily. It is recommended to put nested
if-statements with else-branches in [block statements](statements-block.md) to avoid potential confusion.

> 📝 **Remark 1:** In case *`then-stmt`* or *`else-stmt`* is an [empty statement](statements-empty.md), a
> warning is diagnosed. This is almost always an unintended typo. In case a true empty statement is intended,
> an empty [block statement](statements-block.md) can be used.

> 📝 **Remark 2:** When some threads in the wave or the thread-group take the then-branch and others the
> else-branch, execution diverges. See
> [execution divergence and reconvergence](basics-execution-divergence-reconvergence.md) for details.


## Examples

Basic form:

```hlsl
if (a > b)
    ++i; // executed if a is greater than b
else
    --i; // executed if a is not greater than b
```

Basic form, multiple statements in the *then* branch:

```hlsl
if (a > b)
{
    // if a is greater than b, both are executed
    ++i;
    ++j;
}
```

Nested if-statements with *else* branches:

```hlsl
// a construction such as this can be confusing
if (a > b)
    if (c > d)
        i += 1;
else if (e > f) // probable bug: 'else' binds to "if (c > d)" above
        i += 2;
    else
        i += 3;

// Block statements make nested ifs much easier to read. Likely
// the below was intended
if (a > b)
{
    if (c > d)
        i += 1;
}
else
{
    if (e > f)
        i += 2;
    else
        i += 3;
}
```

Binding if-statement with optional:

```hlsl
int returnValueOrMinus1(Optional<int> optV)
{
    if (let v = optV)
        return v; // the contained value is bound to v
    else
        return -1;
}
```

Binding if-statement with dynamic cast:

```hlsl
interface MyIFace
{
}

struct MyStruct1 : MyIFace
{
    int a;
}

struct MyStruct2 : MyIFace
{
    int b;
}

int returnAOrBx2(MyIFace ifaceObj)
{
    if (let s1 = ifaceObj as MyStruct1)
    {
        return s1.a;
    }
    else if (let s2 = ifaceObj as MyStruct2)
    {
        return s2.b * 2;
    }
    else
    {
        // neither
        return -1;
    }
}

MyIFace createIFaceObject(int x)
{
    if ((x % 2) == 0)
        return MyStruct1(x);
    else
        return MyStruct2(x);
}


StructuredBuffer<int> input;
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    MyIFace ifaceObj = createIFaceObject(input[tid.x]);

    output[tid.x] = returnAOrBx2(ifaceObj);
}
```
