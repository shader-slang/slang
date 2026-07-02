# Operator Expressions

Expressions are sequences of operators and operands. This page lists all operators and their canonical
semantics. Operators in an expression are evaluated in an order according to [operator
precedence](expressions-operator-precedence.md). Slang applications can declare custom implementations for
most operators (see [operator overload declarations](declarations-operators.md).

Operands are inputs for an operator. Operands are either atomic [expressions](expressions.md) or
subexpressions, possibly with parentheses to indicate subexpression grouping.

Slang operators come in the following forms:

- **Postfix operators** — operators that apply to a single operand. The position of the operator is after the
  operand. The associativity is left to right.
- **Prefix operators** — operators that apply to a single operand. The position of the operator is before the
  operand. The associativity is right to left.
- **Binary operators** — operators that apply to two operands. The position of the operator is between the
  operands (infix operators). The associativity is:
  - left to right for binary operators other than assignment.
  - right to left for assignment operators, including compound assignment.
- **Ternary conditional operator** — special three-operand operator (see below). The associativity is from
  right to left.
- **Other operators** — Function call, generic application, subscript, member access, scope


## Built-in Operators

### Arithmetic Operators (scalar)

| Operator 	| Operator function                       | Description                                       |
|-----------|-----------------------------------------|---------------------------------------------------|
| `+`		| `__prefix T operator + (T val)`         | identity (unary plus)                             |
| `-`		| `__prefix T operator - (T val)`         | arithmetic negation (unary minus)                 |
| `++`		| `__prefix T operator ++ (inout T val)`  | increment in place, return incremented value      |
| `++`		| `__postfix T operator ++ (inout T val)` | increment in place, return value before increment |
| `--`		| `__prefix T operator -- (inout T val)`  | decrement in place, return decremented value      |
| `--`		| `__postfix T operator -- (inout T val)` | decrement in place, return value before decrement |
| `*`       | `T operator * (T lhs, T rhs)`         | multiplication                                    |
| `/`       | `T operator / (T lhs, T rhs)`         | division                                          |
| `%`       | `T operator % (T lhs, T rhs)`         | remainder                                         |
| `+`       | `T operator + (T lhs, T rhs)`         | addition                                          |
| `-`       | `T operator - (T lhs, T rhs)`         | subtraction                                       |

The arithmetic operators are defined for
[IArithmetic](../../../core-module-reference/interfaces/iarithmetic-01/index.html) types. This includes
built-in integer and floating-point scalar types.

Description:

- The **identity (unary plus)** operator returns the `val` as is.
- The **arithmetic negation (unary minus)** operator returns the negated value of `val`.
  See [IArithmetic.neg](../../../core-module-reference/interfaces/iarithmetic-01/neg.html) for details.
- The **prefix increment** operator increments `val` by 1 in place and returns the incremented value.
- The **postfix increment** operator increments `val` by 1 and returns the value before increment.
- The **prefix decrement** operator decrements `val` by 1 in place and returns the decremented value.
- The **postfix decrement** operator decrements `val` by 1 in place and returns the value before decrement.
- The **addition** operator adds `lhs` and `rhs`.
  See [IArithmetic.add](../../../core-module-reference/interfaces/iarithmetic-01/add.html).
- The **subtraction** operator subtracts `rhs` from `lhs`.
  See [IArithmetic.sub](../../../core-module-reference/interfaces/iarithmetic-01/sub.html).
- The **multiplication** operator multiplies `lhs` and `rhs`.
  See [IArithmetic.mul](../../../core-module-reference/interfaces/iarithmetic-01/mul.html) for details.
- The **division** operator divides `lhs` by `rhs`.
  See [IArithmetic.div](../../../core-module-reference/interfaces/iarithmetic-01/div.html) for details.
- The **remainder** operator returns the remainder of `lhs` by `rhs` division such that
  `rem = lhs - n * rhs` where `n` is an integer and `abs(rem)` < `abs(rhs)`. The sign of remainder matches the
  sign of `lhs`.
  See [IArithmetic.mod](../../../core-module-reference/interfaces/iarithmetic-01/mod.html) for details.

### Logical Operators (scalar)

| Operator 	| Operator function                   | Description                                  |
|-----------|-------------------------------------|----------------------------------------------|
| `!`		| `__prefix T operator ! (T val)`     | logical NOT                                  |
| `&&`      | `T operator && (T lhs, T rhs)`      | logical AND                                  |
| `||`      | `T operator || (T lhs, T rhs)`      | logical OR                                   |
| `~` 		| `__prefix T operator ~ (T val)`     | bitwise NOT                                  |
| `&`       | `T operator & (T lhs, T rhs)`       | bitwise AND                                  |
| `^`       | `T operator ^ (T lhs, T rhs)`       | bitwise XOR                                  |
| `|`       | `T operator | (T lhs, T rhs)`       | bitwise OR                                   |
| `<<`      | `T operator << (T lhs, int amount)` | bitwise left shift                           |
| `>>`      | `T operator >> (T lhs, int amount)` | bitwise right shift                          |

The logical operators are defined for
[ILogical](../../../core-module-reference/interfaces/ilogical-01/index.html) types. This includes
built-in integer and Boolean scalar types.

Description:

- The **logical NOT** operator interprets `val` as a Boolean value and returns the opposite Boolean value.
  See [ILogical.not](../../../core-module-reference/interfaces/ilogical-01/not.html) for details.
- The **logical AND** operator interprets `lhs` and `rhs` as a Boolean values and returns the `true` if both
  operands are `true`. Otherwise, it returns `false`.
  See [ILogical.and](../../../core-module-reference/interfaces/ilogical-01/and.html) for details.
- The **logical OR** operator interprets `lhs` and `rhs` as a Boolean values and returns the `true` if either
  operand is `true`. Otherwise, it returns `false`.
  See [ILogical.or](../../../core-module-reference/interfaces/ilogical-01/or.html) for details.
- The **bitwise NOT** operator flips all bits in `val` and returns the value. That is, bit value 0 becomes 1,
  and bit value 1 becomes 0.
  See [ILogical.bitNot](../../../core-module-reference/interfaces/ilogical-01/bitNot.html) for details.
- The **bitwise AND** operator performs the logical AND operation for between every corresponding bit in `lhs`
  and `rhs` and returns the value.
  See [ILogical.bitAnd](../../../core-module-reference/interfaces/ilogical-01/bitAnd.html) for details.
- The **bitwise OR** operator performs the logical OR operation for between every corresponding bit in `lhs`
  and `rhs` and returns the value.
  See [ILogical.bitOr](../../../core-module-reference/interfaces/ilogical-01/bitOr.html) for details.
- The **bitwise XOR** operator performs the logical XOR (exclusive or) operation between every corresponding
  bit in `lhs` and `rhs` and returns the value.
  See [ILogical.bitXor](../../../core-module-reference/interfaces/ilogical-01/bitXor.html) for details.
- The **bitwise left shift** operator shifts all bits in `lhs` left by `amount`.
  See [ILogical.shl](../../../core-module-reference/interfaces/ilogical-01/shl.html) for details.
- The **bitwise right shift** operator shifts all bits in `lhs` right by `amount`.
  See [ILogical.shr](../../../core-module-reference/interfaces/ilogical-01/shr.html) for details.


### Comparison Operators (scalar)

| Operator  | Operator function                   | Description                                  |
|-----------|-------------------------------------|----------------------------------------------|
| `<`       | `bool operator < (T lhs, T rhs)`    | less-than comparison                         |
| `<=`      | `bool operator <= (T lhs, T rhs)`   | less-than-or-equal-to comparison             |
| `>`       | `bool operator > (T lhs, T rhs)`    | greater-than comparison                      |
| `>=`      | `bool operator >= (T lhs, T rhs)`   | greater-than-or-equal-to comparison          |
| `==`      | `bool operator == (T lhs, T rhs)`   | equal-to comparison                          |
| `!=`      | `bool operator != (T lhs, T rhs)`   | not-equal-to comparison                      |

The comparison operators are defined for
[IComparable](../../../core-module-reference/interfaces/icomparable-01/index.html) types. This includes
built-in integer and Boolean scalar types.

Description:

- The *less-than* comparison operator returns `true` if `lhs` is less than `rhs`. Otherwise, it returns
  `false`.
  See [IComparable.lessThan](../../../core-module-reference/interfaces/icomparable-01/lessthan-4.html) for details.
- The *less-than-or-equal-to* comparison operator returns `true` if `lhs` is less than or equal to
  `rhs`. Otherwise, it returns `false`.
  See [IComparable.lessThanOrEquals](../../../core-module-reference/interfaces/icomparable-01/lessthanorequals-48a.html)
  for details.
- The *greater-than* comparison operator returns `true` if `lhs` is greater than `rhs`. Otherwise, it returns
  `false`. Implemented using `IComparable.lessThan` with arguments reversed.
- The *greater-than-or-equal-to* comparison operator returns `true` if `lhs` is greater than or equal to
  `rhs`. Otherwise, it returns `false`. Implemented using `IComparable.lessThanOrEquals` with arguments reversed.
- The *equal-to* comparison operator returns `true` if `lhs` is equal to `rhs`. Otherwise, it returns `false`.
  See [IComparable.equals](../../../core-module-reference/interfaces/icomparable-01/equals.html) for details.
- The *not-equal-to* comparison operator returns `true` if `lhs` is not equal to `rhs`. Otherwise, it returns
  `false`. Implemented using `IComparable.equals` with the result negated (logical NOT).


### Assignment Operators (scalar)

| `=`       | `R operator = (inout T1, T2)`       | assignment                                   |
| `+=`      | `R operator += (inout T1, T2)`      | compound addition and assignment             |
| `-=`      | `R operator -= (inout T1, T2)`      | compound subtraction and assignment          |
| `*=`      | `R operator *= (inout T1, T2)`      | compound multiplication and assignment       |
| `/=`      | `R operator /= (inout T1, T2)`      | compound division and assignment             |
| `%=`      | `R operator %= (inout T1, T2)`      | compound remainder and assignment            |
| `<<=`     | `R operator <<= (inout T1, T2)`     | compound bitwise left shift and assignment   |
| `>>=`     | `R operator >>= (inout T1, T2)`     | compound bitwise right shift and assignment  |
| `&=`      | `R operator &= (inout T1, T2)`      | compound bitwise AND shift and assignment    |
| `^=`      | `R operator ^= (inout T1, T2)`      | compound bitwise XOR and assignment          |
| `|=`      | `R operator |= (inout T1, T2)`      | compound bitwise OR and assignment           |
| `,`       | `R operator ,  (T1, T2)`            | comma operator (Slang 2025 and earlier)      |


### Pointer Operators

| Operator 	| Operator function                  | Description                                  |
|-----------|------------------------------------|----------------------------------------------|
| `*`		| `__prefix R operator * (T)`        | pointer dereference (experimental)           |
| `&`		| `__prefix R operator & (__ref T)`  | address of (experimental)                    |

Canonical semantics:

- The **pointer dereference** operator returns the pointed value (L-value). The operand type is a pointer.
- The **address of** operator returns a pointer to the operand. The operand must be
  [addressable](expressions-value-categories.md).





### Ternary Conditional Operator

| Operator  | Operator function                   | Description                                  |
|-----------|-------------------------------------|----------------------------------------------|
| `?:`      | `R operator ?: (T1, T2, T3)`        | Conditional selection                        |

The conditional operator, `?:`, is used to select between two expressions based on the value of a condition:

```hlsl
useNegative ? -1.0f : 1.0f
```

The condition may be either a single value of type `bool`, or a vector of `bool`.
When a vector of `bool` is used, the two values being selected between must be vectors, and selection is performed component-wise.

> Note: Unlike C, C++, GLSL, and most other C-family languages, Slang currently follows the precedent of HLSL where `?:` does not short-circuit.
>
> This decision may change (for the scalar case) in a future version of the language.
> Programmer are encouraged to write code that does not depend on whether or not `?:` short-circuits.

### Call Expression

A _call expression_ consists of a base expression and a list of argument expressions, separated by commas and enclosed in `()`:

```hlsl
myFunction( 1.0f, 20 )
```

When the base expression (e.g., `myFunction`) is overloaded, a call expression can disambiguate the overloaded expression based on the number and type or arguments present.

The base expression of a call may be a member reference expression:

```hlsl
myObject.myFunc( 1.0f )
```

In this case the base expression of the member reference (e.g., `myObject` in this case) is used as the argument for the implicit `this` parameter of the callee.

### Subscript Expression

A _subscript expression_ consists of a base expression and a list of argument expressions, separated by commas and enclosed in `[]`:

```hlsl
myVector[someIndex]
```

A subscript expression invokes one of the subscript declarations in the type of the base expression. Which subscript declaration is invoked is resolved based on the number and types of the arguments.

A subscript expression is an l-value if the base expression is an l-value and if the subscript declaration it refers to has a setter or by-reference accessor.

Subscripts may be formed on the built-in vector, matrix, and array types.


Cast Expression
---------------

A _cast expression_ attempt to coerce a single value (the base expression) to a desired type (the target type):

```hlsl
(int) 1.0f
```

A cast expression can perform both built-in type conversions and invoke any single-argument initializers of the target type.

### Compatibility Feature

As a compatibility feature for older code, Slang supports using a cast where the base expression is an integer literal zero and the target type is a user-defined structure type:

```hlsl
MyStruct s = (MyStruct) 0;
```

The semantics of such a cast are equivalent to initialization from an empty initializer list:

```hlsl
MyStruct s = {};
```

Assignment Expression
---------------------

An _assignment expression_ consists of a left-hand side expression, an equals sign (`=`), and a right-hand-side expressions:

```hlsl
myVar = someValue
```

The semantics of an assignment expression are to:

* Evaluate the left-hand side to produce an l-value,
* Evaluate the right-hand side to produce a value
* Store the value of the right-hand side to the l-value of the left-hand side
* Yield the l-value of the left-hand-side

### Operators with Vector and Matrix Operands

For vector and matrix unary operators, the scalar operator is applied per element.

For binary operators with vector/vector and matrix/matrix operands, the vector and matrix dimensions must
match and the scalar operator is applied per element.

For binary operators with vector/scalar and matrix/scalar mixed operands, the operator is applied on every
vector or matrix element with the scalar operator.

The matrix/matrix and matrix/vector multiplication are special, and they follow the matrix multiplication
rules. See [Vector and Matrix Types](types-vectors-and-matrix.md) for details.









## Non-Overloadable Operators

### Parenthesized Expression

An expression wrapped in parentheses `()` is a _parenthesized expression_ and evaluates to the same value as
the wrapped expression.


### Generic Specialization


TODO: `<...>`

### Disambiguation between Generic Specialization and Less-Than Operator

Generic specialization is context-sensitive. When token `<` is encountered in an expression and the
left-hand-side operand is:

- known to be generic, then `<` is considered to begin generic specialization.
- known to be non-generic, then `<` is considered as the less-than operator.
- undetermined, then parsing as generic arguments is attempted. If parsing succeeds, `<...>` is interpreted as
  generic specialization. Otherwise, the initial `<` is interpreted as the less-than operator. Parsing is
  considered successful if `<...>` can be parsed as generic specialization and it is followed by one of `::`,
  `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`, `>`, `>>`, or the end-of-file marker.
