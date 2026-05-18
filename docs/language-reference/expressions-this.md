
This Expression
===============

A _this expression_ consists of the keyword `this` and refers to the implicit instance of the enclosing type that is being operated on in instance methods, subscripts, and initializers.

The type of `this` is `This`.

### Mutability

If a `[mutating]` instance is being called, the argument for the implicit `this` parameter must be an l-value.

The argument expressions corresponding to any `out` or `in out` parameters of the callee must be l-values.

A call expression is never an l-value.
