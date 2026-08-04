> TODO

# Initializer Expressions

### Initializer Expressions

When the base expression of a call is a type instead of a value, the expression is an initializer expression:

```hlsl
float2(1.0f, 2.0f)
```

An initializer expression initialized an instance of the specified type using the given arguments.

An initializer expression with only a single argument is treated as a cast expression:

```hlsl
// these are equivalent
int(1.0f)
(int) 1.0f
```

Initializer List Expression
---------------------------

An _initializer list expression_ comprises zero or more expressions, separated by commas, enclosed in `{}`:

```
{ 1, "hello", 2.0f }
```

An initialier list expression may only be used directly as the initial-value expression of a variable or parameter declaration; initializer lists are not allowed as arbitrary sub-expressions.

> Note: This section will need to be updated with the detailed rules for how expressions in the initializer list are used to initialize values of each kind of type.

