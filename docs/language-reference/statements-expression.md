# Expression Statement

## Syntax

Expression statement:

> *`expr`* **`';'`**

## Description

An expression statement evaluates an [expression](expressions.md). The resulting value of the expression is
discarded.

For an expression statement to be meaningful, the expression must have side effects. Expressions with side
effects include [assignment](expressions-operators.md), [increment](expressions-operators.md), and
[function calls](expressions-operators.md). See [expressions](expressions.md) for details on side effects.

## Examples

Expression statements:

```hlsl
RWStructuredBuffer<uint> output;

void storeToOutput(uint ix, uint val)
{
    // Expression statement with an assignment expression.
    // This expression stores 'val' at index 'ix' of the
    // output buffer.
    output[ix] = val;
}

[numthreads(8,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint v;

    // Expression statement with an assignment expression.
    // Value of expression 'tid.x * 2U' is assigned to
    // variable 'v'.
    v = tid.x * 2U;

    // Expression statement with a function call expression.
    // Argument expressions 'tid.x' and 'v' are evaluated,
    // after which function 'storeToOutput' is called. The
    // values of the argument expressions are passed as the
    // call arguments.
    storeToOutput(tid.x, v);

    // Expression statement without side effects. The resulting
    // value is discarded. A statement without side effects may
    // be eliminated by the optimizer, and an implementation may
    // warn about it.
    tid.y + tid.z;
}
```
