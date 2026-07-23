# Empty Statement

## Syntax

Empty statement:

> **`';'`**

## Description

An empty statement does nothing.

> 📝 **Remark 1:** Empty statements should generally be avoided. Use an empty block (`{}`) instead, where
> applicable.

> 📝 **Remark 2:** `slangc` currently warns when an empty statement is used as the then- or else-statement in
> an [`if` statement](statements-if.md). This is a common source of bugs. A future Slang language version may
> make this an error and extend the restriction to empty statements used as the body of a
> [for](statements-loop.md), [while](statements-loop.md), or [defer](statements-defer.md) statement, or as a
> [catch](statements-do-catch.md) body. See GitHub issue
> [#12296](https://github.com/shader-slang/slang/issues/12296) for details.

## Examples

Empty statement:

```hlsl
[numthreads(1,1,1)]
void computeMain()
{
    ; // does nothing
}
```

Prefer empty [block statements](statements-block.md) over empty statements:

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

RWStructuredBuffer<uint> output;

[numthreads(16,16,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    do
    {
        // If division fails, it is ok to skip the write.
        output[tid.x + 16 * tid.y] =
            try checkedDivide(tid.x, tid.y);
    }
    catch (ex : DivisionByZero)
    {
        // Ignore this exception silently with an empty block.
        //
        // In Slang 2026, this can also be written as
        //
        //    catch (ex : DivisionByZero) ;
        //
        // However, an empty statement in a catch body may
        // become an error in a future Slang version.
    }
}
```
