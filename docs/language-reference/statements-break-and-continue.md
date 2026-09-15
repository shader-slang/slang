# Break and Continue Statements

## Syntax

`break` statement:

> **`'break'`** [*`stmt-label`*] **`';'`**

Statement label declaration:

> *`stmt-label-decl`* = *`stmt-label`* **`':'`**

Statement label:

> *`stmt-label`* = *`identifier`*

`continue` statement:

> **`'continue'`** **`';'`**

## Description

A `break` statement transfers control out of the innermost enclosing _breakable statement_, that is, the
enclosing [loop](statements-loop.md) or [switch](statements-switch.md) statement. If a statement label
(*`stmt-label`*) is provided, control is transferred out of the enclosing breakable statement whose label
declaration (*`stmt-label-decl`*) matches, which need not be the innermost breakable statement.

A `continue` statement skips the rest of the current loop iteration. In `while` and `do-while` loops, a
`continue` statement jumps to the loop condition evaluation. In `for` loops, a `continue` statement jumps to
the post-loop expression evaluation, after which the loop condition is evaluated. In both cases, if the loop
condition evaluates to `true`, looping continues. A `continue` statement cannot target an outer loop, as it
does not accept a statement label.

Any breakable statement can be prefixed by a statement label declaration (*`stmt-label-decl`*).

It is an error to have a `break` statement not enclosed by a breakable statement. Similarly, it is an error to
have a `continue` statement not enclosed by a loop statement. It is also an error if the statement label given
in a `break` statement does not match any enclosing breakable statement.

A `break` or `continue` does not cancel [deferred statements](statements-defer.md). The deferred statements
are executed when their respective scopes are exited.

A `break` or `continue` may not escape an enclosing deferred statement.

> 📝 **Remark 1:** A `continue` statement within a [switch](statements-switch.md) statement continues the
> enclosing loop.

## Examples

Multi-level break:

```hlsl
StructuredBuffer<uint> inputCommands;
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
  outer:
    for (uint i = 0; ; ++i)
    {
        switch (inputCommands[i])
        {
        case 0:
            // end of command stream --> exit the for loop
            break outer;

        case 1:
            // increment output
            output[i]++;
            break; // exit the switch statement

        case 2:
            // decrement output
            output[i]--;
            break; // exit the switch statement

        default:
            // unknown command, do nothing
            break; // exit the switch statement
        }
    }
}
```

Multi-level break with deferred statements:

```hlsl
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
  outer:
    for (int i = 0; i < 2; ++i)
    {
        // write 67 to output[0] when the block
        // statement is exited
        defer output[0] = 67;

        // write 42 to output[1] when the block
        // statement is exited
        defer output[1] = 42;

        for (int j = 0; j <= tid.y; ++j)
        {
            if (tid.z == 0)
                break outer;

            output[i] += j + 5;
        }

        // Note: the deferred statements are executed
        // at this point regardless of whether this block is
        // exited by a break statement or regular execution.
    }

    // Note: due to the deferred statements, output[0] is
    // always 67 and output[1] is always 42.
}
```

See [loop statements](statements-loop.md) and [`switch` statement](statements-switch.md) for more examples of
`break` and `continue`.
