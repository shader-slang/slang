# Operator Expressions

Expressions are sequences of operators and operands. This page lists all operators and their canonical
semantics. Operators in an expression are evaluated in an order according to [operator
precedence](expressions-operator-precedence.md). Slang applications can declare custom implementations for
most operators (see [operator overload declarations](declarations-operators.md)).

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

| Operator  | Operator function                       | Description                                       |
|-----------|-----------------------------------------|---------------------------------------------------|
| `+`       | `__prefix T operator + (T val)`         | identity (unary plus)                             |
| `-`       | `__prefix T operator - (T val)`         | arithmetic negation (unary minus)                 |
| `++`      | `__prefix T operator ++ (inout T val)`  | increment in place, return incremented value      |
| `++`      | `__postfix T operator ++ (inout T val)` | increment in place, return value before increment |
| `--`      | `__prefix T operator -- (inout T val)`  | decrement in place, return decremented value      |
| `--`      | `__postfix T operator -- (inout T val)` | decrement in place, return value before decrement |
| `*`       | `T operator * (T lhs, T rhs)`           | multiplication                                    |
| `/`       | `T operator / (T lhs, T rhs)`           | division                                          |
| `%`       | `T operator % (T lhs, T rhs)`           | remainder                                         |
| `+`       | `T operator + (T lhs, T rhs)`           | addition                                          |
| `-`       | `T operator - (T lhs, T rhs)`           | subtraction                                       |

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
  sign of `lhs`. That is, `lhs % rhs == lhs - trunc(lhs / rhs) * rhs`.
  See [IArithmetic.mod](../../../core-module-reference/interfaces/iarithmetic-01/mod.html) for details.

### Logical Operators (scalar)

| Operator  | Operator function                   | Description                                  |
|-----------|-------------------------------------|----------------------------------------------|
| `!`       | `__prefix T operator ! (T val)`     | logical NOT                                  |
| `&&`      | `T operator && (T lhs, T rhs)`      | logical AND                                  |
| `\|\|`    | `T operator \|\| (T lhs, T rhs)`    | logical OR                                   |
| `~`       | `__prefix T operator ~ (T val)`     | bitwise NOT                                  |
| `&`       | `T operator & (T lhs, T rhs)`       | bitwise AND                                  |
| `^`       | `T operator ^ (T lhs, T rhs)`       | bitwise XOR                                  |
| `\|`      | `T operator \| (T lhs, T rhs)`      | bitwise OR                                   |
| `<<`      | `T operator << (T lhs, int amount)` | bitwise left shift                           |
| `>>`      | `T operator >> (T lhs, int amount)` | bitwise right shift                          |

The logical operators are defined for
[ILogical](../../../core-module-reference/interfaces/ilogical-01/index.html) types. This includes
built-in integer and Boolean scalar types.

Description:

- The **logical NOT** operator interprets `val` as a Boolean value and returns the opposite Boolean value.
  See [ILogical.not](../../../core-module-reference/interfaces/ilogical-01/not.html) for details.
- The **logical AND** operator interprets `lhs` and `rhs` as Boolean values and returns `true` if both
  operands are `true`. Otherwise, it returns `false`.
  See [ILogical.and](../../../core-module-reference/interfaces/ilogical-01/and.html) for details.
- The **logical OR** operator interprets `lhs` and `rhs` as Boolean values and returns `true` if either
  operand is `true`. Otherwise, it returns `false`.
  See [ILogical.or](../../../core-module-reference/interfaces/ilogical-01/or.html) for details.
- The **bitwise NOT** operator flips all bits in `val` and returns the value. That is, bit value 0 becomes 1,
  and bit value 1 becomes 0.
  See [ILogical.bitNot](../../../core-module-reference/interfaces/ilogical-01/bitnot-3.html) for details.
- The **bitwise AND** operator performs the logical AND operation between every corresponding bit in `lhs`
  and `rhs` and returns the value.
  See [ILogical.bitAnd](../../../core-module-reference/interfaces/ilogical-01/bitand-3.html) for details.
- The **bitwise OR** operator performs the logical OR operation between every corresponding bit in `lhs`
  and `rhs` and returns the value.
  See [ILogical.bitOr](../../../core-module-reference/interfaces/ilogical-01/bitor-3.html) for details.
- The **bitwise XOR** operator performs the logical XOR (exclusive or) operation between every corresponding
  bit in `lhs` and `rhs` and returns the value.
  See [ILogical.bitXor](../../../core-module-reference/interfaces/ilogical-01/bitxor-3.html) for details.
- The **bitwise left shift** operator shifts all bits in `lhs` left by `amount`.
  See [ILogical.shl](../../../core-module-reference/interfaces/ilogical-01/shl.html) for details.
- The **bitwise right shift** operator shifts all bits in `lhs` right by `amount`.
  See [ILogical.shr](../../../core-module-reference/interfaces/ilogical-01/shr.html) for details.

The `&&` and `||` operators short-circuit when their operands are scalars: the right-hand operand is evaluated
only when it can affect the result. That is, in `lhs && rhs`, `rhs` is evaluated only when `lhs` is `true`. In
`lhs || rhs`, `rhs` is evaluated only when `lhs` is `false`. When the operands are vectors or matrices, `&&`
and `||` do not short-circuit and evaluate both operands element-wise. Short-circuiting can be disabled
globally with the `-disable-short-circuit` compiler option.


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
built-in integer, Boolean, and floating-point scalar types.

Description:

- The **less-than** comparison operator returns `true` if `lhs` is less than `rhs`. Otherwise, it returns
  `false`.
  See [IComparable.lessThan](../../../core-module-reference/interfaces/icomparable-01/lessthan-4.html) for details.
- The **less-than-or-equal-to** comparison operator returns `true` if `lhs` is less than or equal to
  `rhs`. Otherwise, it returns `false`.
  See [IComparable.lessThanOrEquals](../../../core-module-reference/interfaces/icomparable-01/lessthanorequals-48a.html)
  for details.
- The **greater-than** comparison operator returns `true` if `lhs` is greater than `rhs`. Otherwise, it returns
  `false`. Implemented using `IComparable.lessThan` with arguments swapped.
- The **greater-than-or-equal-to** comparison operator returns `true` if `lhs` is greater than or equal to
  `rhs`. Otherwise, it returns `false`. Implemented using `IComparable.lessThanOrEquals` with arguments swapped.
- The **equal-to** comparison operator returns `true` if `lhs` is equal to `rhs`. Otherwise, it returns `false`.
  See [IComparable.equals](../../../core-module-reference/interfaces/icomparable-01/equals.html) for details.
- The **not-equal-to** comparison operator returns `true` if `lhs` is not equal to `rhs`. Otherwise, it
  returns `false`. Implemented using `IComparable.equals` with the comparison result negated (logical NOT).


### Assignment Operators (scalar)

| Operator  | Operator function                          | Description                                  |
|-----------|--------------------------------------------|----------------------------------------------|
| `=`       | `T operator = (inout T lhs, T rhs)`        | assignment (non-overloadable built-in type)  |
| `+=`      | `T operator += (inout T lhs, T rhs)`       | compound addition and assignment             |
| `-=`      | `T operator -= (inout T lhs, T rhs)`       | compound subtraction and assignment          |
| `*=`      | `T operator *= (inout T lhs, T rhs)`       | compound multiplication and assignment       |
| `/=`      | `T operator /= (inout T lhs, T rhs)`       | compound division and assignment             |
| `%=`      | `T operator %= (inout T lhs, T rhs)`       | compound remainder and assignment            |
| `&=`      | `T operator &= (inout T lhs, T rhs)`       | compound bitwise AND and assignment          |
| `\|=`     | `T operator \|= (inout T lhs, T rhs)`      | compound bitwise OR and assignment           |
| `^=`      | `T operator ^= (inout T lhs, T rhs)`       | compound bitwise XOR and assignment          |
| `<<=`     | `T operator <<= (inout T lhs, int amount)` | compound bitwise left shift and assignment   |
| `>>=`     | `T operator >>= (inout T lhs, int amount)` | compound bitwise right shift and assignment  |

The assignment operator is a built-in definition available for all copyable types. The assignment operator is
not overloadable. See [Special Types](types-special.md) for a discussion on non-copyable types.

The default compound assignment operators behave as if implemented with the following template:

```hlsl
__generic<L: ..., R: ...>
L operator COMPOUND_ASSIGN_OP (inout L left, R right)
{
    left = left OP right;
    return left;
}
```

where `COMPOUND_ASSIGN_OP` is the operator combined with assignment and `OP` is the operator without
assignment.

The compound assignment operator is defined for the same types as `OP`. For example, `+=` is defined for
[IArithmetic](../../../core-module-reference/interfaces/iarithmetic-01/index.html) types.


> 📝 **Remark:** Unlike in C/C++, the assignment operators return an R-value.



### Miscellaneous Operators (scalar)

| Operator  | Operator function                                                                 | Description                             |
|---------------|-----------------------------------------------------------------------------------|-----------------------------------------|
| `*`           | `__prefix Ref<T, a, s> operator * (Ptr<T, a, s, L>)`                              | pointer dereference (experimental)      |
| `&`           | `__prefix Ptr<T, Access::ReadWrite, AddressSpace::Device> operator & (__ref T v)` | address of (experimental)               |
| `,`           | `T2 operator , (T1 lhs, T2 rhs)`                                                  | comma operator (Slang 2025 and earlier) |

Canonical semantics:

- The **pointer dereference** operator returns the pointed value (L-value). The operand type is a pointer.
  Note that the layout parameter `L` is not captured by the returned reference. See also
  [Ptr](../../../core-module-reference/types/ptr-0/index.html).
- The **address of** operator returns a pointer to the operand. The operand must be
  [addressable](expressions-value-categories.md).
- The **comma operator** (Slang 2025 and earlier) returns the right-hand-side parameter. Starting from Slang
  2026, the comma is no longer an operator.

> ⚠️ **Warning:** The _pointer dereference_ and _address of_ operators are currently experimental in Slang. The
> details are subject to change.

> 📝 **Remark:** Starting from Slang 2026, the comma (`,`) is no longer an operator. It is used as a separator
> in call expressions, [initializer expressions](expressions-initializer.md),
> [tuple expressions](types-tuple.md), and various [declarations](declarations.md).


### Ternary Conditional Operator

| Operator | Operator function                                  | Description           |
|----------|----------------------------------------------------|-----------------------|
| `?:`     | `T operator ?: (bool cond, T trueVal, T falseVal)` | Conditional selection |

Description:

The conditional operator `?:` is used to select between two values based on the condition (`cond`). If the
condition is `true`, `trueVal` is returned. Otherwise, `falseVal` is returned.

The default ternary conditional operator is provided for all copyable types.

The ternary conditional operator is short-circuiting for a scalar condition.

> ⚠️ **Warning:** The ternary conditional operator is also defined for vector condition operands for legacy
> reasons. In this deprecated form, the condition vector length must match the `trueVal` and `falseVal` vector
> lengths, and for each element, the corresponding element of the condition selects the corresponding
> element of either `trueVal` or `falseVal`. However, this form is non-short-circuiting. Use
> [select()](../../../core-module-reference/global-decls/select.html) instead to make the non-short-circuiting
> behavior explicit.

### Call Expression

**Grammar:**

> *`callable-expr`* **`'('`** [ *`arg-expr`* (**`','`** *`arg-expr`*)\* ] **`')'`**

A _call expression_ consists of a base expression *`callable-expr`* and a list of argument expressions
*`arg-expr`*.

The base expression must be a [function](declarations-functions.md), a [member function](types-struct.md), an
invocable object (a [struct](types-struct.md) with the function call operator defined), or a constructible
type. In case the base expression is an [identifier expression](expressions-identifier.md) that is overloaded
with multiple declarations, [overload resolution](expressions-overload-resolution.md) selects the most
appropriate one.

If the callable expression is a function, a member function, or an invocable object, the value of the expression
is the return value of the invocation.

If the callable expression is a type, then an object of that type is instantiated and the arguments are passed
to the constructor. The value of the call expression is the instantiated object.

If the argument type does not match with the parameter type, it is implicitly
[converted](expressions-conversions.md) to the target type. It is an error if the implicit conversion is not
available.

If an argument is not supplied to a parameter that has a default value, the default value is used. It is an
error to omit an argument for a parameter that does not have a default.

If the callable expression is an invocable object or an object member and the function declaration is not
static, then the object is passed as the argument to the implicit `this` parameter.



### Subscript Expression

**Grammar:**

> *`base-expr`* **`'['`** [ *`arg-expr`* (**`','`** *`arg-expr`*)\* ] **`']'`**

A _subscript expression_ consists of a base expression and a list of argument expressions.

The base expression type must be an [array](types-array.md), a [vector](types-vector-and-matrix.md), a
[matrix](types-vector-and-matrix.md), or a [struct](types-struct.md) with the subscript operator defined as a
member.

For array, vector, and matrix types, the built-in subscript operator semantics are defined by
[IArray](../../../core-module-reference/interfaces/iarray-01/subscript.html) (for R-value base expressions)
and [IRWArray](../../../core-module-reference/interfaces/irwarray-0123/subscript.html) (for L-value base
expressions). The subscript operator has a single argument, which returns the element of an array or a vector,
or the row vector of a matrix. The returned value is an L-value if *`base-expr`* is an L-value. Otherwise, it
is an R-value.

For [struct types](types-struct.md), the subscript expression is translated to a call to the subscript member
operator. If the subscript expression is an assignment or assignment-like expression, the call is translated
to the `set` accessor call of the subscript declaration. Otherwise, the call is translated to the `get`
accessor call. The arguments are passed as is. In case there are multiple `__subscript` declarations,
[overload resolution](expressions-overload-resolution.md) selects the most appropriate one.


### Member Expression

**Grammar:**

> *`namespace-identifier`* (**`'.'`** | **`'::'`**) *`member-identifier`*
>
> *`type-expr`* (**`'.'`** | **`'::'`**) *`member-identifier`*
>
> *`value-expr`* **`'.'`** *`member-identifier`*
>
> *`pointer-value-expr`* (**`'.'`** | **`'->'`**) *`member-identifier`*

A _member access expression_ selects a member of a namespace, a type, or a value expression.

For namespace base expressions, *`member-identifier`* is an identifier within the namespace. For type
expressions, *`member-identifier`* is a member identifier.

For value expressions, *`member-identifier`* is a member of the value, such as a component, field, member
function, or a property. For scalars, vectors, matrices, and tuples, the *`member-identifier`* may also be a
sequence of components, resulting in a _swizzle expression_.

For pointer-typed and pointer-like values, members can be selected using either the `.` or `->` operator. When
the `.` operator is used, the pointer is implicitly dereferenced as necessary. The arrow operator `->`
selects a member through a pointer with an explicit dereference.

Member expressions are discussed in more detail in [Member Expression](expressions-member-access.md).

> ⚠️ **Warning:** The arrow operator `->` is currently experimental in Slang. The details are subject to
> change.

### Cast Expression

**Grammar:**

> **`'('`** *`type-expr`* **`')'`** *`base-expr`*

A _cast expression_ converts a value (*`base-expr`*) to the desired type (*`type-expr`*). This is also known
as an explicit type conversion. See [Expression Type Conversions](expressions-conversions.md) for details.


### Operators with Vector and Matrix Operands

For vector and matrix unary operators, the scalar operator is applied per element.

For binary operators with vector/vector and matrix/matrix operands, the vector and matrix dimensions must
match and the scalar operator is applied per element.

For binary operators with vector/scalar and matrix/scalar mixed operands, the operator is applied on every
vector or matrix element with the scalar operator.

The matrix/matrix and matrix/vector multiplication are special, and they follow the matrix multiplication
rules. See [Vector and Matrix Types](types-vector-and-matrix.md) for details.

> 📝 **Remark:** The short-circuiting behavior of operators `&&`, `||`, and `?:` differs between scalar and
> vector/matrix operands. See sections _Logical Operators (scalar)_ and _Ternary Conditional Operator_ for
> details.

## Non-Overloadable Operators

### Parenthesized Expression

An expression wrapped in parentheses `()` is a _parenthesized expression_. It evaluates to the same value as
the wrapped expression. Parenthesized expressions can be used to control the evaluation order of
subexpressions.


### Generic Specialization

**Grammar:**

> *`gen-expr`* **`'<'`** [*`gen-arg-expr`* (**`','`** *`gen-arg-expr`*)\* ] **`'>'`**
>
> *`gen-arg-expr`* = *`type-expr`* | *`value-expr`*

A generic type or an invocable (e.g., function) can be explicitly specialized by supplying the arguments to
the generic parameters. When the arguments are not supplied, they are inferred as required. For details, see
_Parameter Binding_ in [Generics](generics.md).

#### Disambiguation between Generic Specialization and Less-Than Operator

Generic specialization is context-sensitive. When token `<` is encountered in an expression and the
left-hand-side operand is:

- known to be generic, then `<` is considered to begin generic specialization.
- known to be non-generic, then `<` is considered as the less-than operator.
- undetermined, then parsing as generic arguments is attempted. If parsing succeeds, `<...>` is interpreted as
  generic specialization. Otherwise, the initial `<` is interpreted as the less-than operator. Parsing is
  considered successful if `<...>` can be parsed as generic specialization and it is followed by one of `::`,
  `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`, `>`, `>>`, or the end-of-file marker.
