# Shader Coverage Counter Placement

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
hidden `__slang_coverage` buffer, and rewrites markers to an atomic
increment — or, under `-trace-coverage-boolean`, to a plain
non-atomic store of `1` (hit/not-hit). Placement is identical across
the two rewrite forms; everything in this document applies to both.
Slot numbers are not part of the placement contract; the metadata maps
each source coverage entry to the slot chosen for that compile.

Markers and counters are not one-to-one. Coverage metadata is authoritative:
read every entry through its `counterIndex`, and deduplicate aliases for each
source identity. Slots are local to one compiled artifact.

## Line Coverage: `-trace-coverage`

Line coverage reports visits to a source line, rather than adding the number
of statements on that line. For example, four calls to either function below
report four executions of its body line:

```slang
int sequential(int x) { int y = x; y += 2; return y; }
int conditional(int x) { if (x > 0) return 1; else return 2; }
```

Lowering emits markers before executable statements and at evaluated scalar
conditions and conditional-expression arms. The coverage pass groups markers
by function, source file, and line. Blocks containing that line form a region,
and the region is counted through one block so that `conditional` above reports
one visit per call, not the condition plus the selected return. If the region
contains a loop header, that header is the counted block: it runs once per
iteration and once more on exit, so a `for` header reports its condition
evaluations even though its initializer, test, and increment lie in different
blocks. Otherwise the counted block is the one that dominates the rest of the
region, and a probe dominated by another probe of the same line is omitted.
Both rules need only one dominator tree per function and no runtime state, in
count and boolean mode alike. A loop confined to a single source line is the
exception: it counts executions of its counted block, which is the loop header
only when the loop has a test.

Each function/file/line has one canonical metadata entry. Distinct source
functions on the same line contribute separate counts. Lines confined to a
single block can still share a runtime slot when the existing fallthrough
analysis proves they execute together. Calls that may abandon the invocation
split these coalescing groups. Consumers must not sum repeated aliases of
one slot on the same line; boolean values combine with logical OR.

As before, the fallthrough analysis cannot recognize target-level termination
hidden inside a normally returning intrinsic's `GenericAsm`. Ray-tracing hit
terminators are an example of this existing limitation.

## gcov and LCOV compatibility

The semantic contract is evaluated scalar decisions, source-line visits, and
function entries. It does not promise GCC's block numbering, branch order,
optimization-dependent source mapping, or exception edges. GCC 15 can attribute
shared ternary control-flow blocks to both arm lines, even at `-O0`; Slang
instead records the actual evaluation of each arm.

Branch coverage includes line events for its evaluated expressions, so LCOV
has a `DA` record at each decision location. Function-only exports include the
function declaration lines. `BRDA` uses `-` when no outcome of a decision ran,
and `0` for an untaken arm of an evaluated decision. Both consumers must use
the same line/function/branch identities so totals agree. `genhtml` should run
without `--ignore-errors`.

Older manifests can contain only function or branch records at a location.
The converter supplies their known execution evidence as line records. For
multiple legacy decisions on one line, the largest site count is only a lower
bound: exact line counts require recompilation with canonical line events.
The converter cannot reconstruct short-circuit operand counts from old merged
result counters.

The executable reference gate is:

```sh
python3 tools/shader-coverage/test_gcov_semantics.py \
    --slangc build/Debug/bin/slangc --gcc g++-15 --gcov gcov-15 \
    --output-dir /tmp/slang-gcov-parity
```

It runs identical input data through GNU GCC/gcov and Slang's C++ target,
compares outputs and the promised coverage semantics, and imports every result
with strict `genhtml`. It covers count/boolean recording, both counter widths,
and each coverage mode independently. GCC and gcov must be matching versions;
Apple's `/usr/bin/gcov` is an LLVM compatibility implementation.

The normal `coverageCpuRuntimeLineRegions` and
`coverageCpuRuntimeExpressionBranches` unit tests also exercise these semantics
without requiring GCC, gcov, or genhtml. They cover both counter widths and modes,
one-line loops, nested loops, early exits, skipped operands, and nested expressions.
`coverage-line-region-probes.slang` checks that a multi-block line issues one
probe and compiles the region control flow to SPIR-V.

## Function Coverage: `-trace-function-coverage`

Function coverage inserts one counter at the entry of each
user-authored function body that is lowered to executable IR. It
counts function entry, not every basic block in the function.
This includes entry points, free functions, user-authored
constructors, instance/static methods, `[ForceInline]` functions, and
lambda bodies as they are represented by lowering. Compiler-synthesized
helpers that do not carry a user source location are not part of the
function-coverage contract.

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
when available. Generic specializations share one source-level
function entry: specializing `helper<T>` for three different `T`s
still produces a single `helper` entry in the metadata, so reports
stay source-oriented instead of fanning out per specialization.
Reports can aggregate multiple runtime counters that
attribute to the same source function, but hosts must use the metadata
rather than assuming counter-slot identity across compiles or shader
permutations. Lambda and constructor entries can have compiler-facing
display names such as `$init` or `()`; consumers should treat the
mangled name and source location as the stable identity for those
entries.

## Branch Coverage: `-trace-branch-coverage`

Branch coverage inserts counters at selected control-flow arm entry
points. It answers "which branch outcome was selected?" It does not
insert counters before every statement inside the selected arm.

The current scope is: `if`/`else`, `for`/`while`/`do while`
loop-condition outcomes, `switch` case/default dispatch arms, and the
expression-level branches of a scalar `?:` and a short-circuiting `&&`
/ `||`. `return`, `break`, and `continue` are represented through the
branch arm that reaches them rather than as separate branch entries.

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
was selected by dispatch?" All outcomes are attributed to the switch
condition location, including its implicit no-match default.

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

### Ternary `?:`

A `?:` with a scalar condition evaluates only the operand its condition
selects, so it lowers to the same two-way branch as an `if` with an
`else`. Slang records its evaluated condition, attributed to the condition
expression, and line events for the selected value expression:

```slang
uint v = (t > 1u) ? a : b;
```

Conceptually:

```slang
uint v;
if (t > 1u)
{
    coverageAtomic("branch: ?: true");
    v = a;
}
else
{
    coverageAtomic("branch: ?: false");
    v = b;
}
```

A `?:` with a vector condition selects per element and evaluates both
operands (this form is deprecated in favor of `select`), so it does not
branch and gets no counters.

### Short-Circuit `&&` and `||`

`&&` and `||` evaluate their right operand only when the left operand
does not already decide the result. Each evaluated scalar operand has a true
and false outcome, attributed to that operand's source expression.

```slang
if (a && b)
```

This records `a`, then records `b` only when `a` is true. If `a` is false,
both counters for `b` remain zero; LCOV renders those arms as unevaluated.
There is no additional site for the merged result of `a && b`. Parentheses
and logical negation preserve the operand decision tree.

Likewise, `(a && b) || c` has decisions for `a`, `b`, and `c`, with skipped
operands remaining unevaluated. The same source-oriented rule applies when
the expression produces a value, such as `bool r = a && b;`. GCC can eliminate
some branches in value contexts; exact backend branch counts are outside the
compatibility contract.

Under `-disable-short-circuit`, `&&` and `||` evaluate both operands
without branching and get no counters. `?:` is unaffected by that
option.

Expressions that are lowered outside a function body, such as the
initializer of a global (`static bool g = a && b;`), are not
instrumented.

## Combined Function and Branch Coverage

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

## Interactions with Optimization and Variants

Coverage marker ops are emitted before most IR optimization and are
rewritten to atomics after linking. Normal compiler transformations
can clone, inline, specialize, or remove code before the final
coverage pass sees it. The pass assigns slots only to surviving marker
ops in the final linked-program IR.

Preprocessor variants and specialization-heavy builds are separate
compiles. Each compile gets its own counter buffer layout and metadata
mapping. Hosts and report converters should aggregate by the source
attribution fields in `CoverageEntryInfo` or `.coverage-manifest.json`,
not by assuming that counter slot `K` means the same source location
in two different compiles.

Automatic differentiation runs after the coverage pass, so the forward
and backward derivatives generated from a differentiable function carry
that function's counter increments as non-differentiable side effects.
They follow autodiff's rule for side effects: how many times a side
effect runs inside a derivative is not guaranteed. A backward derivative
can skip primal code whose results it does not need, counters included.
Counts gathered while derivatives run therefore do not measure how often
the differentiable function's source executed.

## Future Region Coverage

Line coverage already shares one counter across the source entries of
a straight-line region (see [Counter coalescing](#counter-coalescing)),
but each entry still names a concrete counter and reports a real
count. Future source-region coverage may move further toward a
clang-style model where entries describe source _ranges_ and some
reported counts are derived arithmetically from other counters rather
than read directly. That would extend coverage metadata — most likely
by using `kInvalidCoverageCounterIndex` for derived entries — but it
should not change the basic rule that hidden resource binding is
separate from source attribution.
