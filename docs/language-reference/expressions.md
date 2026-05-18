> Note: This document is a work in progress. It is both incomplete and, in many cases, inaccurate.

> TODO

Expressions
===========

Expressions are terms that can be _evaluated_ to produce values.
This section provides a list of the kinds of expressions that may be used in a Slang program.

In general, the order of evaluation of a Slang expression proceeds from left to right.
Where specific expressions do not follow this order of evaluation, it will be noted.

Some expressions can yield _l-values_, which allows them to be used on the left-hand-side of assignment, or as arguments for `out` or `in out` parameters.

