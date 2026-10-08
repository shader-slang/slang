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

Markers and counters are not one-to-one. Line markers that provably
execute together are coalesced onto a single counter and a single
runtime probe — see [Counter coalescing](#counter-coalescing) below.
Coverage metadata is authoritative: read every entry through its
`counterIndex`. Slots are local to one compiled artifact.

Modes are independent. Enabling more than one mode adds the markers
from each enabled mode into the same counter buffer.

## Line Coverage: `-trace-coverage`

Line coverage inserts a counter before each executable statement that
has a valid source location. Purely structural statement wrappers,
such as blocks, statement sequences, and empty statements, are skipped.
It also marks the evaluated scalar conditions of `if`, loops, `?:`,
`&&`, and `||`, and the selected arms of `?:`.

This is statement coverage, not basic-block coverage. Multiple
statements on the same source line can get multiple counters, and the
LCOV conversion step aggregates those counters back to the source line
by taking their maximum, so a line reports its visits rather than the
number of statements on it; see
[Lines with several markers](#lines-with-several-markers).

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

The `while` line also carries a marker at the evaluated loop condition,
so it reports one count per condition evaluation: `N + 1` for each call.
Per-arm loop condition counts come from branch coverage.

### Counter coalescing

Emitted shader size scales with the number of probe sequences, not with
counter width: every probe expands to index arithmetic, an address
computation, and an atomic. Line coverage therefore coalesces markers
that provably execute together onto one counter and one probe.

The rule is simple because the IR is already in basic-block form when
the coverage pass runs: markers in the same basic block, with nothing
between them that can abandon the invocation, all execute exactly the
same number of times. A basic block has one entry and one exit, so
reaching any instruction in it means reaching all of them.

The `someFunction` example above therefore emits three counters for its
markers: one for the entry block (`uint i = 0` and the `while` statement),
one for the loop header (the evaluated loop condition), and one for the
loop body, where the four body statements are one straight-line region:

```slang
void someFunction(uint N)
{
    // `uint i = 0` and the `while` statement are in the same block: one
    // probe covers both, and it sits at the last of the two.
    uint i = 0;
    coverageAtomic("region: i = 0, while");
    // The loop header block holds the evaluated condition: N + 1 times.
    while (i < N)
    {
        coverageAtomic("region: while condition");
        // The four body statements are one straight-line region.
        buf[0] = buf[1] + buf[2];
        buf[3] = buf[4] + buf[5];
        buf[6] = buf[7] + buf[8];
        i++;
        coverageAtomic("region: loop body");
    }
}
```

Because the block boundary _is_ the region boundary, every structural
split falls out for free: `if` / `else` arms, `switch` cases, loop
bodies versus loop exits, early `return` / `break` / `continue`, and
short-circuit operands all begin new blocks during lowering, so none of
them can be coalesced across.

The probe sits at the **last** marker of a region rather than the
first. Reaching it proves every earlier marker in the region also
executed; placing it first would over-report when a region is entered
but abandoned partway.

Two cases break the "same block means same count" property, and both
split the region:

- An instruction that can abandon the invocation — `discard`, an abort,
  or a call to a function that transitively contains one, or that never
  returns at all. The analysis is a memoized depth-first walk of the
  call graph, not an iterated fixpoint: a re-entered function is
  reported as possibly not returning, which breaks cycles conservatively
  without letting an optimistic answer escape into another function's
  cached result.
- A call whose target cannot be resolved statically, such as an
  interface method dispatched through a witness table. Coverage runs
  before specialization, so these are common; the pass assumes the
  worst and splits, since an extra probe costs emitted code while a
  missed split would cost correctness.

Function and branch markers always take a dedicated counter. They are
already one probe per function or per arm, and their counts carry
per-site meaning that sharing would destroy.

Reported results are unaffected. Every source entry survives with its
own file/line attribution, and hosts read each entry through its own
counter, so several entries reading one slot produce exactly the LCOV
records that dedicated slots did. `counterCount` is therefore markedly
below the entry count — roughly half on the bundled demos.

Known gap: any core-module intrinsic that abandons the invocation lowers
to a `GenericAsm` terminator like every other intrinsic, and the exit
analysis treats `GenericAsm` as a normal exit — so the gap is general to
any present or future abandoning intrinsic modeled that way, not
specific to a fixed list. The ray-tracing hit terminators `IgnoreHit`
and `AcceptHitAndEndSearch` are the concrete examples today: they end
the invocation at the target level, but Slang's IR models them as
ordinary `void` functions that return normally, so the analysis cannot
currently see them.

### Lines with several markers

A source line can carry several markers in different blocks, and it must
still report one count per visit, not one count per statement. For
example, four calls to either function below report four executions of
its body line:

```slang
int sequential(int x) { int y = x; y += 2; return y; }
int conditional(int x) { if (x > 0) return 1; else return 2; }
```

Each marker keeps its own entry, and the LCOV exporter takes the maximum
over the entries of a line. A visit to a line passes its first marker,
and a loop test on the line runs once per iteration, so the largest
count is the number of times the line was visited:

- In `conditional`, the condition runs on every call and a return runs
  on only some, so the line reports the condition's count.
- A `for` header has an initializer, a test, and an increment in
  different blocks, and reports the test's count: its condition
  evaluations, `N + 1` per loop of `N` iterations.
- A loop written on one source line reports its test the same way.

Lowering also records a marker at each evaluated condition and at the
selected value of a conditional expression, so these are visits of their
own lines, and so a branch entry never lands on a line that has no line
entry.

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
`else`. Slang emits one counter per arm, attributed to the condition
expression, and a line event for the selected value expression:

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
does not already decide the result. Each scalar operand that is actually
evaluated is a decision with a true and a false outcome, attributed to
that operand's own expression:

```slang
bool r = p && q;
```

Conceptually:

```slang
bool r;
if (p)
{
    coverageAtomic("branch: p true"); // q is evaluated
    if (q)
        coverageAtomic("branch: q true");
    else
        coverageAtomic("branch: q false");
    r = q;
}
else
{
    coverageAtomic("branch: p false"); // q is short-circuited
    r = false;
}
```

For `||` the roles swap: the true arm of the left operand
short-circuits to `true`, and the false arm evaluates the right operand.

The merged result of the operator is not a decision of its own. In
`if (a && b)`, the `if` records no site for `a && b`: `a` records its
outcomes, and `b` records its outcomes only when `a` was true. If `a` is
false, both counters for `b` stay zero, and LCOV renders them as
unevaluated (`-`), which distinguishes a skipped operand from an
evaluated operand that was false. A chain such as `(a && b) || c` has
decisions for `a`, `b`, and `c` in the same way, and parentheses and
logical negation preserve that decision tree.

The same source-oriented rule applies when the expression produces a
value rather than a condition, as in `bool r = a && b;`. GCC can
eliminate some branches in value contexts; exact backend branch counts
are outside the compatibility contract.

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
