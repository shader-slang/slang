# Statements

Statements define the actions of the program. In Slang, statements are confined in
[function](declarations-functions.md) bodies.

Slang statements are divided into the following categories:

- Control flow statements
- Expression and declaration statements
- Exception handling statements
- Miscellaneous statements

Control flow statements define the control flow of the program. The control flow statements are:

- [if](statements-if.md) — conditional branching.
- [for](statements-loop.md) — loop execution consisting of initialization, loop condition (evaluated before
  iteration), loop statement, and a post-loop action.
- [while](statements-loop.md) — loop execution consisting of loop condition (evaluated before iteration) and
  loop statement.
- [do-while](statements-loop.md) — loop execution consisting of loop condition (evaluated after iteration) and
  loop statement.
- [switch, case, and default](statements-switch.md) — multi-way branching.
- [break and continue](statements-break-and-continue.md) — loop/switch exit, loop continue.
- [return](statements-return.md) — exit the current function, possibly with a return value.
- [defer](statements-defer.md) — deferred statement execution, executed on current scope exit (in last-in,
  first-out (LIFO) order).
- [discard](statements-discard.md) — disable the current thread (fragment shaders only).

Expression statements evaluate expressions, and declaration statements declare types and variables for
successive statements in the current scope:

- [declaration statement](statements-declaration.md) — declares a type or variable in the current scope. The
  declaration is available for successive statements.
- [expression statement](statements-expression.md) — evaluates an expression. An expression statement
  typically has a side effect such as writing to memory. This can be either directly (e.g.,
  [variable assignment](expressions-operators.md)) or indirectly (e.g., via a
  [call expression](expressions-operators.md)).

Exception handling statements:

- [`do-catch` statement](statements-do-catch.md) — handles exceptions thrown within its `do` body, including
  those propagated from a [try expression](expressions-try.md).
- [`throw` statement](statements-throw.md) — throws an exception.

Miscellaneous:

- [block statement](statements-block.md) — groups multiple statements in a single statement.
- [empty statement](statements-empty.md) — does nothing.

## Compile-Time For Statement

TODO: this needs to be moved somewhere

A _compile-time for statement_ is used as an alternative to preprocessor techniques for loop unrolling.
It looks like:

```hlsl
$for( <name> in Range(<initial-value>, <upper-bound>)) <body statement>
```

The _initial value_ and _upper bound_ expressions must be compile-time constant integers.
The semantics of a compile-time for statement are as if it were expanded into:

```hlsl
{
    let <name> = <initial-value>;
    <body statement>
}
{
    let <name> = <initial-value> + 1;
    <body statement>
}
...
{
    let <name> = <upper-bound> - 1;
    <body statement>
}
```
