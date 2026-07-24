# Loop statements

## Syntax

while loop:

> **`'while'`** **`'('`** *`cond-expr`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`loop-stmt`*

do-while loop:

> **`'do'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`loop-stmt`*<br>
> **`'while'`** **`'('`** *`cond-expr`* **`')'`** **`';'`**

for loop:

> **`'for'`** **`'('`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*`init-stmt`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[*`cond-expr`*] **`';'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[*`post-loop-expr`*] **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`loop-stmt`*

## Description

The loop statements execute *`loop-stmt`* while the loop condition *`cond-expr`* evaluates to `true`.

A _while loop_ evaluates the loop condition *`cond-expr`*. If the loop condition evaluates to `false`, the
while loop terminates and the while-statement completes. Otherwise, the loop statement *`loop-stmt`* is
executed, after which the loop condition is evaluated again to determine whether the looping continues.

In sequential form:

1. Evaluate the loop condition *`cond-expr`*.
2. If the loop condition is `false`, go to step 5.
3. Execute *`loop-stmt`*.
4. Go to step 1.
5. End of while loop.

A _do-while loop_ begins by executing the *`loop-stmt`*. Then the loop condition *`cond-expr`* is evaluated to
determine whether the looping continues.

In sequential form:

1. Execute *`loop-stmt`*.
2. Evaluate the loop condition *`cond-expr`*.
3. If the loop condition is `true`, go to step 1.
4. End of do-while loop.

A _for loop_ is a variation of the while loop. It begins by executing the initialization statement
*`init-stmt`*. Then the loop condition *`cond-expr`* is evaluated. If the loop condition evaluates to `false`,
the for loop terminates and the for-statement completes. Otherwise, the loop statement *`loop-stmt`* is
executed, after which the *`post-loop-expr`* is evaluated. Then the loop condition is evaluated again to determine whether the looping continues.

In sequential form:

1. Execute *`init-stmt`*.
2. Evaluate the loop condition *`cond-expr`*.
3. If the loop condition is `false`, go to step 7.
4. Execute *`loop-stmt`*.
5. Evaluate *`post-loop-expr`*.
6. Go to step 2.
7. End of for loop.

In a for loop, the *`init-stmt`* may be an [empty statement](statements-empty.md), i.e., just the semicolon
(**`';'`**). The loop condition *`cond-expr`* may be omitted, in which case it is substituted with `true`. The
post-loop expression *`post-loop-expr`* may also be omitted, in which case the post-loop expression is not
evaluated.

The loop condition may not be omitted in while and do-while statements.

If *`cond-expr`* is provided and it is not a scalar [Boolean](types-fundamental.md) expression, it is
implicitly [converted](expressions-conversions.md) to a Boolean.

A loop can be terminated immediately with a [break statement](statements-break-and-continue.md).

A loop iteration can be terminated with a [continue statement](statements-break-and-continue.md). In
for loops, a continue statement jumps to *`post-loop-expr`* evaluation after which the loop condition
*`cond-expr`* is evaluated to determine whether the looping continues. In while loops and do-while loops, a
continue statement jumps to evaluating the loop condition.


## Examples

Simple for loop:

```hlsl
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain()
{
    for (uint i = 0; i < 4; ++i)
        output[i] = i;

    // stores:
    //
    // output[0] = 0;
    // output[1] = 1;
    // output[2] = 2;
    // output[3] = 3;
}
```

Simple while loop:

```hlsl
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain()
{
    uint i = 0;
    while (i < 4)
    {
        output[i] = i;
        ++i;
    }

    // stores:
    //
    // output[0] = 0;
    // output[1] = 1;
    // output[2] = 2;
    // output[3] = 3;
}
```

A for loop with a break statement:

```hlsl
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain()
{
    uint i = 0;

    // note: 'for (;;)' is an infinite loop without a
    // break statement
    for (;;)
    {
        if (i >= 4)
            break;

        output[i] = i;
        ++i;
    }

    // stores:
    //
    // output[0] = 0;
    // output[1] = 1;
    // output[2] = 2;
    // output[3] = 3;
}
```

A do-while loop:

```hlsl
RWStructuredBuffer<uint> output;

// string copy with null termination
void stringCopyToOutput(in uint8_t[256] src)
{
    uint8_t ch;
    uint i = 0;

    do
    {
        // read a byte from src or 0 if src is exhausted
        ch = i < 256 ? src[i] : 0U;

        // store the byte
        output[i] = ch;

        // next byte
        ++i;
    }
    while (ch != 0);
}

[numthreads(1,1,1)]
void computeMain()
{
    uint8_t src[256] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\0' };

    stringCopyToOutput(src);

    // stores:
    //
    // output[0] = 'H'
    // output[1] = 'e'
    // output[2] = 'l'
    // output[3] = 'l'
    // output[4] = 'o'
    // output[5] = ','
    // output[6] = ' '
    // output[7] = 'w'
    // output[8] = 'o'
    // output[9] = 'r'
    // output[10] = 'l'
    // output[11] = 'd'
    // output[12] = '!'
    // output[13] = '\0'
}
```

A for loop with a continue statement:

```hlsl
StructuredBuffer<uint> input;
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain()
{
    uint start = input[0];
    uint end = input[1];

    uint sum = 0U;

    // compute a sum of even integers in [start, end)
    for (uint i = start; i < end; ++i)
    {
        if ((i % 2) != 0U)
            continue;

        sum += i;
    }

    output[0] = sum;
}
```
