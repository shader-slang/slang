# Switch Statement

## Syntax

`switch` statement:

> [*`stmt-label-decl`*]<br>
> **`'switch'`** **`'('`** *`branch-expr`* **`')'`** *`block-stmt`*

Within the [block statement](statements-block.md) of the `switch` statement, two additional statements are
available:

> **`'case'`** *`label-expr`* **`':'`**
>
> **`'default'`** **`':'`**

## Description

A `switch` statement defines a conditional branch in the program. After the branch expression
(*`branch-expr`*) is evaluated, control is transferred to the `case` statement whose label expression
(*`label-expr`*) matches the value of the branch expression. If no case label expression matches, control is
transferred to the `default` statement, if one is defined. If no `default` statement is defined, the
`switch` statement completes without executing any of its cases.

The branch expression type must be a [Boolean](types-fundamental.md), an [integer](types-fundamental.md)
type, or an [enumeration](types-enum.md).

Each case label expression must be a [compile-time constant](expressions-evaluation-classes.md). The label
expressions are implicitly converted to the type of the branch expression. After the implicit conversion, the
case label expression values must be unique.

There may be at most one `default` statement per `switch` statement.

A `case` or `default` statement may appear only directly in the switch body, that is, the
[block statement](statements-block.md) of the `switch` statement. A `case` or `default` statement may not be
nested in any statement within that block, such as a nested [block statement](statements-block.md) or an
[`if` statement](statements-if.md). A `case` or `default` statement within a nested `switch` statement
belongs to that statement.

A `switch` statement is a [breakable statement](statements-break-and-continue.md), and it can be prefixed by
a statement label declaration. A [`break` statement](statements-break-and-continue.md) can be used to exit
the `switch` statement. A `break` statement without a statement label within a nested loop or `switch`
statement exits that nested statement. A statement label can be used to exit the `switch` statement from
within.

A [`continue` statement](statements-break-and-continue.md) is allowed if the `switch` statement is nested in a
[loop statement](statements-loop.md).

If a `case` or `default` statement is reachable from a previous `case` or `default` statement, the control
flow is said to _fall through_ between them. When there are no statements between the two, the fall-through is
_trivial_. While Slang allows fall-throughs, some targets do not have native support for non-trivial
fall-throughs. See [target compatibility](../target-compatibility.md) for details.

If different threads within a wave or thread group branch to different cases, divergence occurs. See
[execution divergence and reconvergence](basics-execution-divergence-reconvergence.md) for implications.

> 📝 **Remark 1:** Unlike C and C++, `case` and `default` statements may not be interleaved with other control
> flow. Constructions such as [Duff's device](https://en.wikipedia.org/wiki/Duff%27s_device) are not allowed.

> 📝 **Remark 2:** `slangc` attempts to detect unreachable statements within a switch body. Unreachable
> statements include any statement before the first `case` or `default` statement and any statement between a
> `break` statement and a follow-up `case` or `default` statement.

## Examples

Basic operation:

```hlsl
StructuredBuffer<int> input;
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int val = input[tid.x];

    switch (val)
    {
    case 0: // jump here if val is 0
        output[tid.x] = 1;
        break; // jump out of the switch statement

    case 3: // jump here if val is 3
        output[tid.x] = 3;
        if (tid.y == 0)
        {
            // a break is allowed in an if-statement
            break; // jump out of the switch statement
        }

        output[tid.x]++;
        break; // jump out of the switch statement

    default: // jump here if val is anything other than 0 or 3
        output[tid.x] = -1;

        // Note: the last 'break' is optional
        break; // jump out of the switch statement
    }
}
```

Fall-throughs:

```hlsl
StructuredBuffer<int> input;
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    switch (input[tid.x])
    {
    case 0:
        // trivial fall-through to case 1
    case 1:
        // trivial fall-through to case 2
    case 2:
        output[tid.x] = 1;
        break;

    case 3:
        output[tid.x] = 3;

        // non-trivial fall-through to case 4
    case 4:
        output[tid.x]++;
        // exit the switch statement (not a fall-through)
    }
}
```
