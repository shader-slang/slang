Expressions
===========

Expressions are terms that are _evaluated_ to produce values. Expressions are divided into atomic and operator
expressions. Atomic expressions are _indivisible_, whereas operator expressions consist of an operator and
one or more subexpressions.

Examples of atomic expressions (also called leaf expressions) are [literal expressions](expressions-literal.md),
[identifier expressions](expressions-identifier.md), and the [`this` expression](expressions-this.md).

Operator expressions range from single-operand prefix and postfix expressions to binary expressions and
ternary conditional expressions. The grouping of operators is determined by [operator
precedence](expressions-operator-precedence.md).

Expressions may have side effects. That is, they produce observable effects beyond their resulting value.
Notably, an assignment expression has the side effect of storing a value, and a function call expression may
produce any number of side effects depending on the called function. When expressions access memory that is
shared between threads, maintaining [memory consistency](basics-memory-model-consistency.md) is important in
order to avoid [undefined behavior](basics-behavior.md). Slang also imposes additional constraints when
[calling functions with `in`/`out`/`inout` parameters](basics-memory-model-special-topics.md).

In Slang, there are two primary [value categories](expressions-value-categories.md), called _l-values_ and
_r-values_. L-values may be used on the left-hand side of an assignment and as arguments for `out`/`inout`
function parameters. L-values decay automatically to r-values when needed.

When the types of operands do not match the operator type requirements, a
[conversion](expressions-conversions.md) is required. A conversion may be implicit, where the compiler injects
it automatically, or explicit, where the program applies a conversion operator.

An expression may be evaluated at translation time or at runtime. Translation-time-evaluated values are generally
called _constants_, as opposed to _runtime values_. This is discussed in more detail in [evaluation of
expressions](expressions-evaluation-classes.md).


Contents
--------

* [Literal Expressions](expressions-literal.md)
* [Identifiers](expressions-identifier.md)
* [Operators](expressions-operators.md)
* [Member Access Expressions](expressions-member-access.md)
* [Operator Precedence](expressions-operator-precedence.md)
* [`this` Expression](expressions-this.md)
* [Conversions](expressions-conversions.md)
* [Initializer Expressions](expressions-initializer.md)
* [Value Categories](expressions-value-categories.md)
* [Evaluation of Expressions](expressions-evaluation-classes.md)
