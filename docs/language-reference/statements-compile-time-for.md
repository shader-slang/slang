# Compile-Time For Statement

## Syntax

Compile-time `$for` loop:

> **`'$for'`** **`'('`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp; *`identifier`* **`'in'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp; **`'Range'`** **`'('`** *`init-expr`* **`','`** *`upper-bound-expr`* **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp; **`')'`**<br>
> &nbsp;&nbsp;&nbsp;&nbsp; *`loop-stmt`*<br>

## Description

A _compile-time_ `$for` loop replicates *`loop-stmt`* for every integer value in the specified range, starting
from *`init-expr`* and ending to *`upper-bound-expr`*. The upper bound is exclusive.

The expressions *`init-expr`* and *`upper-bound-expr`* are
[compile-time constants](expressions-evaluation-classes.md) and they must have [integer](types-fundamental.md) types.

The loop iterator value is provided as *`identifier`* as an immutable variable.

TODO: The compiler needs some work:
- https://github.com/shader-slang/slang/issues/12398
- https://github.com/shader-slang/slang/issues/12399
