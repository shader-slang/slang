> TODO

Operator Expressions
--------------------

### Prefix Operator Expressions

The following prefix operators are supported:

| Operator 	| Description |
|-----------|-------------|
| `+`		| identity |
| `-`		| arithmetic negation |
| `~` 		| bit-wise Boolean negation |
| `!`		| Boolean negation |
| `++`		| increment in place |
| `--`		| decrement in place |

A prefix operator expression like `+val` is equivalent to a call expression to a function of the matching name `operator+(val)`, except that lookup for the function only considers functions marked with the `__prefix` keyword.

The built-in prefix `++` and `--` operators require that their operand is an l-value, and work as follows:

* Evaluate the operand to produce an l-value
* Read from the l-value to yield an _old value_
* Increment or decrement the value to yield a _new value_
* Write the new value to the l-value
* Yield the new value

### Postfix Operator Expressions

The following postfix operators are supported:

| Operator 	| Description |
|-----------|-------------|
| `++`		| increment in place |
| `--`		| decrement in place |

A postfix operator expression like `val++` is equivalent to a call expression to a function of the matching name `operator++(val)`, except that lookup for the function only considers functions marked with the `__postfix` keyword.

The built-in prefix `++` and `--` operators require that their operand is an l-value, and work as follows:

* Evaluate the operand to produce an l-value
* Read from the l-value to yield an _old value_
* Increment or decrement the value to yield a _new value_
* Write the new value to the l-value
* Yield the old value

### Infix Operator Expressions

The follow infix binary operators are supported:

| Operator 	| Kind        | Description |
|-----------|-------------|-------------|
| `*`		| Multiplicative 	| multiplication |
| `/`		| Multiplicative 	| division |
| `%`		| Multiplicative 	| remainder of division |
| `+`		| Additive 			| addition |
| `-`		| Additive 			| subtraction |
| `<<`		| Shift 			| left shift |
| `>>`		| Shift 			| right shift |
| `<` 		| Relational 		| less than |
| `>`		| Relational 		| greater than |
| `<=`		| Relational 		| less than or equal to |
| `>=`		| Relational 		| greater than or equal to |
| `==`		| Equality 			| equal to |
| `!=`		| Equality 			| not equal to |
| `&`		| BitAnd 			| bitwise and |
| `^`		| BitXor			| bitwise exclusive or |
| `\|`		| BitOr 			| bitwise or |
| `&&`		| And 				| logical and |
| `\|\|`	| Or 				| logical or |
| `+=`		| Assignment  		| compound add/assign |
| `-=`      | Assignment  		| compound subtract/assign |
| `*=`      | Assignment  		| compound multiply/assign |
| `/=`      | Assignment  		| compound divide/assign |
| `%=`      | Assignment  		| compound remainder/assign |
| `<<=`     | Assignment  		| compound left shift/assign |
| `>>=`     | Assignment  		| compound right shift/assign |
| `&=`      | Assignment  		| compound bitwise and/assign |
| `\|=`     | Assignment  		| compound bitwise or/assign |
| `^=`      | Assignment  		| compound bitwise xor/assign |
| `=`       | Assignment  		| assignment |
| `,`		| Sequencing  		| sequence |

With the exception of the assignment operator (`=`), an infix operator expression like `left + right` is equivalent to a call expression to a function of the matching name `operator+(left, right)`.

### Conditional Expression

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

Parenthesized Expression
----------------------

An expression wrapped in parentheses `()` is a _parenthesized expression_ and evaluates to the same value as the wrapped expression.

Call Expression
---------------

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

Subscript Expression
--------------------

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


Generic Specialization
----------------------

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
