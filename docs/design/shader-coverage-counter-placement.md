Shader Coverage Counter Placement
=================================

This document describes where Slang inserts coverage counters for the
current shader coverage modes. It is about instrumentation placement,
not host binding, metadata querying, or report rendering. For the
overall implementation architecture, see
[`shader-coverage.md`](shader-coverage.md). For the host-facing
metadata and binding contract, see
[`shader-coverage-host-interface.md`](shader-coverage-host-interface.md).

The examples below use a conceptual helper:

```slang
coverageAtomic("name"); // AtomicAdd(__slang_coverage[slot], 1)
```

The real compiler first emits marker IR ops during AST lowering. The
IR coverage pass later assigns numeric counter slots, synthesizes the
hidden `__slang_coverage` buffer, and rewrites each marker to an
atomic increment. Slot numbers are not part of the placement contract;
the metadata maps each source coverage entry to the slot chosen for
that compile.

Modes are independent. Enabling more than one mode adds the markers
from each enabled mode into the same counter buffer.

Line Coverage: `-trace-coverage`
--------------------------------

Line coverage inserts a counter before each executable statement that
has a valid source location. Purely structural statement wrappers,
such as blocks, statement sequences, and empty statements, are skipped.

This is statement coverage, not basic-block coverage. Multiple
statements on the same source line can get multiple counters, and the
LCOV conversion step aggregates those counters back to the source
line.

Conceptually, this source:

```slang
void someFunction(uint N)
{
    uint i = 0;
    while (i < N)
    {
        x = y + z;
        a = b + c;
        d = e + f;
        i++;
    }
}
```

is instrumented like this under line coverage:

```slang
void someFunction(uint N)
{
    coverageAtomic("line: uint i = 0");
    uint i = 0;

    coverageAtomic("line: while");
    while (i < N)
    {
        coverageAtomic("line: x = y + z");
        x = y + z;

        coverageAtomic("line: a = b + c");
        a = b + c;

        coverageAtomic("line: d = e + f");
        d = e + f;

        coverageAtomic("line: i++");
        i++;
    }
}
```

The marker on the `while` statement counts execution reaching the loop
statement. It does not count each loop-condition evaluation. Per-arm
loop condition counts come from branch coverage.

Function Coverage: `-trace-function-coverage`
---------------------------------------------

Function coverage inserts one counter at the entry of each
user-authored function body that is lowered to executable IR. It
counts function entry, not every basic block in the function.

Conceptually:

```slang
uint helper(uint x)
{
    return x + 1;
}

void someFunction()
{
    uint y = helper(41);
}
```

is instrumented like this:

```slang
uint helper(uint x)
{
    coverageAtomic("function: helper");
    return x + 1;
}

void someFunction()
{
    coverageAtomic("function: someFunction");
    uint y = helper(41);
}
```

The coverage metadata records both display and mangled function names
when available. Reports can aggregate multiple runtime counters that
attribute to the same source function, but hosts must use the metadata
rather than assuming counter-slot identity across compiles or shader
permutations.

Branch Coverage: `-trace-branch-coverage`
-----------------------------------------

Branch coverage inserts counters at selected control-flow arm entry
points. It answers "which branch outcome was selected?" It does not
insert counters before every statement inside the selected arm.

### If / Else

For an `if` with an `else`, Slang emits one counter for the true arm
and one for the false arm:

```slang
if (p)
{
    x = y + z;
}
else
{
    a = b + c;
}
```

Conceptually:

```slang
if (p)
{
    coverageAtomic("branch: if true");
    x = y + z;
}
else
{
    coverageAtomic("branch: if false");
    a = b + c;
}
```

For an `if` without an `else`, Slang still emits a false-arm counter
in an otherwise-empty false path:

```slang
if (p)
{
    coverageAtomic("branch: if true");
    x = y + z;
}
else
{
    coverageAtomic("branch: if false");
}
```

This lets reports distinguish "the `if` was reached and the condition
was false" from "the `if` was never reached."

### While and For Loops

For `while` and `for` loops, Slang emits counters for the loop
condition's true and false outcomes:

```slang
while (i < N)
{
    x = y + z;
    a = b + c;
    d = e + f;
    i++;
}
```

Conceptually:

```slang
while (true)
{
    if (i < N)
    {
        coverageAtomic("branch: while condition true");

        x = y + z;
        a = b + c;
        d = e + f;
        i++;
    }
    else
    {
        coverageAtomic("branch: while condition false");
        break;
    }
}
```

For a loop that runs `N` times, the true-arm counter increments `N`
times and the false-arm counter increments once for the normal exit.
Statements inside the loop body do not receive branch counters unless
they contain their own branch constructs.

### Do While Loops

For `do while`, the body executes before the condition is tested. The
branch counters are attached to the condition result after the body:

```slang
do
{
    x = y + z;
    i++;
} while (i < N);
```

Conceptually:

```slang
do
{
    x = y + z;
    i++;

    if (i < N)
    {
        coverageAtomic("branch: do-while condition true");
        continue;
    }
    else
    {
        coverageAtomic("branch: do-while condition false");
        break;
    }
} while (true);
```

For a `do while` loop that runs `N` iterations, the true-arm counter
increments `N - 1` times when the loop continues, and the false-arm
counter increments once on exit.

### Switch

For `switch`, counters are inserted on dispatch arms, not in the case
body after fallthrough. This preserves the meaning "which switch label
was selected by dispatch?"

For example:

```slang
switch (v)
{
case 0:
case 1:
    x = 1;
    break;
case 2:
    x = 2;
    // fall through
default:
    x += 10;
    break;
}
```

is conceptually lowered as:

```slang
switch (v)
{
case 0:
    coverageAtomic("branch: switch case 0");
    goto body_case_0_or_1;

case 1:
    coverageAtomic("branch: switch case 1");
    goto body_case_0_or_1;

case 2:
    coverageAtomic("branch: switch case 2");
    goto body_case_2;

default:
    coverageAtomic("branch: switch default");
    goto body_default;
}

body_case_0_or_1:
    x = 1;
    break;

body_case_2:
    x = 2;
    // fall through into default body, but the default arm counter does
    // not increment because default was not selected by dispatch.

body_default:
    x += 10;
    break;
```

If a switch has no `default`, branch coverage creates a synthetic
no-match default arm so the report can distinguish "no case matched"
from "the switch was not reached."

Combined Function and Branch Coverage
-------------------------------------

With function and branch coverage enabled, but line coverage disabled,
the resulting instrumentation is closer to control-flow outcome
coverage than statement coverage. Function entry and branch-arm
entries receive counters; straight-line statements inside a selected
arm do not.

For example:

```slang
void someFunction(uint N)
{
    uint i = 0;
    while (i < N)
    {
        x = y + z;
        a = b + c;
        d = e + f;
        i++;
    }
}
```

is conceptually:

```slang
void someFunction(uint N)
{
    coverageAtomic("function: someFunction");

    uint i = 0;
    while (true)
    {
        if (i < N)
        {
            coverageAtomic("branch: while condition true");

            x = y + z;
            a = b + c;
            d = e + f;
            i++;
        }
        else
        {
            coverageAtomic("branch: while condition false");
            break;
        }
    }
}
```

This is the mode to use when the goal is to reduce probe density while
still answering whether functions and control-flow outcomes were
exercised.

Interactions with Optimization and Variants
-------------------------------------------

Coverage marker ops are emitted before most IR optimization and are
rewritten to atomics after linking. Normal compiler transformations
can clone, inline, specialize, or remove code before the final
coverage pass sees it. The pass assigns slots only to surviving marker
ops in the final linked-program IR.

Preprocessor variants and specialization-heavy builds are separate
compiles. Each compile gets its own counter buffer layout and metadata
mapping. Hosts and report converters should aggregate by the source
attribution fields in `CoverageEntryInfo` or `.coverage-mapping.json`,
not by assuming that counter slot `K` means the same source location
in two different compiles.

Future Region Coverage
----------------------

The current modes all use one direct runtime counter per emitted
source entry. Future source-region coverage may move toward a
clang-style model where entries describe source ranges and some
reported counts are derived from other counters. That should extend
coverage metadata, but it should not change the basic rule that hidden
resource binding is separate from source attribution.
