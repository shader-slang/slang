# Declaration Statement

## Syntax

Declaration statement:

> *`decl-stmt`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`var-decl`* |<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`struct-decl`* |<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`typealias-decl`*

## Description

A declaration statement adds a [declaration](declarations.md) to the current scope. If the declaration is a
variable declaration with initialization (implicit or explicit), initialization is performed.

A declaration statement may be a [variable](declarations-variable.md), [structure](types-struct.md), or
[type alias](types.md#alias) declaration. Other declarations are not allowed.

## Examples

Declaration statements:

```hlsl
// Global variable declaration (not a statement)
RWStructuredBuffer<uint> output;

// Function declaration (not a statement)
uint sum(uint a, uint b)
{
    // Variable declaration statement with
    // initialization. Variable 'ret' is initialized
    // with value of expression 'a + b'.
    uint ret = a + b;

    return ret;
}

[numthreads(8,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Structure declaration statement
    struct LocalStruct
    {
        uint field;

        // no-parameter constructor
        __init()
        {
            field = 10;
        }
    }

    // Variable declaration statement. The declaration
    // does not have explicit initialization. However,
    // since LocalStruct has a no-parameter constructor,
    // the constructor is invoked.
    LocalStruct s;

    // Note: 's.field' is now '10'

    s.field += sum(tid.x, 5);

    output[tid.x] = s.field;
}
```
